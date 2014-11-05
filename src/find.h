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

#define SYB_EVT_FOUND (uint8_t*)"FOUND"
#define SYB_EVT_SUCCEEDED (uint8_t*)"SUCCEEDED"
#define SYB_EVT_FAILED (uint8_t*)"FAILED"
#define SYB_EVT_INTERRUPTED (uint8_t*)"INTERRUPTED"
#define SYB_RETURNS_LONGNAME (uint8_t*)"LONG_NAME"
#define SYB_RETURNS_SHORTNAME (uint8_t*)"SHORT_NAME"
#define SYB_RETURNS_REPARSEPOINTTAG (uint8_t*)"REPARSE_POINT_TAG"
#define SYB_REPARSEPOINT_UNKNOWN (uint8_t*)"UNKNOWN"
#define SYB_REPARSEPOINT_CSV (uint8_t*)"CSV"
#define SYB_REPARSEPOINT_DEDUP (uint8_t*)"DEDUP"
#define SYB_REPARSEPOINT_DFS (uint8_t*)"DFS"
#define SYB_REPARSEPOINT_DFSR (uint8_t*)"DFSR"
#define SYB_REPARSEPOINT_HSM (uint8_t*)"HSM"
#define SYB_REPARSEPOINT_HSM2 (uint8_t*)"HSM2"
#define SYB_REPARSEPOINT_MOUNTPOINT (uint8_t*)"MOUNT_POINT"
#define SYB_REPARSEPOINT_NFS (uint8_t*)"NFS"
#define SYB_REPARSEPOINT_PLACEHOLDER (uint8_t*)"PLACE_HOLDER"
#define SYB_REPARSEPOINT_SIS (uint8_t*)"SIS"
#define SYB_REPARSEPOINT_SYMLINK (uint8_t*)"SYMLINK"
#define SYB_REPARSEPOINT_WIM (uint8_t*)"WIM"

