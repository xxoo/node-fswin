#pragma once
#include "main.h"

class convertPath {
private:
	static const struct workdata {
		uv_work_t req;
		Persistent<Object> self;
		Persistent<Function> func;
		wchar_t *path;
		bool islong;
	};
public:
	static wchar_t *basic(const wchar_t *path, bool islong) {//you need to free the result yourself if it is not NULL
		wchar_t *tpath;
		DWORD sz = islong ? GetLongPathNameW(path, NULL, 0) : GetShortPathNameW(path, NULL, 0);
		if (sz > 0) {
			tpath = (wchar_t*)malloc(sz*sizeof(wchar_t));
			islong ? GetLongPathNameW(path, tpath, sz) : GetShortPathNameW(path, tpath, sz);
		} else {
			tpath = NULL;
		}
		return tpath;
	}
	static Handle<String> js(Handle<String> path, bool islong) {
		Isolate *isolate = Isolate::GetCurrent();
		EscapableHandleScope scope(isolate);
		Local<String> r;
		String::Value spath(path);
		wchar_t *tpath = basic((wchar_t*)*spath, islong);
		if (tpath) {
			r = String::NewFromTwoByte(isolate, (uint16_t*)tpath);
			free(tpath);
		} else {
			r = String::Empty(isolate);
		}
		return scope.Escape(r);
	}
	static Handle<Function> functionRegister(bool isAsyncVersion) {
		Isolate *isolate = Isolate::GetCurrent();
		EscapableHandleScope scope(isolate);
		Local<FunctionTemplate> t = FunctionTemplate::New(isolate, isAsyncVersion ? jsAsync : jsSync);
		Local<String> tmp;
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
				result = js(Local<String>::Cast(args[0]), args[1]->ToBoolean()->IsTrue());
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
				data->islong = args[2]->ToBoolean()->IsTrue();
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
		wchar_t *p = basic(data->path, data->islong);
		free(data->path);
		data->path = p;
	}
	static void afterWork(uv_work_t *req, int status) {
		Isolate *isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
		workdata *data = (workdata*)req->data;
		Local<Value> p;
		if (data->path) {
			p = String::NewFromTwoByte(isolate, (uint16_t*)data->path);
			free(data->path);
		} else {
			p = String::Empty(isolate);
		}
		Local<Function>::New(isolate, data->func)->Call(Local<Object>::New(isolate, data->self), 1, &p);
		data->func.Reset();
		data->self.Reset();
		delete data;
	}
};