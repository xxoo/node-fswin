#pragma once
#include "main.h"

#ifndef IO_REPARSE_TAG_DEDUP
#	define IO_REPARSE_TAG_DEDUP (0x80000013L)
#endif
#ifndef IO_REPARSE_TAG_NFS
#	define IO_REPARSE_TAG_NFS (0x80000014L)
#endif
#ifndef IO_REPARSE_TAG_FILE_PLACEHOLDER
#	define IO_REPARSE_TAG_FILE_PLACEHOLDER (0x80000015L)
#endif

#define SYB_EVT_FOUND "FOUND"
#define SYB_EVT_SUCCEEDED "SUCCEEDED"
#define SYB_EVT_FAILED "FAILED"
#define SYB_EVT_INTERRUPTED "INTERRUPTED"
#define SYB_RETURNS_LONGNAME "LONG_NAME"
#define SYB_RETURNS_SHORTNAME "SHORT_NAME"
#define SYB_RETURNS_REPARSEPOINTTAG "REPARSE_POINT_TAG"
#define SYB_REPARSEPOINT_UNKNOWN "UNKNOWN"
#define SYB_REPARSEPOINT_CSV "CSV"
#define SYB_REPARSEPOINT_DEDUP "DEDUP"
#define SYB_REPARSEPOINT_DFS "DFS"
#define SYB_REPARSEPOINT_DFSR "DFSR"
#define SYB_REPARSEPOINT_HSM "HSM"
#define SYB_REPARSEPOINT_HSM2 "HSM2"
#define SYB_REPARSEPOINT_MOUNTPOINT "MOUNT_POINT"
#define SYB_REPARSEPOINT_NFS "NFS"
#define SYB_REPARSEPOINT_PLACEHOLDER "PLACE_HOLDER"
#define SYB_REPARSEPOINT_SIS "SIS"
#define SYB_REPARSEPOINT_SYMLINK "SYMLINK"
#define SYB_REPARSEPOINT_WIM "WIM"

