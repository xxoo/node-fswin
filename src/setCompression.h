#pragma once
#include "main.h"

class setCompression {
public:
	typedef void(*callbackFunc)(const bool result, void *data);
private:
	const struct workdata {
		Persistent<Object> self;
		Persistent<Function> func;
	};
	const struct workdata2 {
		callbackFunc callback;
		void *data;
		HANDLE hnd;
	};
public:
	static bool basic(const wchar_t *path, const bool compress) {
		bool result = false;
		HANDLE hnd = CreateFileW(path, FILE_GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
		if (hnd != INVALID_HANDLE_VALUE) {
			USHORT c = compress ? COMPRESSION_FORMAT_DEFAULT : COMPRESSION_FORMAT_NONE;
			DWORD d;
			if (DeviceIoControl(hnd, FSCTL_SET_COMPRESSION, &c, sizeof(USHORT), NULL, 0, &d, NULL)) {
				result = true;
			}
			CloseHandle(hnd);
		}
		return result;
	}
	static bool basicWithCallback(const wchar_t *path, const bool compress, callbackFunc callback, void *data) {
		bool result = false;
		workdata2 *work = new workdata2;
		work->callback = callback;
		work->data = data;
		work->hnd = CreateFileW(path, FILE_GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
		if (work->hnd == INVALID_HANDLE_VALUE) {
			delete work;
		} else {
			uv_loop_t *loop = uv_default_loop();
			if (CreateIoCompletionPort(work->hnd, loop->iocp, (ULONG_PTR)work->hnd, 0)) {
				uv_async_t *hnd = new uv_async_t;
				uv_async_init(uv_default_loop(), hnd, afterWork);
				hnd->data = work;
				USHORT c = compress ? COMPRESSION_FORMAT_DEFAULT : COMPRESSION_FORMAT_NONE;
				DeviceIoControl(work->hnd, FSCTL_SET_COMPRESSION, &c, sizeof(USHORT), NULL, 0, NULL, &hnd->async_req.THEASYNCOVERLAP);
				if (GetLastError() == ERROR_IO_PENDING) {
					result = true;
				} else {
					CloseHandle(work->hnd);
					uv_close((uv_handle_t*)hnd, NULL);
					delete hnd;
					delete work;
				}
			}
		}
		return result;
	}
	static Handle<Function> functionRegister(bool isAsyncVersion) {
		ISOLATE_NEW;
		SCOPE_ESCAPABLE;
		RETURNTYPE<String> tmp;
		RETURNTYPE<Function> t = NEWFUNCTION(isAsyncVersion ? jsAsync : jsSync);

		//set errmessages
		RETURNTYPE<Object> errors = Object::New(ISOLATE);
		tmp = NEWSTRING(SYB_ERR_WRONG_ARGUMENTS);
		SETWITHATTR(errors, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_ERR_NOT_A_CONSTRUCTOR);
		SETWITHATTR(errors, tmp, tmp, SYB_ATTR_CONST);
		SETWITHATTR(t, NEWSTRING(SYB_ERRORS), errors, SYB_ATTR_CONST);

		RETURN_SCOPE(t);
	}
private:
	static ASYNCCB(afterWork) {
		workdata2 *work = (workdata2*)hnd->data;
		CloseHandle(work->hnd);
		work->callback(hnd->async_req.THEASYNCOVERLAP.Internal == ERROR_SUCCESS, work->data);
		uv_close((uv_handle_t*)hnd, NULL);
		delete hnd;
		delete work;
	}
	static JSFUNC(jsSync) {
		ISOLATE_NEW_ARGS;
		SCOPE;
		RETURNTYPE<Value> result;
		if (args.IsConstructCall()) {
			result = THROWEXCEPTION(SYB_ERR_NOT_A_CONSTRUCTOR);
		} else {
			if (args.Length() > 0 && (args[0]->IsString() || args[0]->IsStringObject())) {
				String::Value spath(args[0]);
				result = basic((wchar_t*)*spath, args[1]->ToBoolean()->IsTrue()) ? True(ISOLATE) : False(ISOLATE);
			} else {
				result = THROWEXCEPTION(SYB_ERR_WRONG_ARGUMENTS);
			}
		}
		RETURN(result);
	}
	static JSFUNC(jsAsync) {
		ISOLATE_NEW_ARGS;
		SCOPE;
		RETURNTYPE<Value> result;
		if (args.IsConstructCall()) {
			result = THROWEXCEPTION(SYB_ERR_NOT_A_CONSTRUCTOR);
		} else {
			if (args.Length() > 0 && (args[0]->IsString() || args[0]->IsStringObject())) {
				workdata *data = NULL;
				bool b;
				if (args.Length() > 1) {
					if (args[1]->IsFunction()) {
						data = new workdata;
						PERSISTENT_NEW(data->self, args.This(), Object);
						PERSISTENT_NEW(data->func, RETURNTYPE<Function>::Cast(args[1]), Function);
						b = args[2]->ToBoolean()->IsTrue();
					} else {
						b = args[1]->ToBoolean()->IsTrue();
					}
				} else {
					b = false;
				}
				String::Value p(args[0]);
				if (basicWithCallback((wchar_t*)*p, b, asyncCallback, data)) {
					result = True(ISOLATE);
				} else {
					if (data) {
						PERSISTENT_RELEASE(data->self);
						PERSISTENT_RELEASE(data->func);
						delete data;
					}
					result = False(ISOLATE);
				}
			} else {
				result = THROWEXCEPTION(SYB_ERR_WRONG_ARGUMENTS);
			}
		}
		RETURN(result);
	}
	static void asyncCallback(const bool succeeded, void *data) {
		if (data) {
			ISOLATE_NEW;
			SCOPE;
			workdata *work = (workdata*)data;
			RETURNTYPE<Value> r = succeeded ? True(ISOLATE) : False(ISOLATE);
			PERSISTENT_CONV(work->func, Function)->Call(PERSISTENT_CONV(work->self, Object), 1, &r);
			PERSISTENT_RELEASE(work->self);
			PERSISTENT_RELEASE(work->func);
			delete work;
		}
	}
};