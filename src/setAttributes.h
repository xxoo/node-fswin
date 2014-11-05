#pragma once
#include "main.h"

class setAttributes {
public:
	static const struct attrVal {//0=keep,1=yes,-1=no
		char archive;
		char hidden;
		char notContentIndexed;
		char offline;
		char readonly;
		char system;
		char temporary;
	};
private:
	static const struct workdata {
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
		Isolate *isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
		Local<String> tmp;
		attrVal *a = new attrVal;
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISARCHIVED);
		if (attr->HasOwnProperty(tmp)) {
			a->archive = attr->Get(tmp)->ToBoolean()->IsTrue() ? 1 : -1;
		} else {
			a->archive = 0;
		}
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISHIDDEN);
		if (attr->HasOwnProperty(tmp)) {
			a->hidden = attr->Get(tmp)->ToBoolean()->IsTrue() ? 1 : -1;
		} else {
			a->hidden = 0;
		}
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISNOTCONTENTINDEXED);
		if (attr->HasOwnProperty(tmp)) {
			a->notContentIndexed = attr->Get(tmp)->ToBoolean()->IsTrue() ? 1 : -1;
		} else {
			a->notContentIndexed = 0;
		}
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISOFFLINE);
		if (attr->HasOwnProperty(tmp)) {
			a->offline = attr->Get(tmp)->ToBoolean()->IsTrue() ? 1 : -1;
		} else {
			a->offline = 0;
		}
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISREADONLY);
		if (attr->HasOwnProperty(tmp)) {
			a->readonly = (attr->Get(tmp)->ToBoolean()->IsTrue() ? 1 : -1);
		} else {
			a->readonly = 0;
		}
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISSYSTEM);
		if (attr->HasOwnProperty(tmp)) {
			a->system = attr->Get(tmp)->ToBoolean()->IsTrue() ? 1 : -1;
		} else {
			a->system = 0;
		}
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISTEMPORARY);
		if (attr->HasOwnProperty(tmp)) {
			a->temporary = attr->Get(tmp)->ToBoolean()->IsTrue() ? 1 : -1;
		} else {
			a->temporary = 0;
		}
		return a;
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

		//set params
		Local<Object> params = Object::New(isolate);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISARCHIVED);
		params->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISHIDDEN);
		params->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISNOTCONTENTINDEXED);
		params->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISOFFLINE);
		params->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISREADONLY);
		params->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISSYSTEM);
		params->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISTEMPORARY);
		params->Set(tmp, tmp, SYB_ATTR_CONST);
		t->Set(String::NewFromOneByte(isolate, SYB_PARAMS), params, SYB_ATTR_CONST);

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
			if (args.Length() > 1 && (args[0]->IsString() || args[0]->IsStringObject()) && args[1]->IsObject()) {
				attrVal *a = jsToAttrval(Local<Object>::Cast(args[1]));
				String::Value p(args[0]);
				result = basic((wchar_t*)*p, a) ? True(isolate) : False(isolate);
				delete a;
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
			if (args.Length() > 1 && (args[0]->IsString() || args[0]->IsStringObject()) && args[1]->IsObject()) {
				workdata *data = new workdata;
				data->req.data = data;
				data->self.Reset(isolate, args.This());
				if (args.Length() > 2 && args[2]->IsFunction()) {
					data->func.Reset(isolate, Local<Function>::Cast(args[2]));
				}
				String::Value p(args[0]);
				data->path = _wcsdup((wchar_t*)*p);
				data->attr = jsToAttrval(Local<Object>::Cast(args[1]));
				if (uv_queue_work(uv_default_loop(), &data->req, beginWork, afterWork) == 0) {
					result = True(isolate);
				} else {
					free(data->path);
					data->self.Reset();
					if (!data->func.IsEmpty()) {
						data->func.Reset();
					}
					delete data->attr;
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
		data->result = basic(data->path, data->attr);
		free(data->path);
		delete data->attr;
	}
	static void afterWork(uv_work_t *req, int status) {
		Isolate *isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
		workdata *data = (workdata*)req->data;
		Local<Value> p = data->result ? True(isolate) : False(isolate);
		if (!data->func.IsEmpty()) {
			Local<Function>::New(isolate, data->func)->Call(Local<Object>::New(isolate, data->self), 1, &p);
			data->func.Reset();
		}
		data->self.Reset();
		delete data;
	}
};