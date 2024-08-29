#pragma once
#include "common.h"

class getVolumeInformation {
public:
	const struct infor {
		wchar_t label[MAX_PATH + 1];
		wchar_t fileSystem[MAX_PATH + 1];
		DWORD serialNumber;
		DWORD maximumComponentLength;
		DWORD fileSystemFlags;
	};
	static infor* func(const wchar_t* path) {//you need to delete the result yourself if it is not NULL
		infor* result = new infor;
		if (!GetVolumeInformationW(path, result->label, MAX_PATH + 1, &result->serialNumber, &result->maximumComponentLength, &result->fileSystemFlags, result->fileSystem, MAX_PATH + 1)) {
			delete result;
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
		wchar_t* path;
		infor* result;
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
				wchar_t* str = new wchar_t[str_len];
				napi_get_value_string_utf16(env, tmp, (char16_t*)str, str_len, NULL);
				infor* r = func(str);
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
					cbdata* data = new cbdata;
					size_t str_len;
					napi_value tmp;
					napi_create_reference(env, argv[1], 1, &data->cb);
					napi_create_reference(env, self, 1, &data->self);
					napi_coerce_to_string(env, argv[0], &tmp);
					napi_get_value_string_utf16(env, tmp, NULL, 0, &str_len);
					str_len += 1;
					data->path = new wchar_t[str_len];
					napi_get_value_string_utf16(env, tmp, (char16_t*)data->path, str_len, NULL);
					napi_create_string_latin1(env, "fswin.getVolumeInformation", NAPI_AUTO_LENGTH, &tmp);
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
	static napi_value convert(napi_env env, infor* data) {
		napi_value tmp, result;
		napi_create_object(env, &result);
		napi_create_string_utf16(env, (char16_t*)data->label, NAPI_AUTO_LENGTH, &tmp);
		napi_set_named_property(env, result, "LABEL", tmp);
		napi_create_string_utf16(env, (char16_t*)data->fileSystem, NAPI_AUTO_LENGTH, &tmp);
		napi_set_named_property(env, result, "FILESYSTEM", tmp);
		napi_create_uint32(env, data->serialNumber, &tmp);
		napi_set_named_property(env, result, "SERIALNUMBER", tmp);
		napi_create_uint32(env, data->fileSystemFlags, &tmp);
		napi_set_named_property(env, result, "RAW_FLAGS", tmp);
		napi_get_boolean(env, data->fileSystemFlags & FILE_CASE_SENSITIVE_SEARCH, &tmp);
		napi_set_named_property(env, result, "CASE_SENSITIVE_SEARCH", tmp);
		napi_get_boolean(env, data->fileSystemFlags & FILE_CASE_PRESERVED_NAMES, &tmp);
		napi_set_named_property(env, result, "CASE_PRESERVED_NAMES", tmp);
		napi_get_boolean(env, data->fileSystemFlags & FILE_UNICODE_ON_DISK, &tmp);
		napi_set_named_property(env, result, "UNICODE_ON_DISK", tmp);
		napi_get_boolean(env, data->fileSystemFlags & FILE_PERSISTENT_ACLS, &tmp);
		napi_set_named_property(env, result, "PERSISTENT_ACLS", tmp);
		napi_get_boolean(env, data->fileSystemFlags & FILE_FILE_COMPRESSION, &tmp);
		napi_set_named_property(env, result, "FILE_COMPRESSION", tmp);
		napi_get_boolean(env, data->fileSystemFlags & FILE_VOLUME_QUOTAS, &tmp);
		napi_set_named_property(env, result, "VOLUME_QUOTAS", tmp);
		napi_get_boolean(env, data->fileSystemFlags & FILE_RETURNS_CLEANUP_RESULT_INFO, &tmp);
		napi_set_named_property(env, result, "RETURNS_CLEANUP_RESULT_INFO", tmp);
		napi_get_boolean(env, data->fileSystemFlags & FILE_VOLUME_IS_COMPRESSED, &tmp);
		napi_set_named_property(env, result, "VOLUME_IS_COMPRESSED", tmp);
		napi_get_boolean(env, data->fileSystemFlags & FILE_NAMED_STREAMS, &tmp);
		napi_set_named_property(env, result, "NAMED_STREAMS", tmp);
		napi_get_boolean(env, data->fileSystemFlags & FILE_READ_ONLY_VOLUME, &tmp);
		napi_set_named_property(env, result, "READ_ONLY_VOLUME", tmp);
		napi_get_boolean(env, data->fileSystemFlags & FILE_SEQUENTIAL_WRITE_ONCE, &tmp);
		napi_set_named_property(env, result, "SEQUENTIAL_WRITE_ONCE", tmp);
		napi_get_boolean(env, data->fileSystemFlags & FILE_DAX_VOLUME, &tmp);
		napi_set_named_property(env, result, "DAX_VOLUME", tmp);
		napi_get_boolean(env, data->fileSystemFlags & FILE_SUPPORTS_SPARSE_FILES, &tmp);
		napi_set_named_property(env, result, "SUPPORTS_SPARSE_FILES", tmp);
		napi_get_boolean(env, data->fileSystemFlags & FILE_SUPPORTS_REPARSE_POINTS, &tmp);
		napi_set_named_property(env, result, "SUPPORTS_REPARSE_POINTS", tmp);
		napi_get_boolean(env, data->fileSystemFlags & FILE_SUPPORTS_REMOTE_STORAGE, &tmp);
		napi_set_named_property(env, result, "SUPPORTS_REMOTE_STORAGE", tmp);
		napi_get_boolean(env, data->fileSystemFlags & FILE_SUPPORTS_POSIX_UNLINK_RENAME, &tmp);
		napi_set_named_property(env, result, "SUPPORTS_POSIX_UNLINK_RENAME", tmp);
		napi_get_boolean(env, data->fileSystemFlags & FILE_SUPPORTS_BYPASS_IO, &tmp);
		napi_set_named_property(env, result, "SUPPORTS_BYPASS_IO", tmp);
		napi_get_boolean(env, data->fileSystemFlags & FILE_SUPPORTS_OBJECT_IDS, &tmp);
		napi_set_named_property(env, result, "SUPPORTS_OBJECT_IDS", tmp);
		napi_get_boolean(env, data->fileSystemFlags & FILE_SUPPORTS_ENCRYPTION, &tmp);
		napi_set_named_property(env, result, "SUPPORTS_ENCRYPTION", tmp);
		napi_get_boolean(env, data->fileSystemFlags & FILE_SUPPORTS_TRANSACTIONS, &tmp);
		napi_set_named_property(env, result, "SUPPORTS_TRANSACTIONS", tmp);
		napi_get_boolean(env, data->fileSystemFlags & FILE_SUPPORTS_HARD_LINKS, &tmp);
		napi_set_named_property(env, result, "SUPPORTS_HARD_LINKS", tmp);
		napi_get_boolean(env, data->fileSystemFlags & FILE_SUPPORTS_EXTENDED_ATTRIBUTES, &tmp);
		napi_set_named_property(env, result, "SUPPORTS_EXTENDED_ATTRIBUTES", tmp);
		napi_get_boolean(env, data->fileSystemFlags & FILE_SUPPORTS_OPEN_BY_FILE_ID, &tmp);
		napi_set_named_property(env, result, "SUPPORTS_OPEN_BY_FILE_ID", tmp);
		napi_get_boolean(env, data->fileSystemFlags & FILE_SUPPORTS_USN_JOURNAL, &tmp);
		napi_set_named_property(env, result, "SUPPORTS_USN_JOURNAL", tmp);
		napi_get_boolean(env, data->fileSystemFlags & FILE_SUPPORTS_INTEGRITY_STREAMS, &tmp);
		napi_set_named_property(env, result, "SUPPORTS_INTEGRITY_STREAMS", tmp);
		napi_get_boolean(env, data->fileSystemFlags & FILE_SUPPORTS_BLOCK_REFCOUNTING, &tmp);
		napi_set_named_property(env, result, "SUPPORTS_BLOCK_REFCOUNTING", tmp);
		napi_get_boolean(env, data->fileSystemFlags & FILE_SUPPORTS_SPARSE_VDL, &tmp);
		napi_set_named_property(env, result, "SUPPORTS_SPARSE_VDL", tmp);
		napi_get_boolean(env, data->fileSystemFlags & FILE_SUPPORTS_GHOSTING, &tmp);
		napi_set_named_property(env, result, "SUPPORTS_GHOSTING", tmp);

		return result;
	}
	static void execute(napi_env env, void* data) {
		cbdata* d = (cbdata*)data;
		d->result = func(d->path);
	}
	static void complete(napi_env env, napi_status status, void* data) {
		cbdata* d = (cbdata*)data;
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