#pragma once
#include "common.h"

class getAttributes {
public:
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
		BY_HANDLE_FILE_INFORMATION *result;
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
				BY_HANDLE_FILE_INFORMATION data;
				CHAR bak;
				if ((void*)RtlSetThreadPlaceholderCompatibilityMode) {
					bak = RtlSetThreadPlaceholderCompatibilityMode(2);
				}
				HANDLE h = CreateFileW(str, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT, NULL);
				if (GetFileInformationByHandle(h, &data)) {
					result = convert(env, &data);
				} else {
					napi_get_null(env, &result);
				}
				if (h != INVALID_HANDLE_VALUE) {
					CloseHandle(h);
				}
				if ((void*)RtlSetThreadPlaceholderCompatibilityMode && bak != 2) {
					RtlSetThreadPlaceholderCompatibilityMode(bak);
				}
				delete[]str;
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
			if (argc < 2) {
				napi_throw_error(env, SYB_EXP_INVAL, SYB_ERR_WRONG_ARGUMENTS);
			} else {
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
					napi_create_string_latin1(env, "fswin.getAttributes", NAPI_AUTO_LENGTH, &tmp);
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
	static napi_value convert(napi_env env, BY_HANDLE_FILE_INFORMATION *info) {
		napi_value result, tmp, date;
		napi_get_global(env, &date);
		napi_get_named_property(env, date, "Date", &date);
		napi_create_object(env, &result);
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
		napi_create_int32(env, info->nNumberOfLinks, &tmp);
		napi_set_named_property(env, result, "LINK_COUNT", tmp);
		napi_create_int32(env, info->dwFileAttributes, &tmp);
		napi_set_named_property(env, result, SYB_FILEATTR_RAWATTRS, tmp);
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
		napi_get_boolean(env, info->dwFileAttributes & FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS, &tmp);
		napi_set_named_property(env, result, SYB_FILEATTR_ISRECALLONDATAACCESS, tmp);
		napi_get_boolean(env, info->dwFileAttributes & FILE_ATTRIBUTE_RECALL_ON_OPEN, &tmp);
		napi_set_named_property(env, result, SYB_FILEATTR_ISRECALLONOPEN, tmp);
		napi_get_boolean(env, info->dwFileAttributes & FILE_ATTRIBUTE_VIRTUAL, &tmp);
		napi_set_named_property(env, result, SYB_FILEATTR_ISVIRTUAL, tmp);
		napi_get_boolean(env, info->dwFileAttributes & FILE_ATTRIBUTE_EA, &tmp);
		napi_set_named_property(env, result, SYB_FILEATTR_ISEA, tmp);
		napi_get_boolean(env, info->dwFileAttributes & FILE_ATTRIBUTE_PINNED, &tmp);
		napi_set_named_property(env, result, SYB_FILEATTR_ISPINNED, tmp);
		napi_get_boolean(env, info->dwFileAttributes & FILE_ATTRIBUTE_UNPINNED, &tmp);
		napi_set_named_property(env, result, SYB_FILEATTR_ISUNPINNED, tmp);
		napi_get_boolean(env, info->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT, &tmp);
		napi_set_named_property(env, result, "IS_REPARSE_POINT", tmp);
		return result;
	}
	static void execute(napi_env env, void *data) {
		cbdata *d = (cbdata*)data;
		d->result = new BY_HANDLE_FILE_INFORMATION;
		CHAR bak;
		if ((void*)RtlSetThreadPlaceholderCompatibilityMode) {
			bak = RtlSetThreadPlaceholderCompatibilityMode(2);
		}
		HANDLE h = CreateFileW(d->path, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT, NULL);
		if (!GetFileInformationByHandle(h, d->result)) {
			delete d->result;
			d->result = NULL;
		}
		if (h != INVALID_HANDLE_VALUE) {
			CloseHandle(h);
		}
		if ((void*)RtlSetThreadPlaceholderCompatibilityMode && bak != 2) {
			RtlSetThreadPlaceholderCompatibilityMode(bak);
		}
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