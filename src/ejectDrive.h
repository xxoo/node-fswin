#pragma once
#include "common.h"

class ejectDrive {
public:
	static bool func(const char l, const uint32_t method = 0) {
		bool result = false;
		if (method) {
			DEVINST DevInst = getDriveDevice::getParentDevInst(l);
			if (DevInst) {
				wchar_t* devid = getDriveDevice::getDevInstIdByDevInst(DevInst);
				if (devid) {
					if (method == 1) {
						DWORD cap = getDeviceCapabilities::func(devid);
						if (cap & CM_DEVCAP_SURPRISEREMOVALOK) {
							if (cap & CM_DEVCAP_DOCKDEVICE) {
								ensurePrivilege("SeLoadDriverPrivilege");
							} else {
								ensurePrivilege("SeUndockPrivilege");
							}
							result = CM_Request_Device_EjectW(DevInst, NULL, NULL, 0, 0) == CR_SUCCESS;
						} else {
							ensurePrivilege("SeLoadDriverPrivilege");
							result = CM_Query_And_Remove_SubTreeW(DevInst, NULL, NULL, 0, CM_REMOVE_NO_RESTART) == CR_SUCCESS;
						}
					} else if (HotPlugEjectDevice) {
						result = HotPlugEjectDevice(NULL, devid, 0) == CR_SUCCESS;
					}
					delete[]devid;
				}
			}
		} else {
			char p1[] = { '\\', '\\', '?', '\\', l, ':', 0 };
			HANDLE handle = CreateFileA(p1, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
			if (handle != INVALID_HANDLE_VALUE) {
				DWORD bytes = 0;
				PREVENT_MEDIA_REMOVAL d;
				d.PreventMediaRemoval = FALSE;
				if (DeviceIoControl(handle, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &bytes, NULL) && DeviceIoControl(handle, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &bytes, NULL) && DeviceIoControl(handle, IOCTL_STORAGE_MEDIA_REMOVAL, &d, sizeof(PREVENT_MEDIA_REMOVAL), NULL, 0, &bytes, NULL) && DeviceIoControl(handle, IOCTL_STORAGE_EJECT_MEDIA, NULL, 0, NULL, 0, &bytes, NULL)) {
					result = true;
				}
				CloseHandle(handle);
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
	typedef CONFIGRET(WINAPI* HPED)(HWND hWnd, PCWSTR DeviceInstanceId, DWORD dwFlags);
	static HPED HotPlugEjectDevice;
	const struct cbdata {
		napi_async_work work;
		napi_ref self;
		napi_ref cb;
		char l;
		uint32_t method;
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
			if (argc >= 2) {
				size_t str_len;
				napi_value tmp;
				napi_coerce_to_string(env, argv[0], &tmp);
				napi_get_value_string_latin1(env, tmp, NULL, 0, &str_len);
				if (str_len > 0) {
					char path[2];
					napi_get_value_string_latin1(env, tmp, path, 2, NULL);
					if ((path[0] >= 'A' && path[0] <= 'Z') || (path[0] >= 'a' && path[0] <= 'z')) {
						uint32_t method;
						napi_coerce_to_number(env, argv[1], &tmp);
						napi_get_value_uint32(env, tmp, &method);
						napi_get_boolean(env, func(path[0], method), &result);
					}
				}
			}
			if (!result) {
				napi_throw_error(env, SYB_EXP_INVAL, SYB_ERR_WRONG_ARGUMENTS);
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
			if (argc >= 3) {
				napi_valuetype t;
				napi_typeof(env, argv[2], &t);
				if (t == napi_function) {
					cbdata* data = new cbdata;
					size_t str_len;
					napi_value tmp;
					napi_coerce_to_number(env, argv[1], &tmp);
					napi_get_value_uint32(env, tmp, &data->method);
					napi_coerce_to_string(env, argv[0], &tmp);
					napi_get_value_string_latin1(env, tmp, NULL, 0, &str_len);
					if (str_len > 0) {
						char path[2];
						napi_get_value_string_latin1(env, tmp, path, 2, NULL);
						if ((path[0] >= 'A' && path[0] <= 'Z') || (path[0] >= 'a' && path[0] <= 'z')) {
							data->l = path[0];
							napi_create_reference(env, argv[2], 1, &data->cb);
							napi_create_reference(env, self, 1, &data->self);
							napi_create_string_latin1(env, "fswin.ejectDrive", NAPI_AUTO_LENGTH, &tmp);
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
				}
			}
			if (!result) {
				napi_throw_error(env, SYB_EXP_INVAL, SYB_ERR_WRONG_ARGUMENTS);
			}
		}
		return result;
	}
	static void execute(napi_env env, void* data) {
		cbdata* d = (cbdata*)data;
		d->result = func(d->l, d->method);
	}
	static void complete(napi_env env, napi_status status, void* data) {
		cbdata* d = (cbdata*)data;
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

ejectDrive::HPED ejectDrive::HotPlugEjectDevice = (ejectDrive::HPED)GetProcAddress(LoadLibraryA("HotPlug.dll"), "HotPlugEjectDevice");