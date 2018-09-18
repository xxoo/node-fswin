#pragma once
#include "common.h"

class find {
public:
	const struct resultData {//this is a linked table
		WIN32_FIND_DATAW data;
		resultData *next;
	};
	//progressive callback type, if this callback returns true, the search will stop immediately. the contents of info will be rewritten or released after the callback returns, so make a copy before starting a new thread if you still need to use it
	typedef bool(*findResultCall)(const WIN32_FIND_DATAW *info, void *data);
	static resultData *func(const wchar_t *path) {//you have to free every linked data yourself if it is not NULL
		resultData *result = (resultData*)malloc(sizeof(resultData));
		HANDLE hnd = FindFirstFileExW(path, FindExInfoStandard, &result->data, FindExSearchNameMatch, NULL, NULL);
		if (hnd == INVALID_HANDLE_VALUE) {
			free(result);
			result = NULL;
		} else {
			resultData *resultnew, *resultold;
			if (isValidInfo(&result->data)) {
				resultnew = (resultData*)malloc(sizeof(resultData));
				resultold = result;
			} else {
				resultnew = result;
				resultold = NULL;
				result = NULL;
			}
			while (FindNextFileW(hnd, &resultnew->data)) {
				if (isValidInfo(&resultnew->data)) {
					if (resultold) {
						resultold->next = resultnew;
					} else {
						result = resultnew;
					}
					resultold = resultnew;
					resultnew = new resultData;
				}
			}
			resultold->next = NULL;
			FindClose(hnd);
			if (resultnew != result) {
				free(resultnew);
			}
		}
		return result;
	}
	static DWORD funcWithCallback(const wchar_t *path, const findResultCall callback, void *data) {//data could be anything that will directly pass to the callback
		WIN32_FIND_DATAW info;
		HANDLE hnd = FindFirstFileExW(path, FindExInfoStandard, &info, FindExSearchNameMatch, NULL, NULL);
		DWORD result = 0;
		bool stop = false;
		if (hnd != INVALID_HANDLE_VALUE) {
			if (isValidInfo(&info)) {
				stop = callback(&info, data);
				result++;
			}
			while (!stop && FindNextFileW(hnd, &info)) {
				if (isValidInfo(&info)) {
					stop = callback(&info, data);
					result++;
				}
			}
			FindClose(hnd);
		}
		return result;
	}
	static napi_value init(napi_env env, bool isSync = false) {
		napi_value f;
		napi_create_function(env, NULL, 0, isSync ? sync : async, NULL, &f);
		return f;
	}
private:
	const struct syncCbData {
		napi_env env;
		napi_value self;
		napi_value cb;
	};
	const struct asyncCbData {
		napi_async_work work;
		napi_ref self;
		napi_ref cb;
		void *data;
		//the following data is only used in progressive mode
		HANDLE hnd;
		size_t count;
		bool stop;
	};
	static napi_value sync(napi_env env, napi_callback_info info) {
		napi_value result;
		napi_get_new_target(env, info, &result);
		if (result) {
			result = NULL;
			napi_throw_error(env, SYB_EXP_INVAL, SYB_ERR_NOT_A_CONSTRUCTOR);
		} else {
			napi_value argv[2], self;
			size_t argc;
			napi_get_cb_info(env, info, &argc, argv, &self, NULL);
			if (argc < 1) {
				napi_throw_error(env, SYB_EXP_INVAL, SYB_ERR_WRONG_ARGUMENTS);
			} else {
				size_t str_len;
				napi_value tmp, cb = NULL;
				napi_coerce_to_string(env, argv[0], &tmp);
				napi_get_value_string_utf16(env, tmp, NULL, 0, &str_len);
				str_len += 1;
				wchar_t *str = (wchar_t*)malloc(sizeof(wchar_t) * str_len);
				napi_get_value_string_utf16(env, tmp, (char16_t*)str, str_len, NULL);
				if (argc > 1) {
					napi_valuetype t;
					napi_typeof(env, argv[1], &t);
					if (t == napi_function) {
						cb = argv[1];
					}
				}
				if (cb) {
					syncCbData d = { env, self, cb };
					napi_create_int64(env, (int64_t)funcWithCallback(str, syncCallback, &d), &result);
				} else {
					result = resultDataToArray(env, func(str));
				}
				free(str);
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
			if (argc < 2) {
				napi_throw_error(env, SYB_EXP_INVAL, SYB_ERR_WRONG_ARGUMENTS);
			} else {
				napi_valuetype t;
				napi_typeof(env, argv[1], &t);
				if (t == napi_function) {
					bool isProgressive = false;
					asyncCbData *data = (asyncCbData*)malloc(sizeof(asyncCbData));
					size_t str_len;
					napi_value tmp;
					napi_coerce_to_string(env, argv[0], &tmp);
					napi_get_value_string_utf16(env, tmp, NULL, 0, &str_len);
					str_len += 1;
					data->data = malloc(sizeof(wchar_t) * str_len);
					napi_get_value_string_utf16(env, tmp, (char16_t*)data->data, str_len, NULL);
					if (argc > 2) {
						napi_coerce_to_bool(env, argv[2], &tmp);
						napi_get_value_bool(env, tmp, &isProgressive);
					}
					napi_create_reference(env, argv[1], 1, &data->cb);
					napi_create_reference(env, self, 1, &data->self);
					if (isProgressive) {
						data->hnd = INVALID_HANDLE_VALUE;
						data->count = 0;
						data->stop = false;
					} else {
						data->hnd = NULL;
					}
					napi_create_string_latin1(env, "fswin.find", NAPI_AUTO_LENGTH, &tmp);
					napi_create_async_work(env, argv[0], tmp, execute, complete, data, &data->work);
					if (napi_queue_async_work(env, data->work) == napi_ok) {
						napi_get_boolean(env, true, &result);
					} else {
						napi_get_boolean(env, false, &result);
						napi_delete_async_work(env, data->work);
						napi_delete_reference(env, data->cb);
						napi_delete_reference(env, data->self);
						free(data->data);
						free(data);
					}
				} else {
					napi_throw_error(env, SYB_EXP_INVAL, SYB_ERR_WRONG_ARGUMENTS);
				}
			}
		}
		return result;
	}
	static bool isValidInfo(const WIN32_FIND_DATAW *info) {//determine whether it is the real content 
		return wcscmp(info->cFileName, L".") != 0 && wcscmp(info->cFileName, L"..") != 0;
	}
	static napi_value resultDataToArray(napi_env env, resultData *r) {
		napi_value result;
		napi_create_array(env, &result);
		if (r) {
			napi_value push, tmp;
			napi_get_prototype(env, result, &tmp);
			napi_get_named_property(env, tmp, "push", &push);
			while (r) {
				tmp = convert(env, &r->data);
				napi_call_function(env, result, push, 1, &tmp, NULL);
				resultData *n = r->next;
				free(r);
				r = n;
			}
		}
		return result;
	}
	static napi_value convert(napi_env env, const WIN32_FIND_DATAW *info) {
		napi_value result, tmp, date;
		napi_create_object(env, &result);
		napi_get_global(env, &date);
		napi_get_named_property(env, date, "Date", &date);
		napi_create_string_utf16(env, (char16_t*)info->cFileName, wcslen(info->cFileName), &tmp);
		napi_set_named_property(env, result, "LONG_NAME", tmp);
		napi_create_string_utf16(env, (char16_t*)info->cFileName, wcslen(info->cAlternateFileName), &tmp);
		napi_set_named_property(env, result, "SHORT_NAME", tmp);
		napi_create_int64(env, fileTimeToJsDateVal(&info->ftCreationTime), &tmp);
		napi_new_instance(env, date, 1, &tmp, &tmp);
		napi_set_named_property(env, result, SYB_FILEATTR_CREATIONTIME, tmp);
		napi_create_int64(env, fileTimeToJsDateVal(&info->ftLastAccessTime), &tmp);
		napi_new_instance(env, date, 1, &tmp, &tmp);
		napi_set_named_property(env, result, SYB_FILEATTR_LASTACCESSTIME, tmp);
		napi_create_int64(env, fileTimeToJsDateVal(&info->ftLastWriteTime), &tmp);
		napi_new_instance(env, date, 1, &tmp, &tmp);
		napi_set_named_property(env, result, SYB_FILEATTR_LASTWRITETIME, tmp);
		napi_create_int64(env, combineHiLow(info->nFileSizeHigh, info->nFileSizeLow), &tmp);
		napi_set_named_property(env, result, SYB_FILEATTR_SIZE, tmp);
		napi_get_boolean(env, info->dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE, &tmp);
		napi_set_named_property(env, result, SYB_FILEATTR_ISARCHIVED, tmp);
		napi_get_boolean(env, info->dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED, &tmp);
		napi_set_named_property(env, result, SYB_FILEATTR_ISCOMPRESSED, tmp);
		napi_get_boolean(env, info->dwFileAttributes & FILE_ATTRIBUTE_DEVICE, &tmp);
		napi_set_named_property(env, result, SYB_FILEATTR_ISDEVICE, tmp);
		napi_get_boolean(env, info->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY, &tmp);
		napi_set_named_property(env, result, SYB_FILEATTR_ISDIRECTORY, tmp);
		napi_get_boolean(env, info->dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED, &tmp);
		napi_set_named_property(env, result, SYB_FILEATTR_ISENCRYPTED, tmp);
		napi_get_boolean(env, info->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN, &tmp);
		napi_set_named_property(env, result, SYB_FILEATTR_ISHIDDEN, tmp);
		napi_get_boolean(env, info->dwFileAttributes & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED, &tmp);
		napi_set_named_property(env, result, SYB_FILEATTR_ISNOTCONTENTINDEXED, tmp);
		napi_get_boolean(env, info->dwFileAttributes & FILE_ATTRIBUTE_OFFLINE, &tmp);
		napi_set_named_property(env, result, SYB_FILEATTR_ISOFFLINE, tmp);
		napi_get_boolean(env, info->dwFileAttributes & FILE_ATTRIBUTE_READONLY, &tmp);
		napi_set_named_property(env, result, SYB_FILEATTR_ISREADONLY, tmp);
		napi_get_boolean(env, info->dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE, &tmp);
		napi_set_named_property(env, result, SYB_FILEATTR_ISSPARSEFILE, tmp);
		napi_get_boolean(env, info->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM, &tmp);
		napi_set_named_property(env, result, SYB_FILEATTR_ISSYSTEM, tmp);
		napi_get_boolean(env, info->dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY, &tmp);
		napi_set_named_property(env, result, SYB_FILEATTR_ISTEMPORARY, tmp);
		napi_get_boolean(env, info->dwFileAttributes & FILE_ATTRIBUTE_INTEGRITY_STREAM, &tmp);
		napi_set_named_property(env, result, SYB_FILEATTR_ISINTEGERITYSTREAM, tmp);
		napi_get_boolean(env, info->dwFileAttributes & FILE_ATTRIBUTE_NO_SCRUB_DATA, &tmp);
		napi_set_named_property(env, result, SYB_FILEATTR_ISNOSCRUBDATA, tmp);
		char *tag;
		if (info->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
			if (info->dwReserved0 == IO_REPARSE_TAG_MOUNT_POINT) {
				tag = "MOUNT_POINT";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_HSM) {
				tag = "HSM";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_HSM2) {
				tag = "HSM2";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_SIS) {
				tag = "SIS";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_WIM) {
				tag = "WIM";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_CSV) {
				tag = "CSV";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_DFS) {
				tag = "DFS";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_SYMLINK) {
				tag = "SYMLINK";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_DFSR) {
				tag = "DFSR";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_DEDUP) {
				tag = "DEDUP";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_NFS) {
				tag = "NFS";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_FILE_PLACEHOLDER) {
				tag = "FILE_PLACEHOLDER";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_WOF) {
				tag = "WOF";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_WCI) {
				tag = "WCI";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_WCI_1) {
				tag = "WCI_1";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_GLOBAL_REPARSE) {
				tag = "GLOBAL_REPARSE";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_CLOUD) {
				tag = "CLOUD";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_CLOUD_1) {
				tag = "CLOUD_1";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_CLOUD_2) {
				tag = "CLOUD_2";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_CLOUD_3) {
				tag = "CLOUD_3";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_CLOUD_4) {
				tag = "CLOUD_4";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_CLOUD_5) {
				tag = "CLOUD_5";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_CLOUD_6) {
				tag = "CLOUD_6";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_CLOUD_7) {
				tag = "CLOUD_7";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_CLOUD_8) {
				tag = "CLOUD_8";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_CLOUD_9) {
				tag = "CLOUD_9";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_CLOUD_A) {
				tag = "CLOUD_A";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_CLOUD_B) {
				tag = "CLOUD_B";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_CLOUD_C) {
				tag = "CLOUD_C";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_CLOUD_D) {
				tag = "CLOUD_D";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_CLOUD_E) {
				tag = "CLOUD_E";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_CLOUD_F) {
				tag = "CLOUD_F";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_CLOUD_MASK) {
				tag = "CLOUD_MASK";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_APPEXECLINK) {
				tag = "APPEXECLINK";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_GVFS) {
				tag = "GVFS";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_STORAGE_SYNC) {
				tag = "STORAGE_SYNC";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_WCI_TOMBSTONE) {
				tag = "WCI_TOMBSTONE";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_UNHANDLED) {
				tag = "UNHANDLED";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_ONEDRIVE) {
				tag = "ONEDRIVE";
			} else if (info->dwReserved0 == IO_REPARSE_TAG_GVFS_TOMBSTONE) {
				tag = "TOMBSTONE";
			} else {
				tag = "UNKNOWN";
			}
		} else {
			tag = NULL;
		}
		napi_create_string_latin1(env, tag, tag ? NAPI_AUTO_LENGTH : 0, &tmp);
		napi_set_named_property(env, result, "REPARSE_POINT_TAG", tmp);
		return result;
	}
	static bool syncCallback(const WIN32_FIND_DATAW *info, void *data) {
		bool result;
		syncCbData *d = (syncCbData*)data;
		napi_value res, o = convert(d->env, info);
		napi_call_function(d->env, d->self, d->cb, 1, &o, &res);
		napi_coerce_to_bool(d->env, res, &res);
		napi_get_value_bool(d->env, res, &result);
		return result;
	}
	static void execute(napi_env env, void *data) {
		asyncCbData *d = (asyncCbData*)data;
		if (d->hnd) {
			WIN32_FIND_DATAW *info = (WIN32_FIND_DATAW*)malloc(sizeof(WIN32_FIND_DATAW));
			if (d->hnd == INVALID_HANDLE_VALUE) {
				d->hnd = FindFirstFileExW((wchar_t*)d->data, FindExInfoStandard, info, FindExSearchNameMatch, NULL, NULL);
				free(d->data);
				if (d->hnd != INVALID_HANDLE_VALUE) {
					while (!isValidInfo(info)) {
						if (!FindNextFileW(d->hnd, info)) {
							FindClose(d->hnd);
							d->hnd = INVALID_HANDLE_VALUE;
							break;
						}
					}
				}
			} else {
				if (d->stop) {
					FindClose(d->hnd);
					d->hnd = INVALID_HANDLE_VALUE;
				} else {
					if (FindNextFileW(d->hnd, info)) {
						while (!isValidInfo(info)) {
							if (!FindNextFileW(d->hnd, info)) {
								FindClose(d->hnd);
								d->hnd = INVALID_HANDLE_VALUE;
								break;
							}
						}
					} else {
						FindClose(d->hnd);
						d->hnd = INVALID_HANDLE_VALUE;
					}
				}
			}
			if (d->hnd == INVALID_HANDLE_VALUE) {
				free(info);
			} else {
				d->data = info;
			}
		} else {
			resultData *rdata = func((wchar_t*)d->data);
			free(d->data);
			d->data = rdata;
		}
	}
	static void complete(napi_env env, napi_status status, void *data) {
		asyncCbData *d = (asyncCbData*)data;
		napi_value cb, self;
		napi_get_reference_value(env, d->cb, &cb);
		napi_get_reference_value(env, d->self, &self);
		BYTE finish = 0;
		if (status == napi_ok) {
			if (d->hnd) {
				napi_value result, argv[2];
				if (d->hnd == INVALID_HANDLE_VALUE) {
					napi_create_string_latin1(env, "SUCCEEDED", NAPI_AUTO_LENGTH, &argv[0]);
					napi_create_int64(env, (int64_t)d->count, &argv[1]);
					finish = 1;
				} else {
					WIN32_FIND_DATAW *info = (WIN32_FIND_DATAW*)d->data;
					if (d->stop) {
						napi_create_string_latin1(env, "INTERRUPTED", NAPI_AUTO_LENGTH, &argv[0]);
						napi_create_int64(env, (int64_t)d->count, &argv[1]);
						finish = 1;
					} else {
						d->count++;
						napi_create_string_latin1(env, "FOUND", NAPI_AUTO_LENGTH, &argv[0]);
						argv[1] = convert(env, info);
						if (napi_queue_async_work(env, d->work) != napi_ok) {
							finish = 2;
						}
					}
					free(info);
				}
				napi_call_function(env, self, cb, 2, (napi_value*)&argv, &result);
				napi_coerce_to_bool(env, result, &result);
				napi_get_value_bool(env, result, &d->stop);
			} else {
				napi_value argv = resultDataToArray(env, (resultData*)d->data);
				napi_call_function(env, self, cb, 1, &argv, NULL);
				finish = 1;
			}
		} else {
			if (d->hnd) {
				finish = 2;
				if (d->hnd != INVALID_HANDLE_VALUE) {
					FindClose(d->hnd);
				}
			} else {
				finish = 1;
				napi_value argv;
				napi_get_null(env, &argv);
				napi_call_function(env, self, cb, 1, &argv, NULL);
			}
		}
		if (finish) {
			if (finish > 1) {
				napi_value argv[2];
				napi_create_string_latin1(env, "FAILED", NAPI_AUTO_LENGTH, &argv[0]);
				napi_create_int64(env, (int64_t)d->count, &argv[1]);
				napi_call_function(env, self, cb, 2, (napi_value*)&argv, NULL);
			}
			napi_delete_reference(env, d->cb);
			napi_delete_reference(env, d->self);
			napi_delete_async_work(env, d->work);
			free(d);
		}
	}
};