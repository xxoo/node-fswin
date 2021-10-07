#pragma once
#include "common.h"

class getVolumeSpace {
public:
	const struct spaces {
		ULONGLONG totalSpace;
		ULONGLONG freeSpace;
	};
	static spaces *func(const wchar_t *path) {//you need to delete the result yourself if it is not NULL
		ULARGE_INTEGER u1;
		ULARGE_INTEGER u2;
		spaces *result;
		if (GetDiskFreeSpaceExW(path, &u1, &u2, NULL)) {
			result = new spaces;
			result->freeSpace = u1.QuadPart;
			result->totalSpace = u2.QuadPart;
		} else {
			result = NULL;
		}
		return result;
	}
	static napi_value init(napi_env env, bool isSync = false) {
		napi_value f;
		napi_create_function(env, NULL, 0, isSync ? sync : async, NULL, &f);
		return f;
	}
private:
	const struct cbdata {
		napi_async_work work;
		napi_ref self;
		napi_ref cb;
		wchar_t *path;
		spaces *result;
	};
	static napi_value sync(napi_env env, napi_callback_info info) {
		napi_value result;
		napi_get_new_target(env, info, &result);
		if (result) {
			result = NULL;
			napi_throw_error(env, SYB_EXP_INVAL, SYB_ERR_NOT_A_CONSTRUCTOR);
		} else {
			napi_value argv;
			size_t argc = 1;
			napi_get_cb_info(env, info, &argc, &argv, NULL, NULL);
			if (argc < 1) {
				napi_throw_error(env, SYB_EXP_INVAL, SYB_ERR_WRONG_ARGUMENTS);
			} else {
				size_t str_len;
				napi_value tmp;
				napi_coerce_to_string(env, argv, &tmp);
				napi_get_value_string_utf16(env, tmp, NULL, 0, &str_len);
				str_len += 1;
				wchar_t *str = new wchar_t[str_len];
				napi_get_value_string_utf16(env, tmp, (char16_t*)str, str_len, NULL);
				spaces *r = func(str);
				delete[]str;
				if (r) {
					result = convert(env, r);
					delete r;
				} else {
					napi_get_null(env, &result);
				}
			}
		}
		return result;
	}
	static napi_value async(napi_env env, napi_callback_info info) {
		napi_value result;
		napi_get_new_target(env, info, &result);
		if (result) {
			result = NULL;
			napi_throw_error(env, SYB_EXP_INVAL, SYB_ERR_NOT_A_CONSTRUCTOR);
		} else {
			napi_value argv[2], self;
			size_t argc = 2;
			napi_get_cb_info(env, info, &argc, argv, &self, NULL);
			if (argc >= 2) {
				napi_valuetype t;
				napi_typeof(env, argv[1], &t);
				if (t == napi_function) {
					cbdata *data = new cbdata;
					size_t str_len;
					napi_value tmp;
					napi_create_reference(env, argv[1], 1, &data->cb);
					napi_create_reference(env, self, 1, &data->self);
					napi_coerce_to_string(env, argv[0], &tmp);
					napi_get_value_string_utf16(env, tmp, NULL, 0, &str_len);
					str_len += 1;
					data->path = new wchar_t[str_len];
					napi_get_value_string_utf16(env, tmp, (char16_t*)data->path, str_len, NULL);
					napi_create_string_latin1(env, "fswin.getVolumeSize", NAPI_AUTO_LENGTH, &tmp);
					napi_create_async_work(env, NULL, tmp, execute, complete, data, &data->work);
					if (napi_queue_async_work(env, data->work) == napi_ok) {
						napi_get_boolean(env, true, &result);
					} else {
						napi_get_boolean(env, false, &result);
						napi_delete_reference(env, data->cb);
						napi_delete_reference(env, data->self);
						napi_delete_async_work(env, data->work);
						delete[]data->path;
						delete data;
					}
				}
			}
			if (!result) {
				napi_throw_error(env, SYB_EXP_INVAL, SYB_ERR_WRONG_ARGUMENTS);
			}
		}
		return result;
	}
	static napi_value convert(napi_env env, spaces *data) {
		napi_value tmp, result;
		napi_create_object(env, &result);
		napi_create_int64(env, data->freeSpace, &tmp);
		napi_set_named_property(env, result, "FREE", tmp);
		napi_create_int64(env, data->totalSpace, &tmp);
		napi_set_named_property(env, result, "TOTAL", tmp);
		return result;
	}
	static void execute(napi_env env, void *data) {
		cbdata *d = (cbdata*)data;
		d->result = func(d->path);
	}
	static void complete(napi_env env, napi_status status, void *data) {
		cbdata *d = (cbdata*)data;
		delete[]d->path;
		napi_value cb, self, argv;
		napi_get_reference_value(env, d->cb, &cb);
		napi_get_reference_value(env, d->self, &self);
		if (status == napi_ok && d->result) {
			argv = convert(env, d->result);
			delete d->result;
		} else {
			napi_get_null(env, &argv);
		}
		napi_call_function(env, self, cb, 1, &argv, NULL);
		napi_delete_reference(env, d->cb);
		napi_delete_reference(env, d->self);
		napi_delete_async_work(env, d->work);
		delete d;
	}
};