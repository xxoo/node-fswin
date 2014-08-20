#pragma once
#include "main.h"

class splitPath {
public:
	static const Persistent<String> syb_return_parent;
	static const Persistent<String> syb_return_name;
	static const struct splitedPath {
		DWORD parentLen;//the length of the parent
		const wchar_t *name;//this could be also considered as the start position of the name
	};
private:
	static const Persistent<String> syb_err_wrong_arguments;
	static const Persistent<String> syb_err_not_a_constructor;
public:
	static splitedPath *basic(const wchar_t *path) {//you need to delete the return value your self if it is not NULL;
		wchar_t *s = L"\\\\", s1 = L'\\';
		DWORD i, j = 0, k = 0, l = (DWORD)wcslen(path), m = (DWORD)wcslen(s);
		if (wcsncmp(s, path, m) == 0) {//is network path
			for (i = m + 1; i<l - 1; i++) {
				if (path[i] == s1) {
					if (++k == 2) {
						j = i + 1;
						break;
					}
				}
			}
			if (k == 2) {
				k = 0;
				for (i = l - 2; i>j + 1; i--) {
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
		HandleScope scope;
		String::Value p1(path);
		splitedPath *s = basic((wchar_t*)*p1);
		Handle<Object> r = Object::New();
		r->Set(syb_return_parent, String::New(*p1, s->parentLen));
		r->Set(syb_return_name, String::New((uint16_t*)s->name));
		delete s;
		return scope.Close(r);
	}
	static Handle<Function> functionRegister() {
		HandleScope scope;
		Handle<FunctionTemplate> t = FunctionTemplate::New(jsSync);

		//set errmessages
		Handle<Object> errors = Object::New();
		errors->Set(syb_err_wrong_arguments, syb_err_wrong_arguments, global_syb_attr_const);
		errors->Set(syb_err_not_a_constructor, syb_err_not_a_constructor, global_syb_attr_const);
		t->Set(String::NewSymbol("errors"), errors, global_syb_attr_const);

		//set properties of the return value
		Handle<Object> returns = Object::New();
		returns->Set(syb_return_parent, syb_return_parent, global_syb_attr_const);
		returns->Set(syb_return_name, syb_return_name, global_syb_attr_const);
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
				result = js(Handle<String>::Cast(args[0]));
			} else {
				result = ThrowException(Exception::Error(syb_err_wrong_arguments));
			}
		}
		return scope.Close(result);
	}
};
const Persistent<String> splitPath::syb_return_parent = NODE_PSYMBOL("PARENT");
const Persistent<String> splitPath::syb_return_name = NODE_PSYMBOL("NAME");
const Persistent<String> splitPath::syb_err_wrong_arguments = global_syb_err_wrong_arguments;
const Persistent<String> splitPath::syb_err_not_a_constructor = global_syb_err_not_a_constructor;