class find {
public:
	const struct resultData {//this is a linked table
		WIN32_FIND_DATAW data;
		resultData *next;
	};
	//progressive callback type, if this callback returns true, the search will stop immediately. the contents of info will be rewritten or released after the callback returns, so make a copy before starting a new thread if you still need to use it
	typedef bool(*findResultCall)(const WIN32_FIND_DATAW *info, void *data);
private:
	const struct jsCallbackData {
		Handle<Object> self;
		Handle<Function> func;
	};
	const struct workdata {
		uv_work_t req;
		Persistent<Object> self;
		Persistent<Function> func;
		void *data;
		//the following data is only used in progressive mode
		HANDLE hnd;
		size_t count;
		bool stop;
	};
public:
	static resultData *basic(const wchar_t *path) {//you have to delete every linked data yourself if it is not NULL
		resultData *result = new resultData;
		HANDLE hnd = FindFirstFileExW(path, FindExInfoStandard, &result->data, FindExSearchNameMatch, NULL, NULL);
		if (hnd == INVALID_HANDLE_VALUE) {
			delete result;
			result = NULL;
		} else {
			resultData *resultnew, *resultold;
			if (isValidInfo(&result->data)) {
				resultnew = new resultData;
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
				delete resultnew;
			}
		}
		return result;
	}
	static DWORD basicWithCallback(const wchar_t *path, const findResultCall callback, void *data) {//data could be anything that will directly pass to the callback
		WIN32_FIND_DATAW info;
		HANDLE hnd = FindFirstFileExW(path, FindExInfoStandard, &info, FindExSearchNameMatch, NULL, NULL);
		DWORD result = 0;
		bool stop = false;
		if (hnd != INVALID_HANDLE_VALUE) {
			if (isValidInfo(&info)) {
				stop = callback(&info, data);
				result++;
			}
			while (!stop&&FindNextFileW(hnd, &info)) {
				if (isValidInfo(&info)) {
					stop = callback(&info, data);
					result++;
				}
			}
			FindClose(hnd);
		}
		return result;
	}
	static Handle<Object> fileInfoToJs(const WIN32_FIND_DATAW *info) {//this function does not check whether info is NULL, make sure it is not before calling
		ISOLATE_NEW;
		SCOPE_ESCAPABLE;
		RETURNTYPE<String> tmp;
		RETURNTYPE<Object> o = Object::New(ISOLATE);
		o->Set(NEWSTRING(SYB_RETURNS_LONGNAME), NEWSTRING_TWOBYTES(info->cFileName));
		o->Set(NEWSTRING(SYB_RETURNS_SHORTNAME), NEWSTRING_TWOBYTES(info->cAlternateFileName));
		o->Set(NEWSTRING(SYB_FILEATTR_CREATIONTIME), Date::New(ISOLATE_C fileTimeToJsDateVal(&info->ftCreationTime)));
		o->Set(NEWSTRING(SYB_FILEATTR_LASTACCESSTIME), Date::New(ISOLATE_C fileTimeToJsDateVal(&info->ftLastAccessTime)));
		o->Set(NEWSTRING(SYB_FILEATTR_LASTWRITETIME), Date::New(ISOLATE_C fileTimeToJsDateVal(&info->ftLastWriteTime)));
		o->Set(NEWSTRING(SYB_FILEATTR_SIZE), Number::New(ISOLATE_C(double)combineHiLow(info->nFileSizeHigh, info->nFileSizeLow)));
		tmp = NEWSTRING(SYB_RETURNS_REPARSEPOINTTAG);
		if (info->dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT) {
			if (info->dwReserved0 == IO_REPARSE_TAG_CSV) {
				o->Set(tmp, NEWSTRING(SYB_REPARSEPOINT_CSV));
			} else if (info->dwReserved0 == IO_REPARSE_TAG_DEDUP) {
				o->Set(tmp, NEWSTRING(SYB_REPARSEPOINT_DEDUP));
			} else if (info->dwReserved0 == IO_REPARSE_TAG_DFS) {
				o->Set(tmp, NEWSTRING(SYB_REPARSEPOINT_DFS));
			} else if (info->dwReserved0 == IO_REPARSE_TAG_DFSR) {
				o->Set(tmp, NEWSTRING(SYB_REPARSEPOINT_DFSR));
			} else if (info->dwReserved0 == IO_REPARSE_TAG_HSM) {
				o->Set(tmp, NEWSTRING(SYB_REPARSEPOINT_HSM));
			} else if (info->dwReserved0 == IO_REPARSE_TAG_HSM2) {
				o->Set(tmp, NEWSTRING(SYB_REPARSEPOINT_HSM2));
			} else if (info->dwReserved0 == IO_REPARSE_TAG_MOUNT_POINT) {
				o->Set(tmp, NEWSTRING(SYB_REPARSEPOINT_MOUNTPOINT));
			} else if (info->dwReserved0 == IO_REPARSE_TAG_NFS) {
				o->Set(tmp, NEWSTRING(SYB_REPARSEPOINT_NFS));
			} else if (info->dwReserved0 == IO_REPARSE_TAG_FILE_PLACEHOLDER) {
				o->Set(tmp, NEWSTRING(SYB_REPARSEPOINT_PLACEHOLDER));
			} else if (info->dwReserved0 == IO_REPARSE_TAG_SIS) {
				o->Set(tmp, NEWSTRING(SYB_REPARSEPOINT_SIS));
			} else if (info->dwReserved0 == IO_REPARSE_TAG_SYMLINK) {
				o->Set(tmp, NEWSTRING(SYB_REPARSEPOINT_SYMLINK));
			} else if (info->dwReserved0 == IO_REPARSE_TAG_WIM) {
				o->Set(tmp, NEWSTRING(SYB_REPARSEPOINT_WIM));
			} else {
				o->Set(tmp, NEWSTRING(SYB_REPARSEPOINT_UNKNOWN));
			}
		} else {
			o->Set(tmp, String::Empty(ISOLATE));
		}
		o->Set(NEWSTRING(SYB_FILEATTR_ISARCHIVED), info->dwFileAttributes&FILE_ATTRIBUTE_ARCHIVE ? True(ISOLATE) : False(ISOLATE));
		o->Set(NEWSTRING(SYB_FILEATTR_ISCOMPRESSED), info->dwFileAttributes&FILE_ATTRIBUTE_COMPRESSED ? True(ISOLATE) : False(ISOLATE));
		o->Set(NEWSTRING(SYB_FILEATTR_ISDEVICE), info->dwFileAttributes&FILE_ATTRIBUTE_DEVICE ? True(ISOLATE) : False(ISOLATE));
		o->Set(NEWSTRING(SYB_FILEATTR_ISDIRECTORY), info->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY ? True(ISOLATE) : False(ISOLATE));
		o->Set(NEWSTRING(SYB_FILEATTR_ISENCRYPTED), info->dwFileAttributes&FILE_ATTRIBUTE_ENCRYPTED ? True(ISOLATE) : False(ISOLATE));
		o->Set(NEWSTRING(SYB_FILEATTR_ISHIDDEN), info->dwFileAttributes&FILE_ATTRIBUTE_HIDDEN ? True(ISOLATE) : False(ISOLATE));
		o->Set(NEWSTRING(SYB_FILEATTR_ISNOTCONTENTINDEXED), info->dwFileAttributes&FILE_ATTRIBUTE_NOT_CONTENT_INDEXED ? True(ISOLATE) : False(ISOLATE));
		o->Set(NEWSTRING(SYB_FILEATTR_ISOFFLINE), info->dwFileAttributes&FILE_ATTRIBUTE_OFFLINE ? True(ISOLATE) : False(ISOLATE));
		o->Set(NEWSTRING(SYB_FILEATTR_ISREADONLY), info->dwFileAttributes&FILE_ATTRIBUTE_READONLY ? True(ISOLATE) : False(ISOLATE));
		o->Set(NEWSTRING(SYB_FILEATTR_ISSPARSEFILE), info->dwFileAttributes&FILE_ATTRIBUTE_SPARSE_FILE ? True(ISOLATE) : False(ISOLATE));
		o->Set(NEWSTRING(SYB_FILEATTR_ISSYSTEM), info->dwFileAttributes&FILE_ATTRIBUTE_SYSTEM ? True(ISOLATE) : False(ISOLATE));
		o->Set(NEWSTRING(SYB_FILEATTR_ISTEMPORARY), info->dwFileAttributes&FILE_ATTRIBUTE_TEMPORARY ? True(ISOLATE) : False(ISOLATE));
		o->Set(NEWSTRING(SYB_FILEATTR_ISINTEGERITYSTREAM), info->dwFileAttributes&FILE_ATTRIBUTE_INTEGRITY_STREAM ? True(ISOLATE) : False(ISOLATE));
		o->Set(NEWSTRING(SYB_FILEATTR_ISNOSCRUBDATA), info->dwFileAttributes&FILE_ATTRIBUTE_NO_SCRUB_DATA ? True(ISOLATE) : False(ISOLATE));
		RETURN_SCOPE(o);
	}
	static Handle<Array> basicToJs(resultData *data) {
		ISOLATE_NEW;
		SCOPE_ESCAPABLE;
		RETURNTYPE<Array> a = Array::New(ISOLATE);
		while (data) {
			a->Set(a->Length(), fileInfoToJs(&data->data));
			resultData *old = data;
			data = old->next;
			delete old;
		}
		RETURN_SCOPE(a);
	}
	static Handle<Function> functionRegister(bool isAsyncVersion) {
		ISOLATE_NEW;
		SCOPE_ESCAPABLE;
		RETURNTYPE<String> tmp;
		RETURNTYPE<Function> t = NEWFUNCTION(isAsyncVersion ? jsAsync : jsSync);

		//set error messages
		RETURNTYPE<Object> errors = Object::New(ISOLATE);
		tmp = NEWSTRING(SYB_ERR_WRONG_ARGUMENTS);
		SETWITHATTR(errors, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_ERR_NOT_A_CONSTRUCTOR);
		SETWITHATTR(errors, tmp, tmp, SYB_ATTR_CONST);
		SETWITHATTR(t, NEWSTRING(SYB_ERRORS), errors, SYB_ATTR_CONST);

		//set events
		if (isAsyncVersion) {
			RETURNTYPE<Object> events = Object::New(ISOLATE);
			tmp = NEWSTRING(SYB_EVT_FOUND);
			SETWITHATTR(events, tmp, tmp, SYB_ATTR_CONST);
			tmp = NEWSTRING(SYB_EVT_SUCCEEDED);
			SETWITHATTR(events, tmp, tmp, SYB_ATTR_CONST);
			tmp = NEWSTRING(SYB_EVT_FAILED);
			SETWITHATTR(events, tmp, tmp, SYB_ATTR_CONST);
			tmp = NEWSTRING(SYB_EVT_INTERRUPTED);
			SETWITHATTR(events, tmp, tmp, SYB_ATTR_CONST);
			SETWITHATTR(t, NEWSTRING(SYB_EVENTS), events, SYB_ATTR_CONST);
		}

		//set properties of return value
		RETURNTYPE<Object> returns = Object::New(ISOLATE);
		tmp = NEWSTRING(SYB_RETURNS_LONGNAME);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_RETURNS_SHORTNAME);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_CREATIONTIME);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_LASTACCESSTIME);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_LASTWRITETIME);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_SIZE);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISARCHIVED);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISCOMPRESSED);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISDEVICE);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISDIRECTORY);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISENCRYPTED);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISHIDDEN);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISNOTCONTENTINDEXED);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISOFFLINE);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISREADONLY);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISSPARSEFILE);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISSYSTEM);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISTEMPORARY);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISNOSCRUBDATA);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_RETURNS_REPARSEPOINTTAG);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		SETWITHATTR(t, NEWSTRING(SYB_RETURNS), returns, SYB_ATTR_CONST);

		RETURN_SCOPE(t);
	}
