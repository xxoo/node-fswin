#pragma once
#include "main.h"

class setAttributes {
public:
	const struct attrVal {//0=keep,1=yes,-1=no
		char archive;
		char hidden;
		char notContentIndexed;
		char offline;
		char readonly;
		char system;
		char temporary;
	};
private:
	const struct workdata {
		uv_work_t req;
		Persistent<Object> self;
		Persistent<Function> func;
		wchar_t *path;
		attrVal *attr;
		bool result;
	};
public:
	static bool basic(const wchar_t *file, const attrVal *attr) {
		bool result;
		DWORD oldattr = GetFileAttributesW(file);
		if (oldattr == INVALID_FILE_ATTRIBUTES) {
			result = false;
		} else {
			DWORD newattr = oldattr;
			if (attr->archive < 0 && newattr&FILE_ATTRIBUTE_ARCHIVE) {
				newattr ^= FILE_ATTRIBUTE_ARCHIVE;
			} else if (attr->archive > 0 && !(newattr&FILE_ATTRIBUTE_ARCHIVE)) {
				newattr |= FILE_ATTRIBUTE_ARCHIVE;
			}
			if (attr->hidden < 0 && newattr&FILE_ATTRIBUTE_HIDDEN) {
				newattr ^= FILE_ATTRIBUTE_HIDDEN;
			} else if (attr->hidden > 0 && !(newattr&FILE_ATTRIBUTE_HIDDEN)) {
				newattr |= FILE_ATTRIBUTE_HIDDEN;
			}
			if (attr->notContentIndexed < 0 && newattr&FILE_ATTRIBUTE_NOT_CONTENT_INDEXED) {
				newattr ^= FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
			} else if (attr->notContentIndexed > 0 && !(newattr&FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)) {
				newattr |= FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
			}
			if (attr->offline < 0 && newattr&FILE_ATTRIBUTE_OFFLINE) {
				newattr ^= FILE_ATTRIBUTE_OFFLINE;
			} else if (attr->offline > 0 && !(newattr&FILE_ATTRIBUTE_OFFLINE)) {
				newattr |= FILE_ATTRIBUTE_OFFLINE;
			}
			if (attr->readonly < 0 && newattr&FILE_ATTRIBUTE_READONLY) {
				newattr ^= FILE_ATTRIBUTE_READONLY;
			} else if (attr->readonly > 0 && !(newattr&FILE_ATTRIBUTE_READONLY)) {
				newattr |= FILE_ATTRIBUTE_READONLY;
			}
			if (attr->system < 0 && newattr&FILE_ATTRIBUTE_SYSTEM) {
				newattr ^= FILE_ATTRIBUTE_SYSTEM;
			} else if (attr->system > 0 && !(newattr&FILE_ATTRIBUTE_SYSTEM)) {
				newattr |= FILE_ATTRIBUTE_SYSTEM;
			}
			if (attr->temporary < 0 && newattr&FILE_ATTRIBUTE_TEMPORARY) {
				newattr ^= FILE_ATTRIBUTE_TEMPORARY;
			} else if (attr->temporary > 0 && !(newattr&FILE_ATTRIBUTE_TEMPORARY)) {
				newattr |= FILE_ATTRIBUTE_TEMPORARY;
			}
			if (newattr == oldattr) {
				result = true;
			} else {
				result = SetFileAttributesW(file, newattr) ? true : false;
			}
		}
		return result;
	}
	static attrVal *jsToAttrval(Handle<Object> attr) {//delete the result if it is not NULL
		ISOLATE_NEW;
		SCOPE;
		RETURNTYPE<String> tmp;
		attrVal *a = new attrVal;
		tmp = NEWSTRING(SYB_FILEATTR_ISARCHIVED);
		if (attr->HasOwnProperty(tmp)) {
			a->archive = attr->Get(tmp)->ToBoolean()->IsTrue() ? 1 : -1;
		} else {
			a->archive = 0;
		}
		tmp = NEWSTRING(SYB_FILEATTR_ISHIDDEN);
		if (attr->HasOwnProperty(tmp)) {
			a->hidden = attr->Get(tmp)->ToBoolean()->IsTrue() ? 1 : -1;
		} else {
			a->hidden = 0;
		}
		tmp = NEWSTRING(SYB_FILEATTR_ISNOTCONTENTINDEXED);
		if (attr->HasOwnProperty(tmp)) {
			a->notContentIndexed = attr->Get(tmp)->ToBoolean()->IsTrue() ? 1 : -1;
		} else {
			a->notContentIndexed = 0;
		}
		tmp = NEWSTRING(SYB_FILEATTR_ISOFFLINE);
		if (attr->HasOwnProperty(tmp)) {
			a->offline = attr->Get(tmp)->ToBoolean()->IsTrue() ? 1 : -1;
		} else {
			a->offline = 0;
		}
		tmp = NEWSTRING(SYB_FILEATTR_ISREADONLY);
		if (attr->HasOwnProperty(tmp)) {
			a->readonly = (attr->Get(tmp)->ToBoolean()->IsTrue() ? 1 : -1);
		} else {
			a->readonly = 0;
		}
		tmp = NEWSTRING(SYB_FILEATTR_ISSYSTEM);
		if (attr->HasOwnProperty(tmp)) {
			a->system = attr->Get(tmp)->ToBoolean()->IsTrue() ? 1 : -1;
		} else {
			a->system = 0;
		}
		tmp = NEWSTRING(SYB_FILEATTR_ISTEMPORARY);
		if (attr->HasOwnProperty(tmp)) {
			a->temporary = attr->Get(tmp)->ToBoolean()->IsTrue() ? 1 : -1;
		} else {
			a->temporary = 0;
		}
		return a;
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

		//set params
		RETURNTYPE<Object> params = Object::New(ISOLATE);
		tmp = NEWSTRING(SYB_FILEATTR_ISARCHIVED);
		SETWITHATTR(params, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISHIDDEN);
		SETWITHATTR(params, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISNOTCONTENTINDEXED);
		SETWITHATTR(params, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISOFFLINE);
		SETWITHATTR(params, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISREADONLY);
		SETWITHATTR(params, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISSYSTEM);
		SETWITHATTR(params, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISTEMPORARY);
		SETWITHATTR(params, tmp, tmp, SYB_ATTR_CONST);
		SETWITHATTR(t, NEWSTRING(SYB_PARAMS), params, SYB_ATTR_CONST);

		RETURN_SCOPE(t);
	}
private:
	static JSFUNC(jsSync) {
		ISOLATE_NEW_ARGS;
		SCOPE;
		RETURNTYPE<Value> result;
		if (args.IsConstructCall()) {
			THROWEXCEPTION(SYB_ERR_NOT_A_CONSTRUCTOR);
		} else {
			if (args.Length() > 1 && (args[0]->IsString() || args[0]->IsStringObject()) && args[1]->IsObject()) {
				attrVal *a = jsToAttrval(RETURNTYPE<Object>::Cast(args[1]));
				String::Value p(args[0]);
				result = basic((wchar_t*)*p, a) ? True(ISOLATE) : False(ISOLATE);
				delete a;
			} else {
				THROWEXCEPTION(SYB_ERR_WRONG_ARGUMENTS);
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
			if (args.Length() > 1 && (args[0]->IsString() || args[0]->IsStringObject()) && args[1]->IsObject()) {
				workdata *data = new workdata;
				data->req.data = data;
				PERSISTENT_NEW(data->self, args.This(), Object);
				if (args.Length() > 2 && args[2]->IsFunction()) {
					PERSISTENT_NEW(data->func, RETURNTYPE<Function>::Cast(args[2]), Function);
				}
				String::Value p(args[0]);
				data->path = _wcsdup((wchar_t*)*p);
				data->attr = jsToAttrval(RETURNTYPE<Object>::Cast(args[1]));
				if (uv_queue_work(uv_default_loop(), &data->req, beginWork, afterWork) == 0) {
					result = True(ISOLATE);
				} else {
					free(data->path);
					PERSISTENT_RELEASE(data->self);
					if (!data->func.IsEmpty()) {
						PERSISTENT_RELEASE(data->func);
					}
					delete data->attr;
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
		data->result = basic(data->path, data->attr);
		free(data->path);
		delete data->attr;
	}
	static AFTERWORKCB(afterWork) {
		ISOLATE_NEW;
		SCOPE;
		workdata *data = (workdata*)req->data;
		RETURNTYPE<Value> p = data->result ? True(ISOLATE) : False(ISOLATE);
		if (!data->func.IsEmpty()) {
			PERSISTENT_CONV(data->func, Function)->Call(PERSISTENT_CONV(data->self, Object), 1, &p);
			PERSISTENT_RELEASE(data->func);
		}
		PERSISTENT_RELEASE(data->self);
		delete data;
	}
};