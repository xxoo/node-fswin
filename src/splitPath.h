#pragma once
#include "main.h"

#define SYB_RETURN_PARENT "PARENT"
#define SYB_RETURN_NAME "NAME"

class splitPath {
public:
	const struct splitedPath {
		DWORD parentLen;//the length of the parent
		const wchar_t *name;//this could also be considered as the start position of the name
	};

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
		ISOLATE_NEW;
		SCOPE_ESCAPABLE;
		String::Value p1(path);
		splitedPath *s = basic((wchar_t*)*p1);
		RETURNTYPE<Object> r = Object::New(ISOLATE);
		r->Set(NEWSTRING(SYB_RETURN_PARENT), NEWSTRING_TWOBYTES_LEN(*p1, s->parentLen));
		r->Set(NEWSTRING(SYB_RETURN_NAME), NEWSTRING_TWOBYTES(s->name));
		delete s;
		RETURN_SCOPE(r);
	}
	static Handle<Function> functionRegister() {
		ISOLATE_NEW;
		SCOPE_ESCAPABLE;
		RETURNTYPE<String> tmp;
		RETURNTYPE<Function> t = NEWFUNCTION(jsSync);

		//set errmessages
		RETURNTYPE<Object> errors = Object::New(ISOLATE);
		tmp = NEWSTRING(SYB_ERR_WRONG_ARGUMENTS);
		SETWITHATTR(errors, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_ERR_NOT_A_CONSTRUCTOR);
		SETWITHATTR(errors, tmp, tmp, SYB_ATTR_CONST);
		SETWITHATTR(t, NEWSTRING(SYB_ERRORS), errors, SYB_ATTR_CONST);

		//set properties of the return value
		RETURNTYPE<Object> returns = Object::New(ISOLATE);
		tmp = NEWSTRING(SYB_RETURN_PARENT);
		SETWITHATTR(returns, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_RETURN_NAME);
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
				result = js(RETURNTYPE<String>::Cast(args[0]));
			} else {
				result = THROWEXCEPTION(SYB_ERR_WRONG_ARGUMENTS);
			}
		}
		RETURN(result);
	}
};