private:
	static bool isValidInfo(const WIN32_FIND_DATAW *info) {//determine whether it is the real content 
		return wcscmp(info->cFileName, L".") != 0 && wcscmp(info->cFileName, L"..") != 0;
	}
	static JSFUNC(jsSync) {
		ISOLATE_NEW_ARGS;
		SCOPE_ESCAPABLE;
		RETURNTYPE<Value> result;
		if (args.IsConstructCall()) {
			result = THROWEXCEPTION(ISOLATE_C SYB_ERR_NOT_A_CONSTRUCTOR);
		} else {
			if (args.Length() > 0 && (args[0]->IsString() || args[0]->IsStringObject())) {
				String::Value spath(args[0]);
				if (args.Length() > 1 && args[1]->IsFunction()) {
					jsCallbackData callbackdata = {args.This(), Handle<Function>::Cast(args[1])};
					result = Integer::New(ISOLATE_C basicWithCallback((wchar_t*)*spath, jsSyncCallback, &callbackdata));
				} else {
					result = basicToJs(basic((wchar_t*)*spath));
				}
			} else {
				result = THROWEXCEPTION(SYB_ERR_WRONG_ARGUMENTS);
			}
		}
		RETURN(result);
	}
	static bool jsSyncCallback(const WIN32_FIND_DATAW *info, void *data) {
		ISOLATE_NEW;
		SCOPE_ESCAPABLE;
		RETURNTYPE<Value> o = fileInfoToJs(info);
		jsCallbackData *d = (jsCallbackData*)data;
		return d->func->Call(d->self, 1, &o)->ToBoolean()->IsTrue();
	}
	static JSFUNC(jsAsync) {
		ISOLATE_NEW_ARGS;
		SCOPE_ESCAPABLE;
		RETURNTYPE<Value> result;
		if (args.IsConstructCall()) {
			result = THROWEXCEPTION(SYB_ERR_NOT_A_CONSTRUCTOR);
		} else {
			if (args.Length() > 1 && (args[0]->IsString() || args[0]->IsStringObject()) && args[1]->IsFunction()) {
				workdata *data = new workdata;
				data->req.data = data;
				PERSISTENT_NEW(data->self, args.This(), Object);
				PERSISTENT_NEW(data->func, RETURNTYPE<Function>::Cast(args[1]), Function);
				String::Value spath(args[0]);
				data->data = _wcsdup((wchar_t*)*spath);
				if (args.Length() > 2 && args[2]->ToBoolean()->IsTrue()) {
					data->hnd = INVALID_HANDLE_VALUE;
					data->count = 0;
					data->stop = false;
				} else {
					data->hnd = NULL;
				}
				if (uv_queue_work(uv_default_loop(), &data->req, beginWork, afterWork) == 0) {
					result = True(ISOLATE);
				} else {
					free(data->data);
					PERSISTENT_RELEASE(data->self);
					PERSISTENT_RELEASE(data->func);
					delete data;
					result = False(ISOLATE);
				}
			} else {
				result = THROWEXCEPTION(SYB_ERR_WRONG_ARGUMENTS);
			}
		}
		RETURN(result);
	}
	static void beginWork(uv_work_t *req) {
		workdata *data = (workdata*)req->data;
		if (data->hnd) {
			WIN32_FIND_DATAW *info = new WIN32_FIND_DATAW;
			if (data->hnd == INVALID_HANDLE_VALUE) {
				data->hnd = FindFirstFileExW((wchar_t*)data->data, FindExInfoStandard, info, FindExSearchNameMatch, NULL, NULL);
				free(data->data);
				if (data->hnd != INVALID_HANDLE_VALUE) {
					while (!isValidInfo(info)) {
						if (!FindNextFileW(data->hnd, info)) {
							FindClose(data->hnd);
							data->hnd = INVALID_HANDLE_VALUE;
							break;
						}
					}
				}
			} else {
				if (!data->stop) {
					if (!FindNextFileW(data->hnd, info)) {
						FindClose(data->hnd);
						data->hnd = INVALID_HANDLE_VALUE;
					} else {
						while (!isValidInfo(info)) {
							if (!FindNextFileW(data->hnd, info)) {
								FindClose(data->hnd);
								data->hnd = INVALID_HANDLE_VALUE;
								break;
							}
						}
					}
				} else {
					FindClose(data->hnd);
					data->hnd = INVALID_HANDLE_VALUE;
				}
			}
			if (data->hnd == INVALID_HANDLE_VALUE) {
				delete info;
			} else {
				data->data = info;
			}
		} else {
			resultData *rdata = basic((wchar_t*)data->data);
			free(data->data);
			data->data = rdata;
		}
	}
	static AFTERWORKCB(afterWork) {
		ISOLATE_NEW;
		SCOPE;
		workdata *data = (workdata*)req->data;
		int del;
		if (data->hnd) {
			RETURNTYPE<Value> result[2];
			if (data->hnd == INVALID_HANDLE_VALUE) {
				result[0] = NEWSTRING(SYB_EVT_SUCCEEDED);
				result[1] = Number::New(ISOLATE_C (double)data->count);
				del = 1;
			} else {
				WIN32_FIND_DATAW *info = (WIN32_FIND_DATAW*)data->data;
				if (data->stop) {
					result[0] = NEWSTRING(data->stop ? SYB_EVT_INTERRUPTED : SYB_EVT_SUCCEEDED);
					result[1] = Number::New(ISOLATE_C (double)data->count);
					del = 1;
				} else {
					data->count++;
					result[0] = NEWSTRING(SYB_EVT_FOUND);
					result[1] = fileInfoToJs(info);
					del = uv_queue_work(uv_default_loop(), &data->req, beginWork, afterWork);
				}
				delete info;
			}
			data->stop = PERSISTENT_CONV(data->func, Function)->Call(PERSISTENT_CONV(data->self, Object), 2, result)->ToBoolean()->IsTrue();
		} else {
			RETURNTYPE<Value> result;
			result = basicToJs((resultData*)data->data);
			del = 1;
			PERSISTENT_CONV(data->func, Function)->Call(PERSISTENT_CONV(data->self, Object), 1, &result);
		}
		if (del) {
			if (del != 1) {
				RETURNTYPE<Value> result[2] = {NEWSTRING(SYB_EVT_FAILED), Number::New(ISOLATE_C (double)data->count)};
				PERSISTENT_CONV(data->func, Function)->Call(PERSISTENT_CONV(data->self, Object), 2, result);
			}
			PERSISTENT_RELEASE(data->func);
			PERSISTENT_RELEASE(data->self);
			delete data;
		}
	}
};