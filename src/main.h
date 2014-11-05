#pragma once
#define UNICODE
#include <node.h>
#include <node_object_wrap.h>
#include <uv.h>
//#include <iostream>
#pragma comment(lib, "node.lib")

using namespace v8;
using namespace node;

#define SYB_ERR_WRONG_ARGUMENTS (uint8_t*)"WRONG_ARGUMENTS"
#define SYB_ERR_NOT_A_CONSTRUCTOR (uint8_t*)"THIS_FUNCTION_IS_NOT_A_CONSTRUCTOR"
#define SYB_ERR_INITIALIZATION_FAILED (uint8_t*)"INITIALIZATION_FAILED"
#define SYB_EVT_ERR (uint8_t*)"ERROR"
#define SYB_EVT_END (uint8_t*)"ENDED"
#define SYB_EVT_SUCCEEDED (uint8_t*)"SUCCEEDED"
#define SYB_EVT_FAILED (uint8_t*)"FAILED"
#define SYB_FILEATTR_ISARCHIVED (uint8_t*)"IS_ARCHIVED"
#define SYB_FILEATTR_ISHIDDEN (uint8_t*)"IS_HIDDEN"
#define SYB_FILEATTR_ISNOTCONTENTINDEXED (uint8_t*)"IS_NOT_CONTENT_INDEXED"
#define SYB_FILEATTR_ISOFFLINE (uint8_t*)"IS_OFFLINE"
#define SYB_FILEATTR_ISREADONLY (uint8_t*)"IS_READ_ONLY"
#define SYB_FILEATTR_ISSYSTEM (uint8_t*)"IS_SYSTEM"
#define SYB_FILEATTR_ISTEMPORARY (uint8_t*)"IS_TEMPORARY"
#define SYB_FILEATTR_CREATIONTIME (uint8_t*)"CREATION_TIME"
#define SYB_FILEATTR_LASTACCESSTIME (uint8_t*)"LAST_ACCESS_TIME"
#define SYB_FILEATTR_LASTWRITETIME (uint8_t*)"LAST_WRITE_TIME"
#define SYB_FILEATTR_SIZE (uint8_t*)"SIZE"
#define SYB_FILEATTR_ISDIRECTORY (uint8_t*)"IS_DIRECTORY"
#define SYB_FILEATTR_ISCOMPRESSED (uint8_t*)"IS_COMPRESSED"
#define SYB_FILEATTR_ISENCRYPTED (uint8_t*)"IS_ENCRYPTED"
#define SYB_FILEATTR_ISSPARSEFILE (uint8_t*)"IS_SPARSE_FILE"
#define SYB_FILEATTR_ISDEVICE (uint8_t*)"IS_DEVICE"
#define SYB_FILEATTR_ISINTEGERITYSTREAM (uint8_t*)"IS_INTEGRITY_STREAM"
#define SYB_FILEATTR_ISNOSCRUBDATA (uint8_t*)"IS_NO_SCRUB_DATA"

#define SYB_ERRORS (uint8_t*)"errors"
#define SYB_RETURNS (uint8_t*)"returns"
#define SYB_EVENTS (uint8_t*)"events"
#define SYB_OPTIONS (uint8_t*)"options"
#define SYB_PARAMS (uint8_t*)"params"

#define SYB_ATTR_CONST (PropertyAttribute)(ReadOnly | DontDelete)

#ifndef FILE_ATTRIBUTE_INTEGRITY_STREAM
#	define FILE_ATTRIBUTE_INTEGRITY_STREAM 0x8000
#endif

#ifndef FILE_ATTRIBUTE_NO_SCRUB_DATA
#	define FILE_ATTRIBUTE_NO_SCRUB_DATA 0x20000
#endif

#ifndef GetFinalPathNameByHandle
typedef DWORD(WINAPI *GetFinalPathNameByHandle)(__in HANDLE hFile, __out_ecount(cchFilePath) LPWSTR lpszFilePath, __in DWORD cchFilePath, __in DWORD dwFlags);
static const GetFinalPathNameByHandle GetFinalPathNameByHandleW = (GetFinalPathNameByHandle)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "GetFinalPathNameByHandleW");
#endif

#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

