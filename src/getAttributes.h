#pragma once
#include "main.h"

class getAttributes {
private:
	static const Persistent<String> syb_err_wrong_arguments;
	static const Persistent<String> syb_err_not_a_constructor;
	static const Persistent<String> syb_returns_creationTime;
	static const Persistent<String> syb_returns_lastAccessTime;
	static const Persistent<String> syb_returns_lastWriteTime;
	static const Persistent<String> syb_returns_size;
	static const Persistent<String> syb_returns_isArchived;
	static const Persistent<String> syb_returns_isCompressed;
	static const Persistent<String> syb_returns_isDevice;
	static const Persistent<String> syb_returns_isDirectory;
	static const Persistent<String> syb_returns_isEncrypted;
	static const Persistent<String> syb_returns_isHidden;
	static const Persistent<String> syb_returns_isNotContentIndexed;
	static const Persistent<String> syb_returns_isOffline;
	static const Persistent<String> syb_returns_isReadOnly;
	static const Persistent<String> syb_returns_isSparseFile;
	static const Persistent<String> syb_returns_isSystem;
	static const Persistent<String> syb_returns_isTemporary;
	static const Persistent<String> syb_returns_isIntegerityStream;
	static const Persistent<String> syb_returns_isNoScrubData;
	static const Persistent<String> syb_returns_isReparsePoint;
	static const struct workdata {
		uv_work_t req;
		Persistent<Object> self;
		Persistent<Function> func;
		wchar_t *path;
		WIN32_FILE_ATTRIBUTE_DATA *attr;
	};
public:
	static Handle<Object> attrDataToJs(WIN32_FILE_ATTRIBUTE_DATA *data) {
		HandleScope scope;
		Handle<Object> o = Object::New();
		o->Set(syb_returns_creationTime, Date::New(fileTimeToJsDateVal(&data->ftCreationTime)));
		o->Set(syb_returns_lastAccessTime, Date::New(fileTimeToJsDateVal(&data->ftLastAccessTime)));
		o->Set(syb_returns_lastWriteTime, Date::New(fileTimeToJsDateVal(&data->ftLastWriteTime)));
		o->Set(syb_returns_size, Number::New((double)combineHiLow(data->nFileSizeHigh, data->nFileSizeLow)));

		o->Set(syb_returns_isArchived, data->dwFileAttributes&FILE_ATTRIBUTE_ARCHIVE ? True() : False());
		o->Set(syb_returns_isCompressed, data->dwFileAttributes&FILE_ATTRIBUTE_COMPRESSED ? True() : False());
		o->Set(syb_returns_isDevice, data->dwFileAttributes&FILE_ATTRIBUTE_DEVICE ? True() : False());
		o->Set(syb_returns_isDirectory, data->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY ? True() : False());
		o->Set(syb_returns_isEncrypted, data->dwFileAttributes&FILE_ATTRIBUTE_ENCRYPTED ? True() : False());
		o->Set(syb_returns_isHidden, data->dwFileAttributes&FILE_ATTRIBUTE_HIDDEN ? True() : False());
		o->Set(syb_returns_isNotContentIndexed, data->dwFileAttributes&FILE_ATTRIBUTE_NOT_CONTENT_INDEXED ? True() : False());
		o->Set(syb_returns_isOffline, data->dwFileAttributes&FILE_ATTRIBUTE_OFFLINE ? True() : False());
		o->Set(syb_returns_isReadOnly, data->dwFileAttributes&FILE_ATTRIBUTE_READONLY ? True() : False());
		o->Set(syb_returns_isSparseFile, data->dwFileAttributes&FILE_ATTRIBUTE_SPARSE_FILE ? True() : False());
		o->Set(syb_returns_isSystem, data->dwFileAttributes&FILE_ATTRIBUTE_SYSTEM ? True() : False());
		o->Set(syb_returns_isTemporary, data->dwFileAttributes&FILE_ATTRIBUTE_TEMPORARY ? True() : False());
		o->Set(syb_returns_isIntegerityStream, data->dwFileAttributes&FILE_ATTRIBUTE_INTEGRITY_STREAM ? True() : False());
		o->Set(syb_returns_isNoScrubData, data->dwFileAttributes&FILE_ATTRIBUTE_NO_SCRUB_DATA ? True() : False());
		o->Set(syb_returns_isReparsePoint, data->dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT ? True() : False());
		return scope.Close(o);
	}
	static Handle<Function> functionRegister(bool isAsyncVersion) {
		HandleScope scope;
		Handle<FunctionTemplate> t = FunctionTemplate::New(isAsyncVersion ? jsAsync : jsSync);

		//set errmessages
		Handle<Object> errors = Object::New();
		errors->Set(syb_err_wrong_arguments, syb_err_wrong_arguments, global_syb_attr_const);
		errors->Set(syb_err_not_a_constructor, syb_err_not_a_constructor, global_syb_attr_const);
		t->Set(String::NewSymbol("errors"), errors, global_syb_attr_const);

		//set returns
		Handle<Object> returns = Object::New();
		returns->Set(syb_returns_creationTime, syb_returns_creationTime, global_syb_attr_const);
		returns->Set(syb_returns_lastAccessTime, syb_returns_lastAccessTime, global_syb_attr_const);
		returns->Set(syb_returns_lastWriteTime, syb_returns_lastWriteTime, global_syb_attr_const);
		returns->Set(syb_returns_size, syb_returns_size, global_syb_attr_const);
		returns->Set(syb_returns_isArchived, syb_returns_isArchived, global_syb_attr_const);
		returns->Set(syb_returns_isCompressed, syb_returns_isCompressed, global_syb_attr_const);
		returns->Set(syb_returns_isDevice, syb_returns_isDevice, global_syb_attr_const);
		returns->Set(syb_returns_isDirectory, syb_returns_isDirectory, global_syb_attr_const);
		returns->Set(syb_returns_isEncrypted, syb_returns_isEncrypted, global_syb_attr_const);
		returns->Set(syb_returns_isHidden, syb_returns_isHidden, global_syb_attr_const);
		returns->Set(syb_returns_isNotContentIndexed, syb_returns_isNotContentIndexed, global_syb_attr_const);
		returns->Set(syb_returns_isOffline, syb_returns_isOffline, global_syb_attr_const);
		returns->Set(syb_returns_isReadOnly, syb_returns_isReadOnly, global_syb_attr_const);
		returns->Set(syb_returns_isSparseFile, syb_returns_isSparseFile, global_syb_attr_const);
		returns->Set(syb_returns_isSystem, syb_returns_isSystem, global_syb_attr_const);
		returns->Set(syb_returns_isTemporary, syb_returns_isTemporary, global_syb_attr_const);
		returns->Set(syb_returns_isIntegerityStream, syb_returns_isIntegerityStream, global_syb_attr_const);
		returns->Set(syb_returns_isNoScrubData, syb_returns_isNoScrubData, global_syb_attr_const);
		returns->Set(syb_returns_isReparsePoint, syb_returns_isReparsePoint, global_syb_attr_const);

		t->Set(String::NewSymbol("returns"), returns, global_syb_attr_const);

		return scope.Close(t->GetFunction());
	}
private:
	static Handle<Value> jsSync(const Arguments& args) {
		HandleScope scope;
		Handle<Value> result;
		if (args.IsConstructCall()) {
			result = ThrowException(Exception::Error(syb_err_not_a_constructor));
		} else {
			if (args.Length() > 0 && (args[0]->IsString() || args[0]->IsStringObject())) {
				String::Value p(args[0]);
				WIN32_FILE_ATTRIBUTE_DATA data;
				if (GetFileAttributesExW((wchar_t*)*p, GetFileExInfoStandard, &data)) {
					result = attrDataToJs(&data);
				} else {
					result = Undefined();
				}
			} else {
				result = ThrowException(Exception::Error(syb_err_wrong_arguments));
			}
		}
		return scope.Close(result);
	}
	static Handle<Value> jsAsync(const Arguments& args) {
		HandleScope scope;
		Handle<Value> result;
		if (args.IsConstructCall()) {
			result = ThrowException(Exception::Error(syb_err_not_a_constructor));
		} else {
			if (args.Length() > 1 && (args[0]->IsString() || args[0]->IsStringObject()) && args[1]->IsFunction()) {
				workdata *data = new workdata;
				data->req.data = data;
				data->self = Persistent<Object>::New(args.This());
				data->func = Persistent<Function>::New(Handle<Function>::Cast(args[1]));
				String::Value p(args[0]);
				data->path = _wcsdup((wchar_t*)*p);
				data->attr = new WIN32_FILE_ATTRIBUTE_DATA;
				if (uv_queue_work(uv_default_loop(), &data->req, beginWork, afterWork) == 0) {
					result = True();
				} else {
					free(data->path);
					data->self.Dispose();
					if (!data->func.IsEmpty()) {
						data->func.Dispose();
					}
					delete data->attr;
					delete data;
					result = False();
				}
			} else {
				result = ThrowException(Exception::Error(syb_err_wrong_arguments));
			}
		}
		return scope.Close(result);
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
		HandleScope scope;
		workdata *data = (workdata*)req->data;
		if (data->attr) {
			Handle<Value> result = attrDataToJs(data->attr);
			data->func->Call(data->self, 1, &result);
		} else {
			data->func->Call(data->self, 0, NULL);
		}
		data->func.Dispose();
		data->self.Dispose();
		delete data;
	}
};
const Persistent<String> getAttributes::syb_err_wrong_arguments = global_syb_err_wrong_arguments;
const Persistent<String> getAttributes::syb_err_not_a_constructor = global_syb_err_not_a_constructor;
const Persistent<String> getAttributes::syb_returns_creationTime = global_syb_fileAttr_creationTime;
const Persistent<String> getAttributes::syb_returns_lastAccessTime = global_syb_fileAttr_lastAccessTime;
const Persistent<String> getAttributes::syb_returns_lastWriteTime = global_syb_fileAttr_lastWriteTime;
const Persistent<String> getAttributes::syb_returns_size = global_syb_fileAttr_size;
const Persistent<String> getAttributes::syb_returns_isDevice = global_syb_fileAttr_isDevice;
const Persistent<String> getAttributes::syb_returns_isDirectory = global_syb_fileAttr_isDirectory;
const Persistent<String> getAttributes::syb_returns_isCompressed = global_syb_fileAttr_isCompressed;
const Persistent<String> getAttributes::syb_returns_isEncrypted = global_syb_fileAttr_isEncrypted;
const Persistent<String> getAttributes::syb_returns_isSparseFile = global_syb_fileAttr_isSparseFile;
const Persistent<String> getAttributes::syb_returns_isArchived = global_syb_fileAttr_isArchived;
const Persistent<String> getAttributes::syb_returns_isHidden = global_syb_fileAttr_isHidden;
const Persistent<String> getAttributes::syb_returns_isNotContentIndexed = global_syb_fileAttr_isNotContentIndexed;
const Persistent<String> getAttributes::syb_returns_isOffline = global_syb_fileAttr_isOffline;
const Persistent<String> getAttributes::syb_returns_isReadOnly = global_syb_fileAttr_isReadOnly;
const Persistent<String> getAttributes::syb_returns_isSystem = global_syb_fileAttr_isSystem;
const Persistent<String> getAttributes::syb_returns_isTemporary = global_syb_fileAttr_isTemporary;
const Persistent<String> getAttributes::syb_returns_isIntegerityStream = global_syb_fileAttr_isIntegerityStream;
const Persistent<String> getAttributes::syb_returns_isNoScrubData = global_syb_fileAttr_isNoScrubData;
const Persistent<String> getAttributes::syb_returns_isReparsePoint = NODE_PSYMBOL("IS_REPARSE_POINT");