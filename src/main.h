#pragma once
#define UNICODE
#include <node.h>
#pragma comment(lib, "node.lib")

using namespace v8;
using namespace node;

static const PropertyAttribute global_syb_attr_const = (PropertyAttribute)(ReadOnly | DontDelete);
static const Persistent<String> global_syb_err_wrong_arguments = NODE_PSYMBOL("WRONG_ARGUMENTS");
static const Persistent<String> global_syb_err_not_a_constructor = NODE_PSYMBOL("THIS_FUNCTION_IS_NOT_A_CONSTRUCTOR");
static const Persistent<String> global_syb_err_initialization_failed = NODE_PSYMBOL("INITIALIZATION_FAILED");
static const Persistent<String> global_syb_evt_err = NODE_PSYMBOL("ERROR");
static const Persistent<String> global_syb_evt_end = NODE_PSYMBOL("ENDED");
static const Persistent<String> global_syb_evt_succeeded = NODE_PSYMBOL("SUCCEEDED");
static const Persistent<String> global_syb_evt_failed = NODE_PSYMBOL("FAILED");

static const Persistent<String> global_syb_fileAttr_isArchived = NODE_PSYMBOL("IS_ARCHIVED");
static const Persistent<String> global_syb_fileAttr_isHidden = NODE_PSYMBOL("IS_HIDDEN");
static const Persistent<String> global_syb_fileAttr_isNotContentIndexed = NODE_PSYMBOL("IS_NOT_CONTENT_INDEXED");
static const Persistent<String> global_syb_fileAttr_isOffline = NODE_PSYMBOL("IS_OFFLINE");
static const Persistent<String> global_syb_fileAttr_isReadOnly = NODE_PSYMBOL("IS_READ_ONLY");
static const Persistent<String> global_syb_fileAttr_isSystem = NODE_PSYMBOL("IS_SYSTEM");
static const Persistent<String> global_syb_fileAttr_isTemporary = NODE_PSYMBOL("IS_TEMPORARY");
static const Persistent<String> global_syb_fileAttr_creationTime = NODE_PSYMBOL("CREATION_TIME");
static const Persistent<String> global_syb_fileAttr_lastAccessTime = NODE_PSYMBOL("LAST_ACCESS_TIME");
static const Persistent<String> global_syb_fileAttr_lastWriteTime = NODE_PSYMBOL("LAST_WRITE_TIME");
static const Persistent<String> global_syb_fileAttr_size = NODE_PSYMBOL("SIZE");
static const Persistent<String> global_syb_fileAttr_isDirectory = NODE_PSYMBOL("IS_DIRECTORY");
static const Persistent<String> global_syb_fileAttr_isCompressed = NODE_PSYMBOL("IS_COMPRESSED");
static const Persistent<String> global_syb_fileAttr_isEncrypted = NODE_PSYMBOL("IS_ENCRYPTED");
static const Persistent<String> global_syb_fileAttr_isSparseFile = NODE_PSYMBOL("IS_SPARSE_FILE");
static const Persistent<String> global_syb_fileAttr_isDevice = NODE_PSYMBOL("IS_DEVICE");
static const Persistent<String> global_syb_fileAttr_isIntegerityStream = NODE_PSYMBOL("IS_INTEGRITY_STREAM");
static const Persistent<String> global_syb_fileAttr_isNoScrubData = NODE_PSYMBOL("IS_NO_SCRUB_DATA");

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
static Handle<String> getCurrentPathByHandle(HANDLE hnd) {
	HandleScope scope;
	Handle<String> r;
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
			r = String::New((uint16_t*)(sz > 0 ? &s[sz] : s));
			free(s);
		}
	}
	if (r.IsEmpty()) {
		r = String::NewSymbol("");
	}
	return scope.Close(r);
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