static wchar_t *getCurrentPathByHandle(HANDLE hnd) {
	wchar_t *r = NULL;
	if ((void*)GetFinalPathNameByHandleW) {//check whether GetFinalPathNameByHandleW is supported
		DWORD sz = GetFinalPathNameByHandleW(hnd, NULL, 0, FILE_NAME_NORMALIZED);
		if (sz > 0) {
			wchar_t *s = (wchar_t*)malloc(sizeof(wchar_t)*sz);
			wchar_t *s1 = L"\\\\?\\UNC\\";//for network paths
			wchar_t *s2 = L"\\\\?\\";//for local paths
			DWORD sz1 = (DWORD)wcslen(s1);
			DWORD sz2 = (DWORD)wcslen(s2);
			GetFinalPathNameByHandleW(hnd, s, sz, FILE_NAME_NORMALIZED);
			if (wcsncmp(s, s1, sz1) == 0) {
				sz = sz1 - 2;
				s[sz] = L'\\';
			} else if (wcsncmp(s, s2, sz2) == 0 && ((s[sz2] >= L'a'&&s[sz2] <= L'z') || (s[sz2] >= L'A'&&s[sz2] <= L'Z')) && s[sz2 + 1] == L':') {
				sz = (DWORD)wcslen(s2);
			} else {
				sz = 0;
			}
			if (sz > 0) {
				r = _wcsdup(&s[sz]);
				free(s);
			} else {
				r = s;
			}
		}
	}
	return r;
}

static bool ensurePrivilege(const wchar_t *privilegeName) {
	bool result = false;
	HANDLE hToken;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
		LUID tkid;
		if (LookupPrivilegeValueW(NULL, privilegeName, &tkid)) {
			PRIVILEGE_SET ps;
			ps.PrivilegeCount = 1;
			ps.Control = PRIVILEGE_SET_ALL_NECESSARY;
			ps.Privilege[0].Luid = tkid;
			ps.Privilege[0].Attributes = SE_PRIVILEGE_ENABLED;
			BOOL chkresult;
			if (PrivilegeCheck(hToken, &ps, &chkresult)) {
				if (chkresult) {
					result = true;
				} else {
					TOKEN_PRIVILEGES tp;
					tp.PrivilegeCount = 1;
					tp.Privileges[0] = ps.Privilege[0];
					result = AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL) ? true : false;
				}
			}
		}
		CloseHandle(hToken);
	}
	return result;
}

static ULONGLONG combineHiLow(const DWORD hi, const DWORD low) {
	ULARGE_INTEGER ul;
	ul.HighPart = hi;
	ul.LowPart = low;
	return ul.QuadPart;
}

static double fileTimeToJsDateVal(const FILETIME *ft) {//Date::New(fileTimeToJsDateVal(&filetime)) converts FILETIME to javascript date
	return (double)(combineHiLow(ft->dwHighDateTime, ft->dwLowDateTime) / 10000 - 11644473600000);
}
/*
struct pathCom {
	wchar_t *shortName;
	wchar_t *longName;
	pathCom *next;
};
static pathCom *getPathComs(const wchar_t *path, const wchar_t *shortPath) {
	pathCom *result = NULL;
	pathCom *lst = NULL;
	USHORT i = 0;
	USHORT j = 0;
	for (i = 0; i <= wcslen(path); i++) {
		if (i == wcslen(path) || path[i] == L'\\') {
			pathCom *com = new pathCom;
			com->next = NULL;
			com->shortName = NULL;
			//com->longName = _wcsdup(path);
			size_t sz = i - j + 1;
			com->longName = new wchar_t[sz];
			wcsncpy_s(com->longName, sz, &path[j], sz - 1);
			//std::wcout << com->longName << std::endl;
			if (!lst) {
				result = com;
			} else {
				lst->next = com;
			}
			lst = com;
			j = i + 1;
		}
	}
	j = 0;
	lst = result;
	for (i = 0; i <= wcslen(shortPath); i++) {
		if (i == wcslen(shortPath) || shortPath[i] == L'\\') {
			if (wcsncmp(lst->longName, &shortPath[j], wcslen(lst->longName)) != 0) {
				size_t sz = i - j + 1;
				lst->shortName = new wchar_t[sz];
				wcsncpy_s(lst->shortName, sz, &shortPath[j], sz - 1);
			}
			lst = lst->next;
			j = i + 1;
		}
	}
	return result;
}
static void freePathCom(const pathCom *p) {
	if (p->next) {
		freePathCom(p->next);
	}
	if (p->shortName) {
		delete p->shortName;
	}
	if (p->longName) {
		delete p->longName;
	}
	delete p;
}
static bool pathComComp(const pathCom *p, const wchar_t *path, size_t len=0) {
	USHORT i = 0;
	USHORT j = 0;
	const pathCom *c = p;
	if (!len) {
		len = wcslen(path);
	}
	for (i = 0; i <= len; i++) {
		if (i == len || path[i] == L'\\') {
			if (!c || (wcsncmp(c->longName, &path[j], MAX(i - j, wcslen(c->longName)))) != 0 && (!c->shortName || wcsncmp(c->shortName, &path[j], MAX(i - j, wcslen(c->shortName))) != 0)) {
				//std::wcout << L'wrong' << std::endl;
				return false;
			}
			c = c->next;
			j = i + 1;
		}
	}
	return true;
}
*/