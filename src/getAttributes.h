#pragma once
#include "main.h"

#define SYB_RETURNS_ISREPARSEPOINT (uint8_t*)"IS_REPARSE_POINT"

class getAttributes {
private:
	static const struct workdata {
		uv_work_t req;
		Persistent<Object> self;
		Persistent<Function> func;
		wchar_t *path;
		WIN32_FILE_ATTRIBUTE_DATA *attr;
	};
public:
	static Handle<Object> attrDataToJs(WIN32_FILE_ATTRIBUTE_DATA *data) {
		Isolate *isolate = Isolate::GetCurrent();
		EscapableHandleScope scope(isolate);
		Local<Object> o = Object::New(isolate);
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_CREATIONTIME), Date::New(isolate, fileTimeToJsDateVal(&data->ftCreationTime)));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_LASTACCESSTIME), Date::New(isolate, fileTimeToJsDateVal(&data->ftLastAccessTime)));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_LASTWRITETIME), Date::New(isolate, fileTimeToJsDateVal(&data->ftLastWriteTime)));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_SIZE), Number::New(isolate, (double)combineHiLow(data->nFileSizeHigh, data->nFileSizeLow)));

		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_ISARCHIVED), data->dwFileAttributes&FILE_ATTRIBUTE_ARCHIVE ? True(isolate) : False(isolate));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_ISCOMPRESSED), data->dwFileAttributes&FILE_ATTRIBUTE_COMPRESSED ? True(isolate) : False(isolate));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_ISDEVICE), data->dwFileAttributes&FILE_ATTRIBUTE_DEVICE ? True(isolate) : False(isolate));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_ISDIRECTORY), data->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY ? True(isolate) : False(isolate));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_ISENCRYPTED), data->dwFileAttributes&FILE_ATTRIBUTE_ENCRYPTED ? True(isolate) : False(isolate));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_ISHIDDEN), data->dwFileAttributes&FILE_ATTRIBUTE_HIDDEN ? True(isolate) : False(isolate));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_ISNOTCONTENTINDEXED), data->dwFileAttributes&FILE_ATTRIBUTE_NOT_CONTENT_INDEXED ? True(isolate) : False(isolate));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_ISOFFLINE), data->dwFileAttributes&FILE_ATTRIBUTE_OFFLINE ? True(isolate) : False(isolate));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_ISREADONLY), data->dwFileAttributes&FILE_ATTRIBUTE_READONLY ? True(isolate) : False(isolate));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_ISSPARSEFILE), data->dwFileAttributes&FILE_ATTRIBUTE_SPARSE_FILE ? True(isolate) : False(isolate));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_ISSYSTEM), data->dwFileAttributes&FILE_ATTRIBUTE_SYSTEM ? True(isolate) : False(isolate));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_ISTEMPORARY), data->dwFileAttributes&FILE_ATTRIBUTE_TEMPORARY ? True(isolate) : False(isolate));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_ISINTEGERITYSTREAM), data->dwFileAttributes&FILE_ATTRIBUTE_INTEGRITY_STREAM ? True(isolate) : False(isolate));
		o->Set(String::NewFromOneByte(isolate, SYB_FILEATTR_ISNOSCRUBDATA), data->dwFileAttributes&FILE_ATTRIBUTE_NO_SCRUB_DATA ? True(isolate) : False(isolate));
		o->Set(String::NewFromOneByte(isolate, SYB_RETURNS_ISREPARSEPOINT), data->dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT ? True(isolate) : False(isolate));
		return scope.Escape(o);
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

		//set returns
		Local<Object> returns = Object::New(isolate);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_CREATIONTIME);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_LASTACCESSTIME);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_LASTWRITETIME);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_SIZE);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISARCHIVED);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISCOMPRESSED);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISDEVICE);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISDIRECTORY);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISENCRYPTED);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISHIDDEN);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISNOTCONTENTINDEXED);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISREADONLY);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISSPARSEFILE);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISSYSTEM);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISTEMPORARY);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISINTEGERITYSTREAM);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_FILEATTR_ISNOSCRUBDATA);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_RETURNS_ISREPARSEPOINT);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);

		t->Set(String::NewFromOneByte(isolate, SYB_RETURNS), returns, SYB_ATTR_CONST);

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
				String::Value p(args[0]);
				WIN32_FILE_ATTRIBUTE_DATA data;
				if (GetFileAttributesExW((wchar_t*)*p, GetFileExInfoStandard, &data)) {
					result = attrDataToJs(&data);
				} else {
					result = Undefined(isolate);
				}
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
				String::Value p(args[0]);
				data->path = _wcsdup((wchar_t*)*p);
				data->attr = new WIN32_FILE_ATTRIBUTE_DATA;
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
		if (!GetFileAttributesExW(data->path, GetFileExInfoStandard, data->attr)) {
			delete data->attr;
			data->attr = NULL;
		}
		free(data->path);
	}
	static void afterWork(uv_work_t *req, int status) {
		Isolate *isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
		workdata *data = (workdata*)req->data;
		Local<Function> func = Local<Function>::New(isolate, data->func);
		Local<Object> self = Local<Object>::New(isolate, data->self);
		if (data->attr) {
			Local<Value> result = attrDataToJs(data->attr);
			func->Call(self, 1, &result);
		} else {
			func->Call(self, 0, NULL);
		}
		data->func.Reset();
		data->self.Reset();
		delete data;
	}
};