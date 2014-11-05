#pragma once
#include "main.h"

#define SYB_RETURNS_TOTALSPACE (uint8_t*)"TOTAL"
#define SYB_RETURNS_FREESPACE (uint8_t*)"FREE"
class getVolumeSpace {
public:
	static const struct spaces {
		ULONGLONG totalSpace;
		ULONGLONG freeSpace;
	};
private:
	static const struct workdata {
		uv_work_t req;
		Persistent<Object> self;
		Persistent<Function> func;
		void *path;
	};
public:
	static spaces *basic(const wchar_t *path) {//you need to delete the result yourself if it is not NULL
		ULARGE_INTEGER u1;
		ULARGE_INTEGER u2;
		spaces *result;
		if (GetDiskFreeSpaceExW(path, &u1, &u2, NULL)) {
			result = new spaces;
			result->freeSpace = u1.QuadPart;
			result->totalSpace = u2.QuadPart;
		} else {
			result = NULL;
		}
		return result;
	}
	static Handle<Object> spacesToJs(const spaces *spc) {//this function will delete the param if it is not NULL
		Isolate *isolate = Isolate::GetCurrent();
		EscapableHandleScope scope(isolate);
		Local<Object> result;
		if (spc) {
			result = Object::New(isolate);
			result->Set(String::NewFromOneByte(isolate, SYB_RETURNS_TOTALSPACE), Number::New(isolate, (double)spc->totalSpace));
			result->Set(String::NewFromOneByte(isolate, SYB_RETURNS_FREESPACE), Number::New(isolate, (double)spc->freeSpace));
			delete spc;
		}
		return scope.Escape(result);
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

		//set return values
		Local<Object> returns = Object::New(isolate);
		tmp = String::NewFromOneByte(isolate, SYB_RETURNS_TOTALSPACE);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_RETURNS_FREESPACE);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		t->Set(String::NewFromOneByte(isolate, SYB_RETURNS), errors, SYB_ATTR_CONST);

		return scope.Escape(t->GetFunction());
	}
private:
	static void jsSync(const FunctionCallbackInfo<Value>& args) {
		Isolate *isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
		Local<Value> result;
		if (args.IsConstructCall()) {
			result = isolate->ThrowException(Exception::Error(String::NewFromOneByte(isolate, SYB_ERR_NOT_A_CONSTRUCTOR)));
		} else {
			if (args.Length() > 0 && (args[0]->IsString() || args[0]->IsStringObject())) {
				String::Value spath(args[0]);
				result = spacesToJs(basic((wchar_t*)*spath));
			} else {
				result = isolate->ThrowException(Exception::Error(String::NewFromOneByte(isolate, SYB_ERR_WRONG_ARGUMENTS)));
			}
		}
		args.GetReturnValue().Set(result);
	}
	static void jsAsync(const FunctionCallbackInfo<Value>& args) {
		Isolate *isolate = Isolate::GetCurrent();
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
		spaces *p = basic((wchar_t*)data->path);
		free(data->path);
		data->path = p;
	}
	static void afterWork(uv_work_t *req, int status) {
		Isolate *isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
		workdata *data = (workdata*)req->data;
		Local<Value> p = spacesToJs((spaces*)data->path);
		Local<Function>::New(isolate, data->func)->Call(Local<Object>::New(isolate, data->self), 1, &p);
		data->func.Reset();
		data->self.Reset();
		delete data;
	}
};