class find {
public:
	static const struct resultData {//this is a linked table
		WIN32_FIND_DATAW data;
		resultData *next;
	};
	//progressive callback type, if this callback returns true, the search will stop immediately. the contents of info will be rewritten or released after the callback returns, so make a copy before starting a new thread if you still need to use it
	typedef bool(*findResultCall)(const WIN32_FIND_DATAW *info, void *data);
private:
	static const struct jsCallbackData {
		Handle<Object> self;
		Handle<Function> func;
	};
	static const struct workdata {
		uv_work_t req;
		Persistent<Object> self;
		Persistent<Function> func;
		void *data;
		//the following data only used in progressive mode
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
		Isolate *isolate = Isolate::GetCurrent();
		EscapableHandleScope scope(isolate);
		Local<String> tmp;
		Local<Object> o = Object::New(isolate);
		o->Set(String::NewFromOneByte(isolate, SYB_RETURNS_LONGNAME), String::NewFromTwoByte(isolate, (uint16_t*)info->cFileName));
		o->Set(String::NewFromOneByte(isolate, SYB_RETURNS_SHORTNAME), String::NewFromTwoByte(isolate, (uint16_t*)info->cAlternateFileName));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_CREATIONTIME), Date::New(isolate, fileTimeToJsDateVal(&info->ftCreationTime)));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_LASTACCESSTIME), Date::New(isolate, fileTimeToJsDateVal(&info->ftLastAccessTime)));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_LASTWRITETIME), Date::New(isolate, fileTimeToJsDateVal(&info->ftLastWriteTime)));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_SIZE), Number::New(isolate, (double)combineHiLow(info->nFileSizeHigh, info->nFileSizeLow)));
		tmp = String::NewFromOneByte(isolate, SYB_RETURNS_REPARSEPOINTTAG);
		if (info->dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT) {
			if (info->dwReserved0 == IO_REPARSE_TAG_CSV) {
				o->Set(tmp, String::NewFromOneByte(isolate, SYB_REPARSEPOINT_CSV));
			} else if (info->dwReserved0 == IO_REPARSE_TAG_DEDUP) {
				o->Set(tmp, String::NewFromOneByte(isolate, SYB_REPARSEPOINT_DEDUP));
			} else if (info->dwReserved0 == IO_REPARSE_TAG_DFS) {
				o->Set(tmp, String::NewFromOneByte(isolate, SYB_REPARSEPOINT_DFS));
			} else if (info->dwReserved0 == IO_REPARSE_TAG_DFSR) {
				o->Set(tmp, String::NewFromOneByte(isolate, SYB_REPARSEPOINT_DFSR));
			} else if (info->dwReserved0 == IO_REPARSE_TAG_HSM) {
				o->Set(tmp, String::NewFromOneByte(isolate, SYB_REPARSEPOINT_HSM));
			} else if (info->dwReserved0 == IO_REPARSE_TAG_HSM2) {
				o->Set(tmp, String::NewFromOneByte(isolate, SYB_REPARSEPOINT_HSM2));
			} else if (info->dwReserved0 == IO_REPARSE_TAG_MOUNT_POINT) {
				o->Set(tmp, String::NewFromOneByte(isolate, SYB_REPARSEPOINT_MOUNTPOINT));
			} else if (info->dwReserved0 == IO_REPARSE_TAG_NFS) {
				o->Set(tmp, String::NewFromOneByte(isolate, SYB_REPARSEPOINT_NFS));
			} else if (info->dwReserved0 == IO_REPARSE_TAG_FILE_PLACEHOLDER) {
				o->Set(tmp, String::NewFromOneByte(isolate, SYB_REPARSEPOINT_PLACEHOLDER));
			} else if (info->dwReserved0 == IO_REPARSE_TAG_SIS) {
				o->Set(tmp, String::NewFromOneByte(isolate, SYB_REPARSEPOINT_SIS));
			} else if (info->dwReserved0 == IO_REPARSE_TAG_SYMLINK) {
				o->Set(tmp, String::NewFromOneByte(isolate, SYB_REPARSEPOINT_SYMLINK));
			} else if (info->dwReserved0 == IO_REPARSE_TAG_WIM) {
				o->Set(tmp, String::NewFromOneByte(isolate, SYB_REPARSEPOINT_WIM));
			} else {
				o->Set(tmp, String::NewFromOneByte(isolate, SYB_REPARSEPOINT_UNKNOWN));
			}
		} else {
			o->Set(tmp, String::Empty(isolate));
		}
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_ISARCHIVED), info->dwFileAttributes&FILE_ATTRIBUTE_ARCHIVE ? True(isolate) : False(isolate));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_ISCOMPRESSED), info->dwFileAttributes&FILE_ATTRIBUTE_COMPRESSED ? True(isolate) : False(isolate));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_ISDEVICE), info->dwFileAttributes&FILE_ATTRIBUTE_DEVICE ? True(isolate) : False(isolate));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_ISDIRECTORY), info->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY ? True(isolate) : False(isolate));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_ISENCRYPTED), info->dwFileAttributes&FILE_ATTRIBUTE_ENCRYPTED ? True(isolate) : False(isolate));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_ISHIDDEN), info->dwFileAttributes&FILE_ATTRIBUTE_HIDDEN ? True(isolate) : False(isolate));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_ISNOTCONTENTINDEXED), info->dwFileAttributes&FILE_ATTRIBUTE_NOT_CONTENT_INDEXED ? True(isolate) : False(isolate));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_ISOFFLINE), info->dwFileAttributes&FILE_ATTRIBUTE_OFFLINE ? True(isolate) : False(isolate));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_ISREADONLY), info->dwFileAttributes&FILE_ATTRIBUTE_READONLY ? True(isolate) : False(isolate));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_ISSPARSEFILE), info->dwFileAttributes&FILE_ATTRIBUTE_SPARSE_FILE ? True(isolate) : False(isolate));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_ISSYSTEM), info->dwFileAttributes&FILE_ATTRIBUTE_SYSTEM ? True(isolate) : False(isolate));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_ISTEMPORARY), info->dwFileAttributes&FILE_ATTRIBUTE_TEMPORARY ? True(isolate) : False(isolate));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_ISINTEGERITYSTREAM), info->dwFileAttributes&FILE_ATTRIBUTE_INTEGRITY_STREAM ? True(isolate) : False(isolate));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_ISNOSCRUBDATA), info->dwFileAttributes&FILE_ATTRIBUTE_NO_SCRUB_DATA ? True(isolate) : False(isolate));
		return scope.Escape(o);
	}
	static Handle<Array> basicToJs(resultData *data) {
		Isolate *isolate = Isolate::GetCurrent();
		EscapableHandleScope scope(isolate);
		Local<Array> a = Array::New(isolate);
		while (data) {
			a->Set(a->Length(), fileInfoToJs(&data->data));
			resultData *old = data;
			data = old->next;
			delete old;
		}
		return scope.Escape(a);
	}
	static Handle<Function> functionRegister(bool isAsyncVersion) {
		Isolate *isolate = Isolate::GetCurrent();
		EscapableHandleScope scope(isolate);
		Local<String> tmp;
		Local<FunctionTemplate> t = FunctionTemplate::New(isolate, isAsyncVersion ? jsAsync : jsSync);

		//set error messages
		Local<Object> errors = Object::New(isolate);
		tmp = String::NewFromOneByte(isolate, SYB_ERR_WRONG_ARGUMENTS);
		errors->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_ERR_NOT_A_CONSTRUCTOR);
		errors->Set(tmp, tmp, SYB_ATTR_CONST);
		t->Set(String::NewFromOneByte(isolate, SYB_ERRORS), errors, SYB_ATTR_CONST);

		//set events
		if (isAsyncVersion) {
			Local<Object> events = Object::New(isolate);
			tmp = String::NewFromOneByte(isolate, SYB_EVT_FOUND);
			events->Set(tmp, tmp, SYB_ATTR_CONST);
			tmp = String::NewFromOneByte(isolate, SYB_EVT_SUCCEEDED);
			events->Set(tmp, tmp, SYB_ATTR_CONST);
			tmp = String::NewFromOneByte(isolate, SYB_EVT_FAILED);
			events->Set(tmp, tmp, SYB_ATTR_CONST);
			tmp = String::NewFromOneByte(isolate, SYB_EVT_INTERRUPTED);
			events->Set(tmp, tmp, SYB_ATTR_CONST);
			t->Set(String::NewFromOneByte(isolate, SYB_EVENTS), events, SYB_ATTR_CONST);
		}

		//set properties of return value
		Local<Object> returns = Object::New(isolate);
		tmp = String::NewFromOneByte(isolate, SYB_RETURNS_LONGNAME);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_RETURNS_SHORTNAME);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_CREATIONTIME);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_LASTACCESSTIME);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_LASTWRITETIME);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_SIZE);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISARCHIVED);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISCOMPRESSED);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISDEVICE);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISDIRECTORY);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISENCRYPTED);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISHIDDEN);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISNOTCONTENTINDEXED);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISOFFLINE);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISREADONLY);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISSPARSEFILE);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISSYSTEM);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISTEMPORARY);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISNOSCRUBDATA);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_RETURNS_REPARSEPOINTTAG);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		t->Set(String::NewFromOneByte(isolate, SYB_RETURNS), returns, SYB_ATTR_CONST);

		return scope.Escape(t->GetFunction());
	}
