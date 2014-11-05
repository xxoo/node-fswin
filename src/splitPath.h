#pragma once
#include "main.h"

#define SYB_RETURN_PARENT (uint8_t*)"PARENT"
#define SYB_RETURN_NAME (uint8_t*)"NAME"

class splitPath {
public:
	static const struct splitedPath {
		DWORD parentLen;//the length of the parent
		const wchar_t *name;//this could also be considered as the start position of the name
	};
private:
	static const Persistent<String> syb_err_wrong_arguments;
	static const Persistent<String> syb_err_not_a_constructor;
public:
	static splitedPath *basic(const wchar_t *path) {//you need to delete the return value your self if it is not NULL;
		wchar_t *s = L"\\\\", s1 = L'\\';
		DWORD i, j = 0, k = 0, l = (DWORD)wcslen(path), m = (DWORD)wcslen(s);
		if (wcsncmp(s, path, m) == 0) {//is network path
			for (i = m + 1; i < l - 1; i++) {
				if (path[i] == s1) {
					if (++k == 2) {
						j = i + 1;
						break;
					}
				}
			}
			if (k == 2) {
				k = 0;
				for (i = l - 2; i > j + 1; i--) {
					if (path[i] == s1) {
						j = i;
						k = 1;
						break;
					}
				}
			} else {
				k = 0;
			}
		} else {//is local path
			for (i = l - 2; i > 1; i--) {
				if (path[i] == s1) {
					if (j == 0) {//perhaps it's a rootdir
						j = i + 1;
					} else {//it's not a rootdir
						j -= 1;
						k = 1;
						break;
					}
				}
			}
		}
		splitedPath *r = new splitedPath;
		r->parentLen = j;
		r->name = &path[j > 0 ? j + k : j];
		return r;
	}
	//this function returns an Object with two properties: PARENT and NAME
	//the parent property could be empty if path is a rootdir
	static Handle<Object> js(Handle<String> path) {
		Isolate *isolate = Isolate::GetCurrent();
		EscapableHandleScope scope(isolate);
		String::Value p1(path);
		splitedPath *s = basic((wchar_t*)*p1);
		Local<Object> r = Object::New(isolate);
		r->Set(String::NewFromOneByte(isolate, SYB_RETURN_PARENT), String::NewFromTwoByte(isolate, *p1, String::kNormalString, s->parentLen));
		r->Set(String::NewFromOneByte(isolate, SYB_RETURN_NAME), String::NewFromTwoByte(isolate, (uint16_t*)s->name));
		delete s;
		return scope.Escape(r);
	}
	static Handle<Function> functionRegister() {
		Isolate *isolate = Isolate::GetCurrent();
		EscapableHandleScope scope(isolate);
		Local<String> tmp;
		Local<FunctionTemplate> t = FunctionTemplate::New(isolate, jsSync);

		//set errmessages
		Local<Object> errors = Object::New(isolate);
		tmp = String::NewFromOneByte(isolate, SYB_ERR_WRONG_ARGUMENTS);
		errors->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_ERR_NOT_A_CONSTRUCTOR);
		errors->Set(tmp, tmp, SYB_ATTR_CONST);
		t->Set(String::NewFromOneByte(isolate, SYB_ERRORS), errors, SYB_ATTR_CONST);

		//set properties of the return value
		Local<Object> returns = Object::New(isolate);
		tmp = String::NewFromOneByte(isolate, SYB_RETURN_PARENT);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		tmp = String::NewFromOneByte(isolate, SYB_RETURN_NAME);
		returns->Set(tmp, tmp, SYB_ATTR_CONST);
		t->Set(String::NewFromOneByte(isolate, SYB_RETURNS), returns, SYB_ATTR_CONST);

		return scope.Escape(t->GetFunction());
	}
private:
	static void jsSync(const FunctionCallbackInfo<Value>& args) {
		Isolate *isolate = Isolate::GetCurrent();
		HandleScope scope(isolate);
		Local<Value> result;
		if (args.IsConstructCall()) {
			result = isolate->ThrowException(Exception::Error(String::NewFromOneByte(isolate, SYB_ERR_NOT_A_CONSTRUCTOR)));
		} else {
			if (args.Length() > 0 && (args[0]->IsString() || args[0]->IsStringObject())) {
				result = js(Local<String>::Cast(args[0]));
			} else {
				result = isolate->ThrowException(Exception::Error(String::NewFromOneByte(isolate, SYB_ERR_WRONG_ARGUMENTS)));
			}
		}
		args.GetReturnValue().Set(result);
	}
};