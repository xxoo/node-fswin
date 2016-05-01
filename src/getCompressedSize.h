#pragma once
#include "main.h"

class getCompressedSize {
private:
	const struct workdata {
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
	static JSFUNC(jsSync) {
		ISOLATE_NEW_ARGS;
		SCOPE;
		RETURNTYPE<Value> result;
		if (args.IsConstructCall()) {
			result = THROWEXCEPTION(SYB_ERR_NOT_A_CONSTRUCTOR);
		} else {
			if (args.Length() > 0 && (args[0]->IsString() || args[0]->IsStringObject())) {
				String::Value spath(args[0]);
				result = Number::New(ISOLATE_C (double)basic((wchar_t*)*spath));
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
			if (args.Length() > 1 && (args[0]->IsString() || args[0]->IsStringObject()) && args[1]->IsFunction()) {
				workdata *data = new workdata;
				data->req.data = data;
				PERSISTENT_NEW(data->self, args.This(), Object);
				PERSISTENT_NEW(data->func, RETURNTYPE<Function>::Cast(args[1]), Function);
				String::Value p(args[0]);
				data->path = _wcsdup((wchar_t*)*p);
				if (uv_queue_work(uv_default_loop(), &data->req, beginWork, afterWork) == 0) {
					result = True(ISOLATE);
				} else {
					free(data->path);
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
		data->result = basic(data->path);
		free(data->path);
	}
	static AFTERWORKCB(afterWork) {
		ISOLATE_NEW;
		SCOPE_ESCAPABLE;
		workdata *data = (workdata*)req->data;
		RETURNTYPE<Value> p = Number::New(ISOLATE_C(double)data->result);
		PERSISTENT_CONV(data->func, Function)->Call(PERSISTENT_CONV(data->self, Object), 1, &p);
		PERSISTENT_RELEASE(data->self);
		PERSISTENT_RELEASE(data->func);
		delete data;
	}
};