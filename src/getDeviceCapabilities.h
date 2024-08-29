#pragma once
#include "common.h"

class getDeviceCapabilities {
public:
	static DWORD func(wchar_t* devid) {
		HDEVINFO hDevInfo = SetupDiGetClassDevsW(NULL, devid, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE | DIGCF_ALLCLASSES);
		DWORD cap = 0;
		SP_DEVINFO_DATA spdd;
		spdd.cbSize = sizeof(SP_DEVINFO_DATA);
		if (SetupDiEnumDeviceInfo(hDevInfo, 0, &spdd)) {
			SetupDiGetDeviceRegistryPropertyA(hDevInfo, &spdd, SPDRP_CAPABILITIES, NULL, (BYTE*)&cap, sizeof(DWORD), NULL);
		}
		SetupDiDestroyDeviceInfoList(hDevInfo);
		return cap;
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
		wchar_t* DevInstId;
		DWORD result;
	};
	static napi_value convert(napi_env env, DWORD data) {
		napi_value tmp, result;
		napi_create_object(env, &result);
		napi_get_boolean(env, data & CM_DEVCAP_LOCKSUPPORTED, &tmp);
		napi_set_named_property(env, result, "LOCK_SUPPORTED", tmp);
		napi_get_boolean(env, data & CM_DEVCAP_EJECTSUPPORTED, &tmp);
		napi_set_named_property(env, result, "EJECT_SUPPORTED", tmp);
		napi_get_boolean(env, data & CM_DEVCAP_REMOVABLE, &tmp);
		napi_set_named_property(env, result, "REMOVABLE", tmp);
		napi_get_boolean(env, data & CM_DEVCAP_DOCKDEVICE, &tmp);
		napi_set_named_property(env, result, "DOCK_DEVICE", tmp);
		napi_get_boolean(env, data & CM_DEVCAP_UNIQUEID, &tmp);
		napi_set_named_property(env, result, "UNIQUE_ID", tmp);
		napi_get_boolean(env, data & CM_DEVCAP_SILENTINSTALL, &tmp);
		napi_set_named_property(env, result, "SILENT_INSTALL", tmp);
		napi_get_boolean(env, data & CM_DEVCAP_RAWDEVICEOK, &tmp);
		napi_set_named_property(env, result, "RAW_DEVICE_OK", tmp);
		napi_get_boolean(env, data & CM_DEVCAP_SURPRISEREMOVALOK, &tmp);
		napi_set_named_property(env, result, "SURPRISE_REMOVAL_OK", tmp);
		napi_get_boolean(env, data & CM_DEVCAP_HARDWAREDISABLED, &tmp);
		napi_set_named_property(env, result, "HARDWARE_DISABLED", tmp);
		napi_get_boolean(env, data & CM_DEVCAP_NONDYNAMIC, &tmp);
		napi_set_named_property(env, result, "NON_DYNAMIC", tmp);
		return result;
	}
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
				napi_value tmp;
				napi_coerce_to_string(env, argv, &tmp);
				size_t str_len;
				napi_get_value_string_utf16(env, tmp, NULL, 0, &str_len);
				str_len += 1;
				wchar_t* DevInstId = new wchar_t[str_len];
				napi_get_value_string_utf16(env, tmp, (char16_t*)DevInstId, str_len, NULL);
				result = convert(env, func(DevInstId));
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
					cbdata* data = new cbdata;
					napi_value tmp;
					napi_coerce_to_string(env, argv[0], &tmp);
					size_t str_len;
					napi_get_value_string_utf16(env, tmp, NULL, 0, &str_len);
					str_len += 1;
					data->DevInstId = new wchar_t[str_len];
					napi_get_value_string_utf16(env, tmp, (char16_t*)data->DevInstId, str_len, NULL);
					napi_create_reference(env, argv[1], 1, &data->cb);
					napi_create_reference(env, self, 1, &data->self);
					napi_create_string_latin1(env, "fswin.getDeviceCapabilities", NAPI_AUTO_LENGTH, &tmp);
					napi_create_async_work(env, NULL, tmp, execute, complete, data, &data->work);
					if (napi_queue_async_work(env, data->work) == napi_ok) {
						napi_get_boolean(env, true, &result);
					} else {
						napi_get_boolean(env, false, &result);
						napi_delete_reference(env, data->cb);
						napi_delete_reference(env, data->self);
						napi_delete_async_work(env, data->work);
						delete[]data->DevInstId;
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
	static void execute(napi_env env, void* data) {
		cbdata* d = (cbdata*)data;
		d->result = func(d->DevInstId);
	}
	static void complete(napi_env env, napi_status status, void* data) {
		cbdata* d = (cbdata*)data;
		napi_value cb, self, argv = convert(env, d->result);
		napi_get_reference_value(env, d->cb, &cb);
		napi_get_reference_value(env, d->self, &self);
		napi_call_function(env, self, cb, 1, &argv, NULL);
		napi_delete_reference(env, d->cb);
		napi_delete_reference(env, d->self);
		napi_delete_async_work(env, d->work);
		delete[]d->DevInstId;
		delete d;
	}
};