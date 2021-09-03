#pragma once
#include "common.h"

class setAttributes {
public:
	const struct attrVal {//0=keep,1=yes,-1=no
		char archive;
		char hidden;
		char notContentIndexed;
		char offline;
		char readonly;
		char system;
		char temporary;
	};
	static bool func(const wchar_t *file, const attrVal *attr) {
		bool result;
		DWORD oldattr = GetFileAttributesW(file);
		if (oldattr == INVALID_FILE_ATTRIBUTES) {
			result = false;
		} else {
			DWORD newattr = oldattr;
			if (attr->archive < 0 && newattr&FILE_ATTRIBUTE_ARCHIVE) {
				newattr ^= FILE_ATTRIBUTE_ARCHIVE;
			} else if (attr->archive > 0 && !(newattr&FILE_ATTRIBUTE_ARCHIVE)) {
				newattr |= FILE_ATTRIBUTE_ARCHIVE;
			}
			if (attr->hidden < 0 && newattr&FILE_ATTRIBUTE_HIDDEN) {
				newattr ^= FILE_ATTRIBUTE_HIDDEN;
			} else if (attr->hidden > 0 && !(newattr&FILE_ATTRIBUTE_HIDDEN)) {
				newattr |= FILE_ATTRIBUTE_HIDDEN;
			}
			if (attr->notContentIndexed < 0 && newattr&FILE_ATTRIBUTE_NOT_CONTENT_INDEXED) {
				newattr ^= FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
			} else if (attr->notContentIndexed > 0 && !(newattr&FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)) {
				newattr |= FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
			}
			if (attr->offline < 0 && newattr&FILE_ATTRIBUTE_OFFLINE) {
				newattr ^= FILE_ATTRIBUTE_OFFLINE;
			} else if (attr->offline > 0 && !(newattr&FILE_ATTRIBUTE_OFFLINE)) {
				newattr |= FILE_ATTRIBUTE_OFFLINE;
			}
			if (attr->readonly < 0 && newattr&FILE_ATTRIBUTE_READONLY) {
				newattr ^= FILE_ATTRIBUTE_READONLY;
			} else if (attr->readonly > 0 && !(newattr&FILE_ATTRIBUTE_READONLY)) {
				newattr |= FILE_ATTRIBUTE_READONLY;
			}
			if (attr->system < 0 && newattr&FILE_ATTRIBUTE_SYSTEM) {
				newattr ^= FILE_ATTRIBUTE_SYSTEM;
			} else if (attr->system > 0 && !(newattr&FILE_ATTRIBUTE_SYSTEM)) {
				newattr |= FILE_ATTRIBUTE_SYSTEM;
			}
			if (attr->temporary < 0 && newattr&FILE_ATTRIBUTE_TEMPORARY) {
				newattr ^= FILE_ATTRIBUTE_TEMPORARY;
			} else if (attr->temporary > 0 && !(newattr&FILE_ATTRIBUTE_TEMPORARY)) {
				newattr |= FILE_ATTRIBUTE_TEMPORARY;
			}
			if (newattr == oldattr) {
				result = true;
			} else {
				result = SetFileAttributesW(file, newattr) ? true : false;
			}
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
		attrVal *attr;
		bool result;
	};
	static napi_value sync(napi_env env, napi_callback_info info) {
		napi_value result;
		napi_get_new_target(env, info, &result);
		if (result) {
			result = NULL;
			napi_throw_error(env, SYB_EXP_INVAL, SYB_ERR_NOT_A_CONSTRUCTOR);
		} else {
			napi_value argv[2];
			size_t argc = 2;
			napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
			if (argc < 2) {
				napi_throw_error(env, SYB_EXP_INVAL, SYB_ERR_WRONG_ARGUMENTS);
			} else {
				napi_valuetype t;
				napi_typeof(env, argv[1], &t);
				if (t == napi_object) {
					size_t str_len;
					napi_value tmp;
					napi_coerce_to_string(env, argv[0], &tmp);
					napi_get_value_string_utf16(env, tmp, NULL, 0, &str_len);
					str_len += 1;
					wchar_t *str = new wchar_t[str_len];
					napi_get_value_string_utf16(env, tmp, (char16_t*)str, str_len, NULL);
					attrVal *attr = convert(env, argv[1]);
					napi_get_boolean(env, func(str, attr), &result);
					delete attr;
					delete[]str;
				} else {
					napi_throw_error(env, SYB_EXP_INVAL, SYB_ERR_WRONG_ARGUMENTS);
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
			napi_value argv[3], self;
			size_t argc = 3;
			napi_get_cb_info(env, info, &argc, argv, &self, NULL);
			if (argc < 3) {
				napi_throw_error(env, SYB_EXP_INVAL, SYB_ERR_WRONG_ARGUMENTS);
			} else {
				napi_valuetype t, t1;
				napi_typeof(env, argv[1], &t1);
				napi_typeof(env, argv[2], &t);
				if (t1 == napi_object && t == napi_function) {
					cbdata *data = new cbdata;
					size_t str_len;
					napi_value tmp;
					data->attr = convert(env, argv[1]);
					napi_create_reference(env, argv[2], 1, &data->cb);
					napi_create_reference(env, self, 1, &data->self);
					napi_coerce_to_string(env, argv[0], &tmp);
					napi_get_value_string_utf16(env, tmp, NULL, 0, &str_len);
					str_len += 1;
					data->path = new wchar_t[str_len];
					napi_get_value_string_utf16(env, tmp, (char16_t*)data->path, str_len, NULL);
					napi_create_string_latin1(env, "fswin.setAttrubutes", NAPI_AUTO_LENGTH, &tmp);
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
				} else {
					napi_throw_error(env, SYB_EXP_INVAL, SYB_ERR_WRONG_ARGUMENTS);
				}
			}
		}
		return result;
	}
	static attrVal *convert(napi_env env, napi_value attr) {//delete the result yourself
		attrVal *result = new attrVal;
		bool tmp;
		napi_value val;
		napi_has_named_property(env, attr, SYB_FILEATTR_ISARCHIVED, &tmp);
		if (tmp) {
			napi_get_named_property(env, attr, SYB_FILEATTR_ISARCHIVED, &val);
			napi_get_value_bool(env, val, &tmp);
			result->archive = tmp ? 1 : -1;
		} else {
			result->archive = 0;
		}
		napi_has_named_property(env, attr, SYB_FILEATTR_ISHIDDEN, &tmp);
		if (tmp) {
			napi_get_named_property(env, attr, SYB_FILEATTR_ISHIDDEN, &val);
			napi_get_value_bool(env, val, &tmp);
			result->hidden = tmp ? 1 : -1;
		} else {
			result->hidden = 0;
		}
		napi_has_named_property(env, attr, SYB_FILEATTR_ISNOTCONTENTINDEXED, &tmp);
		if (tmp) {
			napi_get_named_property(env, attr, SYB_FILEATTR_ISNOTCONTENTINDEXED, &val);
			napi_get_value_bool(env, val, &tmp);
			result->notContentIndexed = tmp ? 1 : -1;
		} else {
			result->notContentIndexed = 0;
		}
		napi_has_named_property(env, attr, SYB_FILEATTR_ISOFFLINE, &tmp);
		if (tmp) {
			napi_get_named_property(env, attr, SYB_FILEATTR_ISOFFLINE, &val);
			napi_get_value_bool(env, val, &tmp);
			result->offline = tmp ? 1 : -1;
		} else {
			result->offline = 0;
		}
		napi_has_named_property(env, attr, SYB_FILEATTR_ISREADONLY, &tmp);
		if (tmp) {
			napi_get_named_property(env, attr, SYB_FILEATTR_ISREADONLY, &val);
			napi_get_value_bool(env, val, &tmp);
			result->readonly = tmp ? 1 : -1;
		} else {
			result->readonly = 0;
		}
		napi_has_named_property(env, attr, SYB_FILEATTR_ISSYSTEM, &tmp);
		if (tmp) {
			napi_get_named_property(env, attr, SYB_FILEATTR_ISSYSTEM, &val);
			napi_get_value_bool(env, val, &tmp);
			result->system = tmp ? 1 : -1;
		} else {
			result->system = 0;
		}
		napi_has_named_property(env, attr, SYB_FILEATTR_ISTEMPORARY, &tmp);
		if (tmp) {
			napi_get_named_property(env, attr, SYB_FILEATTR_ISTEMPORARY, &val);
			napi_get_value_bool(env, val, &tmp);
			result->temporary = tmp ? 1 : -1;
		} else {
			result->temporary = 0;
		}
		return result;
	}
	static void execute(napi_env env, void *data) {
		cbdata *d = (cbdata*)data;
		d->result = func(d->path, d->attr);
	}
	static void complete(napi_env env, napi_status status, void *data) {
		cbdata *d = (cbdata*)data;
		delete[]d->path;
		delete d->attr;
		napi_value cb, self, argv;
		napi_get_reference_value(env, d->cb, &cb);
		napi_get_reference_value(env, d->self, &self);
		napi_get_boolean(env, status == napi_ok && d->result, &argv);
		napi_call_function(env, self, cb, 1, &argv, NULL);
		napi_delete_reference(env, d->cb);
		napi_delete_reference(env, d->self);
		napi_delete_async_work(env, d->work);
		delete d;
	}
};