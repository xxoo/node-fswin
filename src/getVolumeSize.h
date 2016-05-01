#pragma once
#include "main.h"

#define SYB_RETURNS_TOTALSPACE "TOTAL"
#define SYB_RETURNS_FREESPACE "FREE"
class getVolumeSpace {
public:
	const struct spaces {
		ULONGLONG totalSpace;
		ULONGLONG freeSpace;
	};
private:
	const struct workdata {
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
		ISOLATE_NEW;
		SCOPE_ESCAPABLE;
		RETURNTYPE<Object> result;
		if (spc) {
			result = Object::New(ISOLATE);
			result->Set(NEWSTRING(SYB_RETURNS_TOTALSPACE), Number::New(ISOLATE_C (double)spc->totalSpace));
			result->Set(NEWSTRING(SYB_RETURNS_FREESPACE), Number::New(ISOLATE_C (double)spc->freeSpace));
			delete spc;
		}
		RETURN_SCOPE(result);
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

		//set return values
		RETURNTYPE<Object> returns = Object::New(ISOLATE);
		tmp = NEWSTRING(SYB_RETURNS_TOTALSPACE);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_RETURNS_FREESPACE);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		SETWITHATTR(t, NEWSTRING(SYB_RETURNS), errors, SYB_ATTR_CONST);

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
				result = spacesToJs(basic((wchar_t*)*spath));
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
		spaces *p = basic((wchar_t*)data->path);
		free(data->path);
		data->path = p;
	}
	static AFTERWORKCB(afterWork) {
		ISOLATE_NEW;
		SCOPE;
		workdata *data = (workdata*)req->data;
		RETURNTYPE<Value> p = spacesToJs((spaces*)data->path);
		PERSISTENT_CONV(data->func, Function)->Call(PERSISTENT_CONV(data->self, Object), 1, &p);
		PERSISTENT_RELEASE(data->self);
		PERSISTENT_RELEASE(data->func);
		delete data;
	}
};