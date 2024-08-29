#pragma once
#include "common.h"

class getDriveDevice {
public:
	struct infor {
		DWORD deviceNumber;
		SP_DEVICE_INTERFACE_DETAIL_DATA_A* pspdidd;
		wchar_t* deviceId;
		wchar_t* parentDeviceId;
	};
	static DEVINST getParentDevInst(const char l, infor* inf = NULL) {
		DEVINST DevInst = 0;
		if ((l >= 'A' && l <= 'Z') || (l >= 'a' && l <= 'z')) {
			char p1[] = { '\\', '\\', '?', '\\', l, ':', 0 };
			char* p2 = &p1[4];
			HANDLE hVolume = CreateFileA(p1, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
			if (hVolume != INVALID_HANDLE_VALUE) {
				STORAGE_DEVICE_NUMBER sdn;
				DWORD dwBytesReturned = 0;
				if (DeviceIoControl(hVolume, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &sdn, sizeof(STORAGE_DEVICE_NUMBER), &dwBytesReturned, NULL)) {
					CloseHandle(hVolume);
					UINT DriveType = GetDriveTypeA(p2);
					const GUID* guid = NULL;
					if (DriveType == DRIVE_REMOVABLE) {
						char szDosDeviceName[MAX_PATH + 1];
						if (QueryDosDeviceA(p2, szDosDeviceName, MAX_PATH + 1)) {
							guid = strstr(szDosDeviceName, "\\Floppy") ? &GUID_DEVINTERFACE_FLOPPY : &GUID_DEVINTERFACE_DISK;
						}
					} else if (DriveType == DRIVE_FIXED) {
						guid = &GUID_DEVINTERFACE_DISK;
					} else if (DriveType == DRIVE_CDROM) {
						guid = &GUID_DEVINTERFACE_CDROM;
					}
					if (guid) {
						HDEVINFO hDevInfo = SetupDiGetClassDevsA(guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
						if (hDevInfo != INVALID_HANDLE_VALUE) {
							DWORD dwSize = 0;
							DWORD dwIndex = 0;
							SP_DEVICE_INTERFACE_DATA spdid;
							SP_DEVINFO_DATA spdd;
							spdid.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
							spdd.cbSize = sizeof(SP_DEVINFO_DATA);
							while (!DevInst && SetupDiEnumDeviceInterfaces(hDevInfo, NULL, guid, dwIndex++, &spdid)) {
								SetupDiGetDeviceInterfaceDetailA(hDevInfo, &spdid, NULL, 0, &dwSize, NULL);
								if (dwSize > 0) {
									SP_DEVICE_INTERFACE_DETAIL_DATA_A* pspdidd = (SP_DEVICE_INTERFACE_DETAIL_DATA_A*)malloc(dwSize);
									if (pspdidd) {
										pspdidd->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);
										if (SetupDiGetDeviceInterfaceDetailA(hDevInfo, &spdid, pspdidd, dwSize, NULL, &spdd)) {
											HANDLE hDrive = CreateFileA(pspdidd->DevicePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
											if (hDrive != INVALID_HANDLE_VALUE) {
												STORAGE_DEVICE_NUMBER sdn1;
												DWORD dwBytesReturned = 0;
												if (DeviceIoControl(hDrive, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &sdn1, sizeof(STORAGE_DEVICE_NUMBER), &dwBytesReturned, NULL) && sdn.DeviceNumber == sdn1.DeviceNumber) {
													//std::cout << sdn.DeviceNumber << pspdidd->DevicePath << std::endl;
													DevInst = spdd.DevInst;
													if (inf) {
														inf->deviceNumber = sdn.DeviceNumber;
														inf->deviceId = getDevInstIdByDevInst(DevInst);
													}
												}
												CloseHandle(hDrive);
											}
										}
										if (inf) {
											inf->pspdidd = pspdidd;
										} else {
											free(pspdidd);
										}
									}
								}
							}
							SetupDiDestroyDeviceInfoList(hDevInfo);
							if (DevInst && CM_Get_Parent(&DevInst, DevInst, 0) != CR_SUCCESS) {
								DevInst = 0;
							}
						}
					}
				} else {
					CloseHandle(hVolume);
				}
			}
		}
		return DevInst;
	}
	static wchar_t* getDevInstIdByDevInst(DEVINST DevInst) {//delete the result yourself if it is not NULL
		wchar_t* result = NULL;
		ULONG sz;
		if (CM_Get_Device_ID_Size(&sz, DevInst, 0) == CR_SUCCESS) {
			++sz;
			result = new wchar_t[sz];
			if (CM_Get_Device_IDW(DevInst, result, sz, 0) != CR_SUCCESS) {
				delete[]result;
				result = NULL;
			}
		}
		return result;
	}
	static infor* func(const char l) {
		infor* inf = new infor({ 0 });
		DEVINST DevInst = getParentDevInst(l, inf);
		if (DevInst) {
			inf->parentDeviceId = getDevInstIdByDevInst(DevInst);
		} else {
			if (inf->pspdidd) {
				free(inf->pspdidd);
			}
			if (inf->deviceId) {
				delete[]inf->deviceId;
			}
			delete inf;
			inf = NULL;
		}
		return inf;
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
		char l;
		infor* result;
	};
	static napi_value convert(napi_env env, infor* inf) {
		napi_value result, tmp;
		napi_create_object(env, &result);
		napi_create_uint32(env, inf->deviceNumber, &tmp);
		napi_set_named_property(env, result, "deviceNumber", tmp);
		napi_create_string_latin1(env, inf->pspdidd->DevicePath, NAPI_AUTO_LENGTH, &tmp);
		napi_set_named_property(env, result, "devicePath", tmp);
		napi_create_string_utf16(env, (char16_t*)inf->deviceId, NAPI_AUTO_LENGTH, &tmp);
		napi_set_named_property(env, result, "deviceId", tmp);
		napi_create_string_utf16(env, (char16_t*)inf->parentDeviceId, NAPI_AUTO_LENGTH, &tmp);
		napi_set_named_property(env, result, "parentDeviceId", tmp);
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
				size_t str_len;
				napi_value tmp;
				napi_coerce_to_string(env, argv, &tmp);
				napi_get_value_string_latin1(env, tmp, NULL, 0, &str_len);
				if (str_len > 0) {
					char path[2];
					napi_get_value_string_latin1(env, tmp, path, 2, NULL);
					infor* inf = func(path[0]);
					if (inf) {
						result = convert(env, inf);
						free(inf->pspdidd);
						delete[]inf->deviceId;
						delete[]inf->parentDeviceId;
						delete inf;
					}
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
					cbdata* data = new cbdata;
					size_t str_len;
					napi_value tmp;
					napi_coerce_to_string(env, argv[0], &tmp);
					napi_get_value_string_latin1(env, tmp, NULL, 0, &str_len);
					if (str_len > 0) {
						char path[2];
						napi_get_value_string_latin1(env, tmp, path, 2, NULL);
						data->l = path[0];
						napi_create_reference(env, argv[1], 1, &data->cb);
						napi_create_reference(env, self, 1, &data->self);
						napi_create_string_latin1(env, "fswin.getDriveDeviceLocation", NAPI_AUTO_LENGTH, &tmp);
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
			if (!result) {
				napi_throw_error(env, SYB_EXP_INVAL, SYB_ERR_WRONG_ARGUMENTS);
			}
		}
		return result;
	}
	static void execute(napi_env env, void* data) {
		cbdata* d = (cbdata*)data;
		d->result = func(d->l);
	}
	static void complete(napi_env env, napi_status status, void* data) {
		cbdata* d = (cbdata*)data;
		napi_value cb, self, argv;
		napi_get_reference_value(env, d->cb, &cb);
		napi_get_reference_value(env, d->self, &self);
		if (d->result) {
			argv = convert(env, d->result);
			free(d->result->pspdidd);
			delete[]d->result->deviceId;
			delete[]d->result->parentDeviceId;
			delete d->result;
		} else {
			argv = NULL;
		}
		napi_call_function(env, self, cb, 1, &argv, NULL);
		napi_delete_reference(env, d->cb);
		napi_delete_reference(env, d->self);
		napi_delete_async_work(env, d->work);
		delete d;
	}
};