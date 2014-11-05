#pragma once
#include "main.h"

class setShortName {
private:
	static const struct workdata {
		uv_work_t req;
		Persistent<Object> self;
		Persistent<Function> func;
		wchar_t *path;
		wchar_t *newname;
		bool result;
	};
public:
	static bool basic(const wchar_t *path, const wchar_t *newname) {
		bool result = false;
		if (ensurePrivilege(SE_RESTORE_NAME)) {//make sure the process has SE_RESTORE_NAME privilege
			HANDLE hnd = CreateFileW(path, GENERIC_WRITE | DELETE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
			if (hnd == INVALID_HANDLE_VALUE) {
				result = false;
			} else {
				result = SetFileShortNameW(hnd, newname ? newname : L"") ? true : false;
				CloseHandle(hnd);
			}
		} else {
			result = false;
		}
		return result;
	}
	static Handle<Boolean> js(const Handle<String> path, const Handle<String> newname) {
		Isolate *isolate = Isolate::GetCurrent();
		EscapableHandleScope scope(isolate);
		String::Value p(path);
		String::Value n(newname);
		Local<Boolean> result = basic((wchar_t*)*p, (wchar_t*)*n) ? True(isolate) : False(isolate);
		return scope.Escape(result);
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
				Local<String> newname;
				if (args[1]->IsString() || args[1]->IsStringObject()) {
					newname = Local<String>::Cast(args[1]);
				} else {
					newname = String::Empty(isolate);
				}
				result = js(Local<String>::Cast(args[0]), newname);
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
			if (args.Length() > 1) {
				workdata *data = new workdata;
				data->req.data = data;
				String::Value p(args[0]->ToString());
				data->path = _wcsdup((wchar_t*)*p);
				String::Value n(args[1]->ToString());
				data->newname = _wcsdup((wchar_t*)*n);
				if (args.Length() > 2 && args[2]->IsFunction()) {
					data->self.Reset(isolate, args.This());
					data->func.Reset(isolate, Local<Function>::Cast(args[2]));
				}
				if (uv_queue_work(uv_default_loop(), &data->req, beginWork, afterWork) == 0) {
					result = True(isolate);
				} else {
					free(data->path);
					if (data->newname) {
						free(data->newname);
					}
					if (!data->func.IsEmpty()) {
						data->func.Reset();
						data->self.Reset();
					}
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
		data->result = basic(data->path, data->newname);
		free(data->path);
		if (data->newname) {
			free(data->newname);
		}
	}
	static void afterWork(uv_work_t *req, int status) {
		Isolate *isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
		workdata *data = (workdata*)req->data;
		Local<Value> p = data->result ? True(isolate) : False(isolate);
		if (!data->func.IsEmpty()) {
			Local<Function>::New(isolate, data->func)->Call(Local<Object>::New(isolate, data->self), 1, &p);
			data->func.Reset();
			data->self.Reset();
		}
		delete data;
	}
};