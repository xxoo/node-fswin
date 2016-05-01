#pragma once
#include "main.h"

class convertPath {
private:
	const struct workdata {
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
		ISOLATE_NEW;
		SCOPE_ESCAPABLE;
		RETURNTYPE<String> r;
		String::Value spath(path);
		wchar_t *tpath = basic((wchar_t*)*spath, islong);
		if (tpath) {
			r = NEWSTRING_TWOBYTES(tpath);
			free(tpath);
		} else {
			r = String::Empty(ISOLATE);
		}
		RETURN_SCOPE(r);
	}
	static Handle<Function> functionRegister(bool isAsyncVersion) {
		ISOLATE_NEW;
		SCOPE_ESCAPABLE;
		RETURNTYPE<Function> t = NEWFUNCTION(isAsyncVersion ? jsAsync : jsSync);
		RETURNTYPE<String> tmp;
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
		ISOLATE_NEW;
		SCOPE;
		RETURNTYPE<Value> result;
		if (args.IsConstructCall()) {
			result = THROWEXCEPTION(SYB_ERR_NOT_A_CONSTRUCTOR);
		} else {
			if (args.Length() > 0 && (args[0]->IsString() || args[0]->IsStringObject())) {
				result = js(RETURNTYPE<String>::Cast(args[0]), args[1]->ToBoolean()->IsTrue());
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
				data->islong = args[2]->ToBoolean()->IsTrue();
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
		wchar_t *p = basic(data->path, data->islong);
		free(data->path);
		data->path = p;
	}
	static AFTERWORKCB(afterWork) {
		ISOLATE_NEW;
		SCOPE;
		workdata *data = (workdata*)req->data;
		RETURNTYPE<Value> p;
		if (data->path) {
			p = NEWSTRING_TWOBYTES(data->path);
			free(data->path);
		} else {
			p = String::Empty(ISOLATE);
		}
		PERSISTENT_CONV(data->func, Function)->Call(PERSISTENT_CONV(data->self, Object), 1, &p);
		PERSISTENT_RELEASE(data->func);
		PERSISTENT_RELEASE(data->self);
		delete data;
	}
};