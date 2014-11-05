#pragma once
#include "main.h"

class setCompression {
public:
	typedef void(*callbackFunc)(const bool result, void *data);
private:
	static const struct workdata {
		Persistent<Object> self;
		Persistent<Function> func;
	};
	static const struct workdata2 {
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
			uv_async_t *hnd = new uv_async_t;
			uv_async_init(uv_default_loop(), hnd, afterWork);
			hnd->data = work;
			if (CreateIoCompletionPort(work->hnd, hnd->loop->iocp, (ULONG_PTR)hnd, 0)) {
				USHORT c = compress ? COMPRESSION_FORMAT_DEFAULT : COMPRESSION_FORMAT_NONE;
				DeviceIoControl(work->hnd, FSCTL_SET_COMPRESSION, &c, sizeof(USHORT), NULL, 0, NULL, &hnd->async_req.overlapped);
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
		Isolate *isolate = Isolate::GetCurrent();
		EscapableHandleScope scope(isolate);
		Local<String> tmp;
		Local<FunctionTemplate> t = FunctionTemplate::New(isolate, isAsyncVersion ? jsAsync : jsSync);

		//set errmessages
		Local<Object> errors = Object::New(isolate);
		tmp = String::NewFromOneByte(isolate, SYB_ERR_WRONG_ARGUMENTS);
		errors->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_ERR_NOT_A_CONSTRUCTOR);
		errors->Set(tmp, tmp, SYB_ATTR_CONST);
		t->Set(String::NewFromOneByte(isolate, SYB_ERRORS), errors, SYB_ATTR_CONST);

		return scope.Escape(t->GetFunction());
	}
private:
	static void afterWork(uv_async_t *hnd) {
		workdata2 *work = (workdata2*)hnd->data;
		CloseHandle(work->hnd);
		work->callback(hnd->async_req.overlapped.Internal == ERROR_SUCCESS, work->data);
		uv_close((uv_handle_t*)hnd, NULL);
		delete hnd;
		delete work;
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
				result = basic((wchar_t*)*spath, args[1]->ToBoolean()->IsTrue()) ? True(isolate) : False(isolate);
			} else {
				result = isolate->ThrowException(Exception::Error(String::NewFromOneByte(isolate, SYB_ERR_WRONG_ARGUMENTS)));
			}
		}
		args.GetReturnValue().Set(result);
	}
	static void jsAsync(const FunctionCallbackInfo<Value>& args) {
		Isolate *isolate = args.GetIsolate();
		HandleScope scope(isolate);
		Local<Value> result;
		if (args.IsConstructCall()) {
			result = isolate->ThrowException(Exception::Error(String::NewFromOneByte(isolate, SYB_ERR_NOT_A_CONSTRUCTOR)));
		} else {
			if (args.Length() > 0 && (args[0]->IsString() || args[0]->IsStringObject())) {
				workdata *data = NULL;
				bool b;
				if (args.Length() > 1) {
					if (args[1]->IsFunction()) {
						data = new workdata;
						data->self.Reset(isolate, args.This());
						data->func.Reset(isolate, Local<Function>::Cast(args[1]));
						b = args[2]->ToBoolean()->IsTrue();
					} else {
						b = args[1]->ToBoolean()->IsTrue();
					}
				} else {
					b = false;
				}
				String::Value p(args[0]);
				if (basicWithCallback((wchar_t*)*p, b, asyncCallback, data)) {
					result = True(isolate);
				} else {
					if (data) {
						data->self.Reset();
						data->func.Reset();
						delete data;
					}
					result = False(isolate);
				}
			} else {
				result = isolate->ThrowException(Exception::Error(String::NewFromOneByte(isolate, SYB_ERR_WRONG_ARGUMENTS)));
			}
		}
		args.GetReturnValue().Set(result);
	}
	static void asyncCallback(const bool succeeded, void *data) {
		if (data) {
			Isolate *isolate = Isolate::GetCurrent();
			EscapableHandleScope scope(isolate);
			workdata *work = (workdata*)data;
			Local<Value> r = succeeded ? True(isolate) : False(isolate);
			Local<Function>::New(isolate, work->func)->Call(Local<Object>::New(isolate, work->self), 1, &r);
			work->func.Reset();
			work->self.Reset();
			delete work;
		}
	}
};