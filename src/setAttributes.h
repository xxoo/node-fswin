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
	static const Persistent<String> syb_param_attr_archive;
	static const Persistent<String> syb_param_attr_hidden;
	static const Persistent<String> syb_param_attr_notContentIndexed;
	static const Persistent<String> syb_param_attr_offline;
	static const Persistent<String> syb_param_attr_readonly;
	static const Persistent<String> syb_param_attr_system;
	static const Persistent<String> syb_param_attr_temporary;
	static const Persistent<String> syb_err_wrong_arguments;
	static const Persistent<String> syb_err_not_a_constructor;
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
		HandleScope scope;
		attrVal *a = new attrVal;
		if (attr->HasOwnProperty(syb_param_attr_archive)) {
			a->archive = attr->Get(syb_param_attr_archive)->ToBoolean()->IsTrue() ? 1 : -1;
		} else {
			a->archive = 0;
		}
		if (attr->HasOwnProperty(syb_param_attr_hidden)) {
			a->hidden = attr->Get(syb_param_attr_hidden)->ToBoolean()->IsTrue() ? 1 : -1;
		} else {
			a->hidden = 0;
		}
		if (attr->HasOwnProperty(syb_param_attr_notContentIndexed)) {
			a->notContentIndexed = attr->Get(syb_param_attr_notContentIndexed)->ToBoolean()->IsTrue() ? 1 : -1;
		} else {
			a->notContentIndexed = 0;
		}
		if (attr->HasOwnProperty(syb_param_attr_offline)) {
			a->offline = attr->Get(syb_param_attr_offline)->ToBoolean()->IsTrue() ? 1 : -1;
		} else {
			a->offline = 0;
		}
		if (attr->HasOwnProperty(syb_param_attr_readonly)) {
			a->readonly = (attr->Get(syb_param_attr_readonly)->ToBoolean()->IsTrue() ? 1 : -1);
		} else {
			a->readonly = 0;
		}
		if (attr->HasOwnProperty(syb_param_attr_system)) {
			a->system = attr->Get(syb_param_attr_system)->ToBoolean()->IsTrue() ? 1 : -1;
		} else {
			a->system = 0;
		}
		if (attr->HasOwnProperty(syb_param_attr_temporary)) {
			a->temporary = attr->Get(syb_param_attr_temporary)->ToBoolean()->IsTrue() ? 1 : -1;
		} else {
			a->temporary = 0;
		}
		return a;
	}
	static Handle<Function> functionRegister(bool isAsyncVersion) {
		HandleScope scope;
		Handle<FunctionTemplate> t = FunctionTemplate::New(isAsyncVersion ? jsAsync : jsSync);

		//set errmessages
		Handle<Object> errors = Object::New();
		errors->Set(syb_err_wrong_arguments, syb_err_wrong_arguments, global_syb_attr_const);
		errors->Set(syb_err_not_a_constructor, syb_err_not_a_constructor, global_syb_attr_const);
		t->Set(String::NewSymbol("errors"), errors, global_syb_attr_const);

		//set params
		Handle<Object> params = Object::New();
		params->Set(syb_param_attr_archive, syb_param_attr_archive, global_syb_attr_const);
		params->Set(syb_param_attr_hidden, syb_param_attr_hidden, global_syb_attr_const);
		params->Set(syb_param_attr_notContentIndexed, syb_param_attr_notContentIndexed, global_syb_attr_const);
		params->Set(syb_param_attr_offline, syb_param_attr_offline, global_syb_attr_const);
		params->Set(syb_param_attr_readonly, syb_param_attr_readonly, global_syb_attr_const);
		params->Set(syb_param_attr_system, syb_param_attr_system, global_syb_attr_const);
		params->Set(syb_param_attr_temporary, syb_param_attr_temporary, global_syb_attr_const);
		t->Set(String::NewSymbol("params"), params, global_syb_attr_const);

		return scope.Close(t->GetFunction());
	}
private:
	static Handle<Value> jsSync(const Arguments& args) {
		HandleScope scope;
		Handle<Value> result;
		if (args.IsConstructCall()) {
			result = ThrowException(Exception::Error(syb_err_not_a_constructor));
		} else {
			if (args.Length() > 1 && (args[0]->IsString() || args[0]->IsStringObject()) && args[1]->IsObject()) {
				attrVal *a = jsToAttrval(Handle<Object>::Cast(args[1]));
				String::Value p(args[0]);
				result = basic((wchar_t*)*p, a) ? True() : False();
				delete a;
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
			if (args.Length() > 1 && (args[0]->IsString() || args[0]->IsStringObject()) && args[1]->IsObject()) {
				workdata *data = new workdata;
				data->req.data = data;
				data->self = Persistent<Object>::New(args.This());
				if (args.Length() > 2 && args[2]->IsFunction()) {
					data->func = Persistent<Function>::New(Handle<Function>::Cast(args[2]));
				}
				String::Value p(args[0]);
				data->path = _wcsdup((wchar_t*)*p);
				data->attr = jsToAttrval(Handle<Object>::Cast(args[1]));
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
		data->result = basic(data->path, data->attr);
		free(data->path);
		delete data->attr;
	}
	static void afterWork(uv_work_t *req, int status) {
		HandleScope scope;
		workdata *data = (workdata*)req->data;
		Handle<Value> p = data->result ? True() : False();
		if (!data->func.IsEmpty()) {
			data->func->Call(data->self, 1, &p);
			data->func.Dispose();
		}
		data->self.Dispose();
		delete data;
	}
};
const Persistent<String> setAttributes::syb_err_wrong_arguments = global_syb_err_wrong_arguments;
const Persistent<String> setAttributes::syb_err_not_a_constructor = global_syb_err_not_a_constructor;
const Persistent<String> setAttributes::syb_param_attr_archive = global_syb_fileAttr_isArchived;
const Persistent<String> setAttributes::syb_param_attr_hidden = global_syb_fileAttr_isHidden;
const Persistent<String> setAttributes::syb_param_attr_notContentIndexed = global_syb_fileAttr_isNotContentIndexed;
const Persistent<String> setAttributes::syb_param_attr_offline = global_syb_fileAttr_isOffline;
const Persistent<String> setAttributes::syb_param_attr_readonly = global_syb_fileAttr_isReadOnly;
const Persistent<String> setAttributes::syb_param_attr_system = global_syb_fileAttr_isSystem;
const Persistent<String> setAttributes::syb_param_attr_temporary = global_syb_fileAttr_isTemporary;