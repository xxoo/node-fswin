#pragma once
#include "main.h"

class getCompressedSize {
private:
	static const struct workdata {
		uv_work_t req;
		Persistent<Object> self;
		Persistent<Function> func;
		wchar_t *path;
		ULONGLONG result;
	};
public:
	static ULONGLONG basic(const wchar_t *path) {
		ULARGE_INTEGER u;
		u.LowPart = GetCompressedFileSizeW(path, &u.HighPart);
		if (u.LowPart == INVALID_FILE_SIZE&&GetLastError() != NO_ERROR) {
			u.QuadPart = 0;
		}
		return u.QuadPart;
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
	static void jsSync(const FunctionCallbackInfo<Value>& args) {
		Isolate *isolate = args.GetIsolate();
		HandleScope scope(isolate);
		Local<Value> result;
		if (args.IsConstructCall()) {
			result = isolate->ThrowException(Exception::Error(String::NewFromOneByte(isolate, SYB_ERR_NOT_A_CONSTRUCTOR)));
		} else {
			if (args.Length() > 0 && (args[0]->IsString() || args[0]->IsStringObject())) {
				String::Value spath(args[0]);
				result = Number::New(isolate, (double)basic((wchar_t*)*spath));
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
			if (args.Length() > 1 && (args[0]->IsString() || args[0]->IsStringObject()) && args[1]->IsFunction()) {
				workdata *data = new workdata;
				data->req.data = data;
				data->self.Reset(isolate, args.This());
				data->func.Reset(isolate, Local<Function>::Cast(args[1]));
				String::Value p(args[0]);
				data->path = _wcsdup((wchar_t*)*p);
				if (uv_queue_work(uv_default_loop(), &data->req, beginWork, afterWork) == 0) {
					result = True(isolate);
				} else {
					free(data->path);
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
		data->result = basic(data->path);
		free(data->path);
	}
	static void afterWork(uv_work_t *req, int status) {
		Isolate *isolate = Isolate::GetCurrent();
		EscapableHandleScope scope(isolate);
		workdata *data = (workdata*)req->data;
		Local<Value> p = Number::New(isolate, (double)data->result);
		Local<Function>::New(isolate, data->func)->Call(Local<Object>::New(isolate, data->self), 1, &p);
		data->func.Reset();
		data->self.Reset();
		delete data;
	}
};