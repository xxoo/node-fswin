#pragma once
#include "main.h"

#define SYB_RETURNS_ISREPARSEPOINT "IS_REPARSE_POINT"

class getAttributes {
private:
	const struct workdata {
		uv_work_t req;
		Persistent<Object> self;
		Persistent<Function> func;
		wchar_t *path;
		WIN32_FILE_ATTRIBUTE_DATA *attr;
	};
public:
	static Handle<Object> attrDataToJs(WIN32_FILE_ATTRIBUTE_DATA *data) {
		ISOLATE_NEW;
		SCOPE_ESCAPABLE;

		RETURNTYPE<Object> o = Object::New(ISOLATE);
		o->Set(NEWSTRING(SYB_FILEATTR_CREATIONTIME), Date::New(ISOLATE_C fileTimeToJsDateVal(&data->ftCreationTime)));
		o->Set(NEWSTRING(SYB_FILEATTR_LASTACCESSTIME), Date::New(ISOLATE_C fileTimeToJsDateVal(&data->ftLastAccessTime)));
		o->Set(NEWSTRING(SYB_FILEATTR_LASTWRITETIME), Date::New(ISOLATE_C fileTimeToJsDateVal(&data->ftLastWriteTime)));
		o->Set(NEWSTRING(SYB_FILEATTR_SIZE), Number::New(ISOLATE_C (double)combineHiLow(data->nFileSizeHigh, data->nFileSizeLow)));

		o->Set(NEWSTRING(SYB_FILEATTR_ISARCHIVED), data->dwFileAttributes&FILE_ATTRIBUTE_ARCHIVE ? True(ISOLATE) : False(ISOLATE));
		o->Set(NEWSTRING(SYB_FILEATTR_ISCOMPRESSED), data->dwFileAttributes&FILE_ATTRIBUTE_COMPRESSED ? True(ISOLATE) : False(ISOLATE));
		o->Set(NEWSTRING(SYB_FILEATTR_ISDEVICE), data->dwFileAttributes&FILE_ATTRIBUTE_DEVICE ? True(ISOLATE) : False(ISOLATE));
		o->Set(NEWSTRING(SYB_FILEATTR_ISDIRECTORY), data->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY ? True(ISOLATE) : False(ISOLATE));
		o->Set(NEWSTRING(SYB_FILEATTR_ISENCRYPTED), data->dwFileAttributes&FILE_ATTRIBUTE_ENCRYPTED ? True(ISOLATE) : False(ISOLATE));
		o->Set(NEWSTRING(SYB_FILEATTR_ISHIDDEN), data->dwFileAttributes&FILE_ATTRIBUTE_HIDDEN ? True(ISOLATE) : False(ISOLATE));
		o->Set(NEWSTRING(SYB_FILEATTR_ISNOTCONTENTINDEXED), data->dwFileAttributes&FILE_ATTRIBUTE_NOT_CONTENT_INDEXED ? True(ISOLATE) : False(ISOLATE));
		o->Set(NEWSTRING(SYB_FILEATTR_ISOFFLINE), data->dwFileAttributes&FILE_ATTRIBUTE_OFFLINE ? True(ISOLATE) : False(ISOLATE));
		o->Set(NEWSTRING(SYB_FILEATTR_ISREADONLY), data->dwFileAttributes&FILE_ATTRIBUTE_READONLY ? True(ISOLATE) : False(ISOLATE));
		o->Set(NEWSTRING(SYB_FILEATTR_ISSPARSEFILE), data->dwFileAttributes&FILE_ATTRIBUTE_SPARSE_FILE ? True(ISOLATE) : False(ISOLATE));
		o->Set(NEWSTRING(SYB_FILEATTR_ISSYSTEM), data->dwFileAttributes&FILE_ATTRIBUTE_SYSTEM ? True(ISOLATE) : False(ISOLATE));
		o->Set(NEWSTRING(SYB_FILEATTR_ISTEMPORARY), data->dwFileAttributes&FILE_ATTRIBUTE_TEMPORARY ? True(ISOLATE) : False(ISOLATE));
		o->Set(NEWSTRING(SYB_FILEATTR_ISINTEGERITYSTREAM), data->dwFileAttributes&FILE_ATTRIBUTE_INTEGRITY_STREAM ? True(ISOLATE) : False(ISOLATE));
		o->Set(NEWSTRING(SYB_FILEATTR_ISNOSCRUBDATA), data->dwFileAttributes&FILE_ATTRIBUTE_NO_SCRUB_DATA ? True(ISOLATE) : False(ISOLATE));
		o->Set(NEWSTRING(SYB_RETURNS_ISREPARSEPOINT), data->dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT ? True(ISOLATE) : False(ISOLATE));

		RETURN_SCOPE(o);
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

		//set returns
		RETURNTYPE<Object> returns = Object::New(ISOLATE);
		tmp = NEWSTRING(SYB_FILEATTR_CREATIONTIME);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_LASTACCESSTIME);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_LASTWRITETIME);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_SIZE);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISARCHIVED);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISCOMPRESSED);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISDEVICE);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISDIRECTORY);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISENCRYPTED);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISHIDDEN);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISNOTCONTENTINDEXED);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISREADONLY);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISSPARSEFILE);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISSYSTEM);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISTEMPORARY);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISINTEGERITYSTREAM);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_FILEATTR_ISNOSCRUBDATA);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_RETURNS_ISREPARSEPOINT);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		SETWITHATTR(t, NEWSTRING(SYB_RETURNS), returns, SYB_ATTR_CONST);

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
				String::Value p(args[0]);
				WIN32_FILE_ATTRIBUTE_DATA data;
				if (GetFileAttributesExW((wchar_t*)*p, GetFileExInfoStandard, &data)) {
					result = attrDataToJs(&data);
				} else {
					result = Undefined(ISOLATE);
				}
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
				data->attr = new WIN32_FILE_ATTRIBUTE_DATA;
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
		if (!GetFileAttributesExW(data->path, GetFileExInfoStandard, data->attr)) {
			delete data->attr;
			data->attr = NULL;
		}
		free(data->path);
	}
	static AFTERWORKCB(afterWork) {
		ISOLATE_NEW;
		SCOPE;
		workdata *data = (workdata*)req->data;
		if (data->attr) {
			RETURNTYPE<Value> result = attrDataToJs(data->attr);
			PERSISTENT_CONV(data->func, Function)->Call(PERSISTENT_CONV(data->self, Object), 1, &result);
		} else {
			PERSISTENT_CONV(data->func, Function)->Call(PERSISTENT_CONV(data->self, Object), 0, NULL);
		}
		PERSISTENT_RELEASE(data->func);
		PERSISTENT_RELEASE(data->self);
		delete data;
	}
};