private:
	static bool isValidInfo(const WIN32_FIND_DATAW *info) {//determine whether it is the real content 
		return wcscmp(info->cFileName, L".") != 0 && wcscmp(info->cFileName, L"..") != 0;
	}
	static void jsSync(const FunctionCallbackInfo<Value>& args) {
		Isolate *isolate = args.GetIsolate();
		HandleScope scope(isolate);
		Local<Value> result;
		if (args.IsConstructCall()) {
			result = isolate->ThrowException(Exception::Error(String::NewFromOneByte(isolate, SYB_ERR_NOT_A_CONSTRUCTOR)));
		} else {
			if (args.Length() > 0 && (args[0]->IsString() || args[0]->IsStringObject())) {
				String::Value spath(args[0]);
				if (args.Length() > 1 && args[1]->IsFunction()) {
					jsCallbackData callbackdata = {args.This(), Handle<Function>::Cast(args[1])};
					result = Integer::New(isolate, basicWithCallback((wchar_t*)*spath, jsSyncCallback, &callbackdata));
				} else {
					result = basicToJs(basic((wchar_t*)*spath));
				}
			} else {
				result = isolate->ThrowException(Exception::Error(String::NewFromOneByte(isolate, SYB_ERR_WRONG_ARGUMENTS)));
			}
		}
		args.GetReturnValue().Set(result);
	}
	static bool jsSyncCallback(const WIN32_FIND_DATAW *info, void *data) {
		Isolate *isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
		Local<Value> o = fileInfoToJs(info);
		jsCallbackData *d = (jsCallbackData*)data;
		return d->func->Call(d->self, 1, &o)->ToBoolean()->IsTrue();
	}
	static void jsAsync(const FunctionCallbackInfo<Value>& args) {
		Isolate *isolate = args.GetIsolate();
		HandleScope scope(isolate);
		Local<Value> result;
		if (args.IsConstructCall()) {
			result = isolate->ThrowException(Exception::Error(String::NewFromOneByte(isolate, SYB_ERR_NOT_A_CONSTRUCTOR)));
		} else {
			if (args.Length() > 1 && (args[0]->IsString() || args[0]->IsStringObject()) && args[1]->IsFunction()) {
				workdata *data = new workdata;
				data->req.data = data;
				data->self.Reset(isolate, args.This());
				data->func.Reset(isolate, Local<Function>::Cast(args[1]));
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
					result = True(isolate);
				} else {
					free(data->data);
					data->self.Reset();
					data->func.Reset();
					delete data;
					result = False(isolate);
				}
			} else {
				result = isolate->ThrowException(Exception::Error(String::NewFromOneByte(isolate, SYB_ERR_WRONG_ARGUMENTS)));
			}
		}
		args.GetReturnValue().Set(result);
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
	static void afterWork(uv_work_t *req, int status) {
		Isolate *isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
		workdata *data = (workdata*)req->data;
		Local<Function> func = Local<Function>::New(isolate, data->func);
		Local<Object> self = Local<Object>::New(isolate, data->self);
		int del;
		if (data->hnd) {
			Local<Value> result[2];
			if (data->hnd == INVALID_HANDLE_VALUE) {
				result[0] = String::NewFromOneByte(isolate, SYB_EVT_SUCCEEDED);
				result[1] = Number::New(isolate, (double)data->count);
				del = 1;
			} else {
				WIN32_FIND_DATAW *info = (WIN32_FIND_DATAW*)data->data;
				if (data->stop) {
					result[0] = String::NewFromOneByte(isolate, data->stop ? SYB_EVT_INTERRUPTED : SYB_EVT_SUCCEEDED);
					result[1] = Number::New(isolate, (double)data->count);
					del = 1;
				} else {
					data->count++;
					result[0] = String::NewFromOneByte(isolate, SYB_EVT_FOUND);
					result[1] = fileInfoToJs(info);
					del = uv_queue_work(uv_default_loop(), &data->req, beginWork, afterWork);
				}
				delete info;
			}
			data->stop = func->Call(self, 2, result)->ToBoolean()->IsTrue();
		} else {
			Local<Value> result;
			result = basicToJs((resultData*)data->data);
			del = 1;
			func->Call(self, 1, &result);
		}
		if (del) {
			if (del != 1) {
				Local<Value> result[2];
				result[0] = String::NewFromOneByte(isolate, SYB_EVT_FAILED);
				result[1] = Number::New(isolate, (double)data->count);
				func->Call(self, 2, result);
			}
			data->func.Reset();
			data->self.Reset();
			delete data;
		}
	}
};