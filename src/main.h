#pragma once
#define UNICODE
#include <node.h>
#include <node_object_wrap.h>
#include <uv.h>
//#include <iostream>

//#pragma comment(lib, "iojs.lib")

using namespace v8;
using namespace node;

# if NODE_MODULE_VERSION < 11
#	define AFTERWORKCB(name) void (name)(uv_work_t *req)
#else
#	define AFTERWORKCB(name) void (name)(uv_work_t *req, int status)
#endif

#if NODE_MODULE_VERSION < 14
#	define ASYNCCB(name) void (name)(uv_async_t *hnd, int status)
#	define ISOLATE
#	define ISOLATE_C
#	define ISOLATE_NEW
#	define ISOLATE_NEW_ARGS
#	define RETURNTYPE Handle
#	define RETURN(v) return scope.Close(v)
#	define RETURN_SCOPE(v) RETURN(v)
#	define NEWSTRING(v) String::New((v))
#	define NEWSTRING_TWOBYTES(v) String::New((uint16_t*)(v))
#	define NEWSTRING_TWOBYTES_LEN(v, l) String::New((uint16_t*)(v), (l))
#	define THROWEXCEPTION(v) ThrowException(Exception::Error(String::New((v))))
#	define JSFUNC(name) Handle<Value> (name)(const Arguments& args)
#	define PERSISTENT_NEW(name, v, t) (name) = Persistent<t>::New((v))
#	define PERSISTENT_CONV(v, t) (v)
#	define PERSISTENT_RELEASE(name) (name).Dispose();(name).Clear()
#	define SCOPE HandleScope scope
#	define SCOPE_ESCAPABLE SCOPE
#	define OBJ_HANDLE handle_
#	define THEASYNCOVERLAP overlapped
#	define SETWITHATTR(obj, key, value, attr) (obj)->Set((key), (value), (attr))
#	define NEWFUNCTION(call) FunctionTemplate::New((call))->GetFunction()
#else
#	define ASYNCCB(name) void (name)(uv_async_t *hnd)
#	define ISOLATE isolate
#	define ISOLATE_C isolate,
#	define ISOLATE_NEW Isolate *isolate = Isolate::GetCurrent()
#	define ISOLATE_NEW_ARGS Isolate *isolate = args.GetIsolate()
#	define RETURNTYPE Local
#	define RETURN(v) args.GetReturnValue().Set((v))
#	define RETURN_SCOPE(v) return scope.Escape((v))
#	define NEWSTRING(v) String::NewFromOneByte(isolate, (uint8_t*)(v))
#	define NEWSTRING_TWOBYTES(v) String::NewFromTwoByte(isolate, (uint16_t*)(v))
#	define NEWSTRING_TWOBYTES_LEN(v, l) String::NewFromTwoByte(isolate, (uint16_t*)(v), String::kNormalString, (l))
#	define THROWEXCEPTION(v) isolate->ThrowException(Exception::Error(String::NewFromOneByte(isolate, (uint8_t*)(v))))
#	define JSFUNC(name) void (name)(const FunctionCallbackInfo<Value>& args)
#	define PERSISTENT_NEW(name, v, t) (name).Reset(isolate, (v))
#	define PERSISTENT_CONV(v, t) Local<t>::New(isolate, (v))
#	define PERSISTENT_RELEASE(name) (name).Reset()
#	define SCOPE HandleScope scope(isolate)
#	define SCOPE_ESCAPABLE EscapableHandleScope scope(isolate)
#	define OBJ_HANDLE persistent()
#	define THEASYNCOVERLAP u.io.overlapped
#	define SETWITHATTR(obj, key, value, attr) (obj)->ForceSet((key), (value), (attr))
#	define NEWFUNCTION(call) Function::New(isolate, (call))
#endif

#define SYB_ERR_WRONG_ARGUMENTS "WRONG_ARGUMENTS"
#define SYB_ERR_NOT_A_CONSTRUCTOR "THIS_FUNCTION_IS_NOT_A_CONSTRUCTOR"
#define SYB_ERR_INITIALIZATION_FAILED "INITIALIZATION_FAILED"
#define SYB_EVT_ERR "ERROR"
#define SYB_EVT_END "ENDED"
#define SYB_EVT_SUCCEEDED "SUCCEEDED"
#define SYB_EVT_FAILED "FAILED"
#define SYB_FILEATTR_ISARCHIVED "IS_ARCHIVED"
#define SYB_FILEATTR_ISHIDDEN "IS_HIDDEN"
#define SYB_FILEATTR_ISNOTCONTENTINDEXED "IS_NOT_CONTENT_INDEXED"
#define SYB_FILEATTR_ISOFFLINE "IS_OFFLINE"
#define SYB_FILEATTR_ISREADONLY "IS_READ_ONLY"
#define SYB_FILEATTR_ISSYSTEM "IS_SYSTEM"
#define SYB_FILEATTR_ISTEMPORARY "IS_TEMPORARY"
#define SYB_FILEATTR_CREATIONTIME "CREATION_TIME"
#define SYB_FILEATTR_LASTACCESSTIME "LAST_ACCESS_TIME"
#define SYB_FILEATTR_LASTWRITETIME "LAST_WRITE_TIME"
#define SYB_FILEATTR_SIZE "SIZE"
#define SYB_FILEATTR_ISDIRECTORY "IS_DIRECTORY"
#define SYB_FILEATTR_ISCOMPRESSED "IS_COMPRESSED"
#define SYB_FILEATTR_ISENCRYPTED "IS_ENCRYPTED"
#define SYB_FILEATTR_ISSPARSEFILE "IS_SPARSE_FILE"
#define SYB_FILEATTR_ISDEVICE "IS_DEVICE"
#define SYB_FILEATTR_ISINTEGERITYSTREAM "IS_INTEGRITY_STREAM"
#define SYB_FILEATTR_ISNOSCRUBDATA "IS_NO_SCRUB_DATA"

#define SYB_ERRORS "errors"
#define SYB_RETURNS "returns"
#define SYB_EVENTS "events"
#define SYB_OPTIONS "options"
#define SYB_PARAMS "params"
#define SYB_PROTOTYPE "prototype"

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