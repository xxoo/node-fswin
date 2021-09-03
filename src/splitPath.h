#pragma once
#include "common.h"

class splitPath {
public:
	const struct splitedPath {
		DWORD parentLen;//the length of the parent
		const wchar_t *name;//this could also be considered as the start position of the name
	};
	static splitedPath *func(const wchar_t *path) {//you need to delete the return value your self
		const wchar_t *s = L"\\\\", s1 = L'\\';
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
	static napi_value init(napi_env env) {
		napi_value f;
		napi_create_function(env, NULL, 0, sync, NULL, &f);
		return f;
	}
private:
	static napi_value sync(napi_env env, napi_callback_info info) {
		napi_value result;
		napi_get_new_target(env, info, &result);
		if (result) {
			result = NULL;
			napi_throw_error(env, SYB_EXP_INVAL, SYB_ERR_NOT_A_CONSTRUCTOR);
		} else {
			napi_value argv;
			size_t argc = 1;
			napi_get_cb_info(env, info, &argc, &argv, NULL, NULL);
			if (argc < 1) {
				napi_throw_error(env, SYB_EXP_INVAL, SYB_ERR_WRONG_ARGUMENTS);
			} else {
				size_t str_len;
				napi_value tmp;
				napi_coerce_to_string(env, argv, &tmp);
				napi_get_value_string_utf16(env, tmp, NULL, 0, &str_len);
				str_len += 1;
				wchar_t *str = new wchar_t[str_len];
				napi_get_value_string_utf16(env, tmp, (char16_t*)str, str_len, NULL);
				splitedPath *s = func(str);
				napi_create_object(env, &result);
				napi_create_string_utf16(env, (char16_t*)str, s->parentLen, &tmp);
				napi_set_named_property(env, result, "PARENT", tmp);
				napi_create_string_utf16(env, (char16_t*)s->name, wcslen(s->name), &tmp);
				napi_set_named_property(env, result, "NAME", tmp);
				delete s;
				delete[]str;
			}
		}
		return result;
	}
};