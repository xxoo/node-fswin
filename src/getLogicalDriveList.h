#pragma once
#include "common.h"

class getLogicalDriveList {
public:
	const struct drive {
		char v;
		BYTE type;
	};
	static drive* func() { // you need to delete the result yourself if it is not NULL
		DWORD all = GetLogicalDrives();
		drive* result = NULL;
		const char v = 'A';
		char p[4];
		p[1] = ':';
		p[2] = '\\';
		p[3] = 0;
		DWORD d = 1;
		BYTE c = 0;
		for (BYTE i = 0; i < 26; i++) {
			if (all & d) {
				++c;
			}
			d *= 2;
		}
		if (c > 0) {
			result = new drive[c];
			d = 1;
			c = 0;
			for (BYTE i = 0; i < 26; i++) {
				if (all & d) {
					result[c].v = p[0] = v + i;
					result[c].type = (BYTE)GetDriveTypeA(p);
					++c;
				}
				d *= 2;
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
		drive* result;
	};
	static napi_value sync(napi_env env, napi_callback_info info) {
		napi_value result;
		napi_get_new_target(env, info, &result);
		if (result) {
			result = NULL;
			napi_throw_error(env, SYB_EXP_INVAL, SYB_ERR_NOT_A_CONSTRUCTOR);
		} else {
			result = convert(env, func());
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
			napi_value argv, self;
			size_t argc = 1;
			napi_get_cb_info(env, info, &argc, &argv, &self, NULL);
			if (argc >= 1) {
				napi_valuetype t;
				napi_typeof(env, argv, &t);
				if (t == napi_function) {
					cbdata* data = new cbdata;
					napi_value tmp;
					napi_create_reference(env, argv, 1, &data->cb);
					napi_create_reference(env, self, 1, &data->self);
					napi_create_string_latin1(env, "fswin.getLogicalDriveList", NAPI_AUTO_LENGTH, &tmp);
					napi_create_async_work(env, NULL, tmp, execute, complete, data, &data->work);
					if (napi_queue_async_work(env, data->work) == napi_ok) {
						napi_get_boolean(env, true, &result);
					} else {
						napi_get_boolean(env, false, &result);
						napi_delete_reference(env, data->cb);
						napi_delete_reference(env, data->self);
						napi_delete_async_work(env, data->work);
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
	static napi_value convert(napi_env env, drive* data) {
		napi_value name, value, result;
		napi_create_object(env, &result);
		if (data) {
			size_t c = _msize(data) / sizeof(drive);
			for (BYTE i = 0; i < c; i++) {
				const char* type;
				if (data[i].type == DRIVE_NO_ROOT_DIR) {
					type = "NO_ROOT_DIR";
				} else if (data[i].type == DRIVE_REMOVABLE) {
					type = "REMOVABLE";
				} else if (data[i].type == DRIVE_FIXED) {
					type = "FIXED";
				} else if (data[i].type == DRIVE_REMOTE) {
					type = "REMOTE";
				} else if (data[i].type == DRIVE_CDROM) {
					type = "CDROM";
				} else if (data[i].type == DRIVE_RAMDISK) {
					type = "RAMDISK";
				} else {
					type = "UNKNOWN";
				}
				napi_create_string_latin1(env, &data[i].v, 1, &name);
				napi_create_string_latin1(env, type, NAPI_AUTO_LENGTH, &value);
				napi_set_property(env, result, name, value);
			}
		}
		delete[]data;
		return result;
	}
	static void execute(napi_env env, void* data) {
		cbdata* d = (cbdata*)data;
		d->result = func();
	}
	static void complete(napi_env env, napi_status status, void* data) {
		cbdata* d = (cbdata*)data;
		napi_value cb, self, argv;
		napi_get_reference_value(env, d->cb, &cb);
		napi_get_reference_value(env, d->self, &self);
		if (d->result) {
			argv = convert(env, d->result);
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