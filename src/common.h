#pragma once
#include <windows.h>
#include <node_api.h>
//#include <iostream>

#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

constexpr auto SYB_EXP_INVAL = "EINVAL";
constexpr auto SYB_ERR_WRONG_ARGUMENTS = "WRONG_ARGUMENTS";
constexpr auto SYB_ERR_NOT_A_CONSTRUCTOR = "THIS_FUNCTION_IS_NOT_A_CONSTRUCTOR";
constexpr auto SYB_FILEATTR_ISARCHIVED = "IS_ARCHIVED";
constexpr auto SYB_FILEATTR_ISHIDDEN = "IS_HIDDEN";
constexpr auto SYB_FILEATTR_ISNOTCONTENTINDEXED = "IS_NOT_CONTENT_INDEXED";
constexpr auto SYB_FILEATTR_ISOFFLINE = "IS_OFFLINE";
constexpr auto SYB_FILEATTR_ISREADONLY = "IS_READ_ONLY";
constexpr auto SYB_FILEATTR_ISSYSTEM = "IS_SYSTEM";
constexpr auto SYB_FILEATTR_ISTEMPORARY = "IS_TEMPORARY";
constexpr auto SYB_FILEATTR_CREATIONTIME = "CREATION_TIME";
constexpr auto SYB_FILEATTR_LASTACCESSTIME = "LAST_ACCESS_TIME";
constexpr auto SYB_FILEATTR_LASTWRITETIME = "LAST_WRITE_TIME";
constexpr auto SYB_FILEATTR_SIZE = "SIZE";
constexpr auto SYB_FILEATTR_ISDIRECTORY = "IS_DIRECTORY";
constexpr auto SYB_FILEATTR_ISCOMPRESSED = "IS_COMPRESSED";
constexpr auto SYB_FILEATTR_ISENCRYPTED = "IS_ENCRYPTED";
constexpr auto SYB_FILEATTR_ISSPARSEFILE = "IS_SPARSE_FILE";
constexpr auto SYB_FILEATTR_ISDEVICE = "IS_DEVICE";
constexpr auto SYB_FILEATTR_ISINTEGERITYSTREAM = "IS_INTEGRITY_STREAM";
constexpr auto SYB_FILEATTR_ISNOSCRUBDATA = "IS_NO_SCRUB_DATA";
constexpr auto SYB_FILEATTR_ISRECALLONDATAACCESS = "IS_RECALL_ON_DATA_ACCESS";
constexpr auto SYB_FILEATTR_ISRECALLONOPEN = "IS_RECALL_ON_OPEN";
constexpr auto SYB_FILEATTR_ISVIRTUAL = "IS_VIRTUAL";
constexpr auto SYB_FILEATTR_ISEA = "IS_EA";
constexpr auto SYB_FILEATTR_ISPINNED = "IS_PINNED";
constexpr auto SYB_FILEATTR_ISUNPINNED = "IS_UNPINNED";
constexpr auto SYB_FILEATTR_RAWATTRS = "RAW_ATTRIBUTES";
constexpr auto NETWORK_PATH = L"\\\\?\\UNC\\";
constexpr auto LOCALE_PATH = L"\\\\?\\";
constexpr auto MAX_LONG_PATH = 32767;

typedef CHAR(__stdcall* _RtlSetThreadPlaceholderCompatibilityMode)(__in CHAR Mode);
static const _RtlSetThreadPlaceholderCompatibilityMode RtlSetThreadPlaceholderCompatibilityMode = (_RtlSetThreadPlaceholderCompatibilityMode)GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlSetThreadPlaceholderCompatibilityMode");

wchar_t *getCurrentPathByHandle(HANDLE hnd) {
	wchar_t* r = NULL;
	DWORD sz0, sz = GetFinalPathNameByHandleW(hnd, NULL, 0, FILE_NAME_NORMALIZED);
	if (sz > 0) {
		wchar_t* s = new wchar_t[sz];
		DWORD sz1 = (DWORD)wcslen(NETWORK_PATH);
		DWORD sz2 = (DWORD)wcslen(LOCALE_PATH);
		GetFinalPathNameByHandleW(hnd, s, sz, FILE_NAME_NORMALIZED);
		if (wcsncmp(s, NETWORK_PATH, sz1) == 0) {
			sz0 = sz1 - 2;
			s[sz0] = L'\\';
		} else if (wcsncmp(s, LOCALE_PATH, sz2) == 0 && ((s[sz2] >= L'a'&&s[sz2] <= L'z') || (s[sz2] >= L'A'&&s[sz2] <= L'Z')) && s[sz2 + 1] == L':') {
			sz0 = (DWORD)wcslen(LOCALE_PATH);
		} else {
			sz0 = 0;
		}
		if (sz0 > 0) {
			size_t l = sz - sz0;
			r = new wchar_t[l];
			wcscpy_s(r, l, &s[sz0]);
			delete[]s;
		} else {
			r = s;
		}
	}
	return r;
}
bool ensurePrivilege(const char *privilegeName) {
	bool result = false;
	HANDLE hToken;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
		LUID tkid;
		if (LookupPrivilegeValueA(NULL, privilegeName, &tkid)) {
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
ULONGLONG combineHiLow(const DWORD hi, const DWORD low) {
	ULARGE_INTEGER ul;
	ul.HighPart = hi;
	ul.LowPart = low;
	return ul.QuadPart;
}
int64_t fileTimeToJsDateVal(const FILETIME *ft) {//converts FILETIME to javascript date value
	return combineHiLow(ft->dwHighDateTime, ft->dwLowDateTime) / 10000 - 11644473600000;
}