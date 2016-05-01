#pragma once
#include "main.h"

class setShortName {
private:
	const struct workdata {
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
				String::Value oldname(args[0]);
				wchar_t *newname;
				if (args[1]->IsString() || args[1]->IsStringObject()) {
					String::Value p(args[1]);
					newname = (wchar_t*)*p;
				} else {
					newname = L"";
				}
				result = basic((wchar_t*)*oldname, newname)?True(ISOLATE):False(ISOLATE);
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
			if (args.Length() > 1) {
				workdata *data = new workdata;
				data->req.data = data;
				String::Value p(args[0]->ToString());
				data->path = _wcsdup((wchar_t*)*p);
				String::Value n(args[1]->ToString());
				data->newname = _wcsdup((wchar_t*)*n);
				if (args.Length() > 2 && args[2]->IsFunction()) {
					PERSISTENT_NEW(data->self, args.This(), Object);
					PERSISTENT_NEW(data->func, RETURNTYPE<Function>::Cast(args[2]), Function);
				}
				if (uv_queue_work(uv_default_loop(), &data->req, beginWork, afterWork) == 0) {
					result = True(ISOLATE);
				} else {
					free(data->path);
					if (data->newname) {
						free(data->newname);
					}
					if (!data->func.IsEmpty()) {
						PERSISTENT_RELEASE(data->func);
						PERSISTENT_RELEASE(data->self);
					}
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
		data->result = basic(data->path, data->newname);
		free(data->path);
		if (data->newname) {
			free(data->newname);
		}
	}
	static AFTERWORKCB(afterWork) {
		ISOLATE_NEW;
		SCOPE;
		workdata *data = (workdata*)req->data;
		RETURNTYPE<Value> p = data->result ? True(ISOLATE) : False(ISOLATE);
		if (!data->func.IsEmpty()) {
			PERSISTENT_CONV(data->func, Function)->Call(PERSISTENT_CONV(data->self, Object), 1, &p);
			PERSISTENT_RELEASE(data->func);
			PERSISTENT_RELEASE(data->self);
		}
		delete data;
	}
};