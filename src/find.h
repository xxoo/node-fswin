#pragma once
#include "main.h"

#ifndef IO_REPARSE_TAG_DEDUP
#	define IO_REPARSE_TAG_DEDUP (0x80000013L)
#endif
#ifndef IO_REPARSE_TAG_NFS
#	define IO_REPARSE_TAG_NFS (0x80000014L)
#endif
#ifndef IO_REPARSE_TAG_FILE_PLACEHOLDER
#	define IO_REPARSE_TAG_FILE_PLACEHOLDER (0x80000015L)
#endif

class find {
public:
	static const Persistent<String> syb_returns_longName;
	static const Persistent<String> syb_returns_shortName;
	static const Persistent<String> syb_returns_creationTime;
	static const Persistent<String> syb_returns_lastAccessTime;
	static const Persistent<String> syb_returns_lastWriteTime;
	static const Persistent<String> syb_returns_size;
	static const Persistent<String> syb_returns_reparsePointTag;
	static const Persistent<String> syb_returns_isArchived;
	static const Persistent<String> syb_returns_isCompressed;
	static const Persistent<String> syb_returns_isDirectory;
	static const Persistent<String> syb_returns_isEncrypted;
	static const Persistent<String> syb_returns_isHidden;
	static const Persistent<String> syb_returns_isNotContentIndexed;
	static const Persistent<String> syb_returns_isOffline;
	static const Persistent<String> syb_returns_isReadOnly;
	static const Persistent<String> syb_returns_isSparseFile;
	static const Persistent<String> syb_returns_isSystem;
	static const Persistent<String> syb_returns_isTemporary;
	static const Persistent<String> syb_reparsePoint_unknown;
	static const Persistent<String> syb_reparsePoint_csv;
	static const Persistent<String> syb_reparsePoint_dedup;
	static const Persistent<String> syb_reparsePoint_dfs;
	static const Persistent<String> syb_reparsePoint_dfsr;
	static const Persistent<String> syb_reparsePoint_hsm;
	static const Persistent<String> syb_reparsePoint_hsm2;
	static const Persistent<String> syb_reparsePoint_mountPoint;
	static const Persistent<String> syb_reparsePoint_nfs;
	static const Persistent<String> syb_reparsePoint_placeHolder;
	static const Persistent<String> syb_reparsePoint_sis;
	static const Persistent<String> syb_reparsePoint_symlink;
	static const Persistent<String> syb_reparsePoint_wim;
	static const struct resultData {//this is a linked table
		WIN32_FIND_DATAW data;
		resultData *next;
	};
	//progressive callback type, if this callback returns true, the search will stop immediately. the contents of info will be rewritten or released after the callback returns, so make a copy before starting a new thread if you still need to use it
	typedef bool(*findResultCall)(const WIN32_FIND_DATAW *info, void *data);
private:
	static const Persistent<String> syb_err_wrong_arguments;
	static const Persistent<String> syb_err_not_a_constructor;
	static const Persistent<String> syb_evt_found;
	static const Persistent<String> syb_evt_succeeded;
	static const Persistent<String> syb_evt_failed;
	static const Persistent<String> syb_evt_interrupted;
	static const struct jsCallbackData {
		Handle<Object> self;
		Handle<Function> func;
	};
	static const struct workdata {
		uv_work_t req;
		Persistent<Object> self;
		Persistent<Function> func;
		void *data;
		//the following data only used in progressive mode
		HANDLE hnd;
		size_t count;
		bool stop;
	};
public:
	static resultData *basic(const wchar_t *path) {//you have to delete every linked data yourself if it is not NULL
		resultData *result = new resultData;
		HANDLE hnd = FindFirstFileExW(path, FindExInfoStandard, &result->data, FindExSearchNameMatch, NULL, NULL);
		if (hnd == INVALID_HANDLE_VALUE) {
			delete result;
			result = NULL;
		} else {
			resultData *resultnew, *resultold;
			if (isValidInfo(&result->data)) {
				resultnew = new resultData;
				resultold = result;
			} else {
				resultnew = result;
				resultold = NULL;
				result = NULL;
			}
			while (FindNextFileW(hnd, &resultnew->data)) {
				if (isValidInfo(&resultnew->data)) {
					if (resultold) {
						resultold->next = resultnew;
					} else {
						result = resultnew;
					}
					resultold = resultnew;
					resultnew = new resultData;
				}
			}
			resultold->next = NULL;
			FindClose(hnd);
			if (resultnew != result) {
				delete resultnew;
			}
		}
		return result;
	}
	static DWORD basicWithCallback(const wchar_t *path, const findResultCall callback, void *data) {//data could be anything that will directly pass to the callback
		WIN32_FIND_DATAW info;
		HANDLE hnd = FindFirstFileExW(path, FindExInfoStandard, &info, FindExSearchNameMatch, NULL, NULL);
		DWORD result = 0;
		bool stop = false;
		if (hnd != INVALID_HANDLE_VALUE) {
			if (isValidInfo(&info)) {
				stop = callback(&info, data);
				result++;
			}
			while (!stop&&FindNextFileW(hnd, &info)) {
				if (isValidInfo(&info)) {
					stop = callback(&info, data);
					result++;
				}
			}
			FindClose(hnd);
		}
		return result;
	}
	static Handle<Object> fileInfoToJs(const WIN32_FIND_DATAW *info) {//this function does not check whether info is NULL, make sure it is not before calling
		HandleScope scope;
		Handle<Object> o = Object::New();
		o->Set(syb_returns_longName, String::New((uint16_t*)info->cFileName));
		o->Set(syb_returns_shortName, String::New((uint16_t*)info->cAlternateFileName));
		o->Set(syb_returns_creationTime, Date::New(fileTimeToJsDateVal(&info->ftCreationTime)));
		o->Set(syb_returns_lastAccessTime, Date::New(fileTimeToJsDateVal(&info->ftLastAccessTime)));
		o->Set(syb_returns_lastWriteTime, Date::New(fileTimeToJsDateVal(&info->ftLastWriteTime)));
		o->Set(syb_returns_size, Number::New((double)combineHiLow(info->nFileSizeHigh, info->nFileSizeLow)));
		if (info->dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT) {
			if (info->dwReserved0 == IO_REPARSE_TAG_CSV) {
				o->Set(syb_returns_reparsePointTag, syb_reparsePoint_csv);
			} else if (info->dwReserved0 == IO_REPARSE_TAG_DEDUP) {
				o->Set(syb_returns_reparsePointTag, syb_reparsePoint_dedup);
			} else if (info->dwReserved0 == IO_REPARSE_TAG_DFS) {
				o->Set(syb_returns_reparsePointTag, syb_reparsePoint_dfs);
			} else if (info->dwReserved0 == IO_REPARSE_TAG_DFSR) {
				o->Set(syb_returns_reparsePointTag, syb_reparsePoint_dfsr);
			} else if (info->dwReserved0 == IO_REPARSE_TAG_HSM) {
				o->Set(syb_returns_reparsePointTag, syb_reparsePoint_hsm);
			} else if (info->dwReserved0 == IO_REPARSE_TAG_HSM2) {
				o->Set(syb_returns_reparsePointTag, syb_reparsePoint_hsm2);
			} else if (info->dwReserved0 == IO_REPARSE_TAG_MOUNT_POINT) {
				o->Set(syb_returns_reparsePointTag, syb_reparsePoint_mountPoint);
			} else if (info->dwReserved0 == IO_REPARSE_TAG_NFS) {
				o->Set(syb_returns_reparsePointTag, syb_reparsePoint_nfs);
			} else if (info->dwReserved0 == IO_REPARSE_TAG_FILE_PLACEHOLDER) {
				o->Set(syb_returns_reparsePointTag, syb_reparsePoint_placeHolder);
			} else if (info->dwReserved0 == IO_REPARSE_TAG_SIS) {
				o->Set(syb_returns_reparsePointTag, syb_reparsePoint_sis);
			} else if (info->dwReserved0 == IO_REPARSE_TAG_SYMLINK) {
				o->Set(syb_returns_reparsePointTag, syb_reparsePoint_symlink);
			} else if (info->dwReserved0 == IO_REPARSE_TAG_WIM) {
				o->Set(syb_returns_reparsePointTag, syb_reparsePoint_wim);
			} else {
				o->Set(syb_returns_reparsePointTag, syb_reparsePoint_unknown);
			}
		} else {
			o->Set(syb_returns_reparsePointTag, String::NewSymbol(""));
		}
		o->Set(syb_returns_isArchived, info->dwFileAttributes&FILE_ATTRIBUTE_ARCHIVE ? True() : False());
		o->Set(syb_returns_isCompressed, info->dwFileAttributes&FILE_ATTRIBUTE_COMPRESSED ? True() : False());
		o->Set(syb_returns_isDirectory, info->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY ? True() : False());
		o->Set(syb_returns_isEncrypted, info->dwFileAttributes&FILE_ATTRIBUTE_ENCRYPTED ? True() : False());
		o->Set(syb_returns_isHidden, info->dwFileAttributes&FILE_ATTRIBUTE_HIDDEN ? True() : False());
		o->Set(syb_returns_isNotContentIndexed, info->dwFileAttributes&FILE_ATTRIBUTE_NOT_CONTENT_INDEXED ? True() : False());
		o->Set(syb_returns_isOffline, info->dwFileAttributes&FILE_ATTRIBUTE_OFFLINE ? True() : False());
		o->Set(syb_returns_isReadOnly, info->dwFileAttributes&FILE_ATTRIBUTE_READONLY ? True() : False());
		o->Set(syb_returns_isSparseFile, info->dwFileAttributes&FILE_ATTRIBUTE_SPARSE_FILE ? True() : False());
		o->Set(syb_returns_isSystem, info->dwFileAttributes&FILE_ATTRIBUTE_SYSTEM ? True() : False());
		o->Set(syb_returns_isTemporary, info->dwFileAttributes&FILE_ATTRIBUTE_TEMPORARY ? True() : False());
		return scope.Close(o);
	}
	static Handle<Array> basicToJs(resultData *data) {
		HandleScope scope;
		Handle<Array> a = Array::New();
		while (data) {
			a->Set(a->Length(), fileInfoToJs(&data->data));
			resultData *old = data;
			data = old->next;
			delete old;
		}
		return scope.Close(a);
	}
	static Handle<Function> functionRegister(bool isAsyncVersion) {
		HandleScope scope;
		Handle<FunctionTemplate> t = FunctionTemplate::New(isAsyncVersion ? jsAsync : jsSync);

		//set error messages
		Handle<Object> errors = Object::New();
		errors->Set(syb_err_wrong_arguments, syb_err_wrong_arguments, global_syb_attr_const);
		errors->Set(syb_err_not_a_constructor, syb_err_not_a_constructor, global_syb_attr_const);
		t->Set(String::NewSymbol("errors"), errors, global_syb_attr_const);

		//set events
		if (isAsyncVersion) {
			Handle<Object> events = Object::New();
			events->Set(syb_evt_found, syb_evt_found, global_syb_attr_const);
			events->Set(syb_evt_succeeded, syb_evt_succeeded, global_syb_attr_const);
			events->Set(syb_evt_failed, syb_evt_failed, global_syb_attr_const);
			events->Set(syb_evt_interrupted, syb_evt_interrupted, global_syb_attr_const);
			t->Set(String::NewSymbol("events"), events, global_syb_attr_const);
		}

		//set properties of return value
		Handle<Object> returns = Object::New();
		returns->Set(syb_returns_longName, syb_returns_longName, global_syb_attr_const);
		returns->Set(syb_returns_shortName, syb_returns_shortName, global_syb_attr_const);
		returns->Set(syb_returns_creationTime, syb_returns_creationTime, global_syb_attr_const);
		returns->Set(syb_returns_lastAccessTime, syb_returns_lastAccessTime, global_syb_attr_const);
		returns->Set(syb_returns_lastWriteTime, syb_returns_lastWriteTime, global_syb_attr_const);
		returns->Set(syb_returns_size, syb_returns_size, global_syb_attr_const);
		returns->Set(syb_returns_isArchived, syb_returns_isArchived, global_syb_attr_const);
		returns->Set(syb_returns_isCompressed, syb_returns_isCompressed, global_syb_attr_const);
		returns->Set(syb_returns_isDirectory, syb_returns_isDirectory, global_syb_attr_const);
		returns->Set(syb_returns_isEncrypted, syb_returns_isEncrypted, global_syb_attr_const);
		returns->Set(syb_returns_isHidden, syb_returns_isHidden, global_syb_attr_const);
		returns->Set(syb_returns_isNotContentIndexed, syb_returns_isNotContentIndexed, global_syb_attr_const);
		returns->Set(syb_returns_isOffline, syb_returns_isOffline, global_syb_attr_const);
		returns->Set(syb_returns_isReadOnly, syb_returns_isReadOnly, global_syb_attr_const);
		returns->Set(syb_returns_isSparseFile, syb_returns_isSparseFile, global_syb_attr_const);
		returns->Set(syb_returns_isSystem, syb_returns_isSystem, global_syb_attr_const);
		returns->Set(syb_returns_isTemporary, syb_returns_isTemporary, global_syb_attr_const);
		returns->Set(syb_returns_reparsePointTag, syb_returns_reparsePointTag, global_syb_attr_const);
		t->Set(String::NewSymbol("returns"), returns, global_syb_attr_const);

		return scope.Close(t->GetFunction());
	}
private:
	static bool isValidInfo(const WIN32_FIND_DATAW *info) {//determine whether it is the real content 
		return wcscmp(info->cFileName, L".") != 0 && wcscmp(info->cFileName, L"..") != 0;
	}
	static Handle<Value> jsSync(const Arguments& args) {
		HandleScope scope;
		Handle<Value> result;
		if (args.IsConstructCall()) {
			result = ThrowException(Exception::Error(syb_err_not_a_constructor));
		} else {
			if (args.Length() > 0 && (args[0]->IsString() || args[0]->IsStringObject())) {
				String::Value spath(args[0]);
				if (args.Length() > 1 && args[1]->IsFunction()) {
					jsCallbackData callbackdata = {args.This(), Local<Function>::Cast(args[1])};
					result = Integer::New(basicWithCallback((wchar_t*)*spath, jsSyncCallback, &callbackdata));
				} else {
					result = basicToJs(basic((wchar_t*)*spath));
				}
			} else {
				result = ThrowException(Exception::Error(syb_err_wrong_arguments));
			}
		}
		return scope.Close(result);
	}
	static bool jsSyncCallback(const WIN32_FIND_DATAW *info, void *data) {
		HandleScope scope;
		Handle<Value> o = fileInfoToJs(info);
		jsCallbackData *d = (jsCallbackData*)data;
		return d->func->Call(d->self, 1, &o)->ToBoolean()->IsTrue();
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
				String::Value spath(args[0]);
				data->data = _wcsdup((wchar_t*)*spath);
				if (args.Length() > 2 && args[2]->ToBoolean()->IsTrue()) {
					data->hnd = INVALID_HANDLE_VALUE;
					data->count = 0;
					data->stop = false;
				} else {
					data->hnd = NULL;
				}
				if (uv_queue_work(uv_default_loop(), &data->req, beginWork, afterWork) == 0) {
					result = True();
				} else {
					free(data->data);
					data->self.Dispose();
					data->func.Dispose();
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
		if (data->hnd) {
			WIN32_FIND_DATAW *info = new WIN32_FIND_DATAW;
			if (data->hnd == INVALID_HANDLE_VALUE) {
				data->hnd = FindFirstFileExW((wchar_t*)data->data, FindExInfoStandard, info, FindExSearchNameMatch, NULL, NULL);
				free(data->data);
				if (data->hnd != INVALID_HANDLE_VALUE) {
					while (!isValidInfo(info)) {
						if (!FindNextFileW(data->hnd, info)) {
							FindClose(data->hnd);
							data->hnd = INVALID_HANDLE_VALUE;
							break;
						}
					}
				}
			} else {
				if (!data->stop) {
					if (!FindNextFileW(data->hnd, info)) {
						FindClose(data->hnd);
						data->hnd = INVALID_HANDLE_VALUE;
					} else {
						while (!isValidInfo(info)) {
							if (!FindNextFileW(data->hnd, info)) {
								FindClose(data->hnd);
								data->hnd = INVALID_HANDLE_VALUE;
								break;
							}
						}
					}
				} else {
					FindClose(data->hnd);
					data->hnd = INVALID_HANDLE_VALUE;
				}
			}
			if (data->hnd == INVALID_HANDLE_VALUE) {
				delete info;
			} else {
				data->data = info;
			}
		} else {
			resultData *rdata = basic((wchar_t*)data->data);
			free(data->data);
			data->data = rdata;
		}
	}
	static void afterWork(uv_work_t *req, int status) {
		HandleScope scope;
		workdata *data = (workdata*)req->data;
		int del;
		if (data->hnd) {
			Handle<Value> result[2];
			if (data->hnd == INVALID_HANDLE_VALUE) {
				result[0] = syb_evt_succeeded;
				result[1] = Number::New((double)data->count);
				del = 1;
			} else {
				WIN32_FIND_DATAW *info = (WIN32_FIND_DATAW*)data->data;
				if (data->stop) {
					result[0] = data->stop ? syb_evt_interrupted : syb_evt_succeeded;
					result[1] = Number::New((double)data->count);
					del = 1;
				} else {
					data->count++;
					result[0] = syb_evt_found;
					result[1] = fileInfoToJs(info);
					del = uv_queue_work(uv_default_loop(), &data->req, beginWork, afterWork);
				}
				delete info;
			}
			data->stop = data->func->Call(data->self, 2, result)->ToBoolean()->IsTrue();
		} else {
			Handle<Value> result;
			result = basicToJs((resultData*)data->data);
			del = 1;
			data->func->Call(data->self, 1, &result);
		}
		if (del) {
			if (del != 1) {
				Handle<Value> result[2];
				result[0] = syb_evt_failed;
				result[1] = Number::New((double)data->count);
				data->func->Call(data->self, 2, result);
			}
			data->func.Dispose();
			data->self.Dispose();
			delete data;
		}
	}
};
const Persistent<String> find::syb_err_wrong_arguments = global_syb_err_wrong_arguments;
const Persistent<String> find::syb_err_not_a_constructor = global_syb_err_not_a_constructor;
const Persistent<String> find::syb_evt_found = NODE_PSYMBOL("FOUND");
const Persistent<String> find::syb_evt_succeeded = NODE_PSYMBOL("SUCCEEDED");
const Persistent<String> find::syb_evt_failed = NODE_PSYMBOL("FAILED");
const Persistent<String> find::syb_evt_interrupted = NODE_PSYMBOL("INTERRUPTED");
const Persistent<String> find::syb_returns_longName = NODE_PSYMBOL("LONG_NAME");
const Persistent<String> find::syb_returns_shortName = NODE_PSYMBOL("SHORT_NAME");
const Persistent<String> find::syb_returns_creationTime = NODE_PSYMBOL("CREATION_TIME");
const Persistent<String> find::syb_returns_lastAccessTime = NODE_PSYMBOL("LAST_ACCESS_TIME");
const Persistent<String> find::syb_returns_lastWriteTime = NODE_PSYMBOL("LAST_WRITE_TIME");
const Persistent<String> find::syb_returns_size = NODE_PSYMBOL("SIZE");
const Persistent<String> find::syb_returns_reparsePointTag = NODE_PSYMBOL("REPARSE_POINT_TAG");
const Persistent<String> find::syb_returns_isDirectory = NODE_PSYMBOL("IS_DIRECTORY");
const Persistent<String> find::syb_returns_isCompressed = NODE_PSYMBOL("IS_COMPRESSED");
const Persistent<String> find::syb_returns_isEncrypted = NODE_PSYMBOL("IS_ENCRYPTED");
const Persistent<String> find::syb_returns_isSparseFile = NODE_PSYMBOL("IS_SPARSE_FILE");
const Persistent<String> find::syb_returns_isArchived = global_syb_fileAttr_isArchived;
const Persistent<String> find::syb_returns_isHidden = global_syb_fileAttr_isHidden;
const Persistent<String> find::syb_returns_isNotContentIndexed = global_syb_fileAttr_isNotContentIndexed;
const Persistent<String> find::syb_returns_isOffline = global_syb_fileAttr_isOffline;
const Persistent<String> find::syb_returns_isReadOnly = global_syb_fileAttr_isReadOnly;
const Persistent<String> find::syb_returns_isSystem = global_syb_fileAttr_isSystem;
const Persistent<String> find::syb_returns_isTemporary = global_syb_fileAttr_isTemporary;
const Persistent<String> find::syb_reparsePoint_unknown = NODE_PSYMBOL("UNKNOWN");
const Persistent<String> find::syb_reparsePoint_csv = NODE_PSYMBOL("CSV");
const Persistent<String> find::syb_reparsePoint_dedup = NODE_PSYMBOL("DEDUP");
const Persistent<String> find::syb_reparsePoint_dfs = NODE_PSYMBOL("DFS");
const Persistent<String> find::syb_reparsePoint_dfsr = NODE_PSYMBOL("DFSR");
const Persistent<String> find::syb_reparsePoint_hsm = NODE_PSYMBOL("HSM");
const Persistent<String> find::syb_reparsePoint_hsm2 = NODE_PSYMBOL("HSM2");
const Persistent<String> find::syb_reparsePoint_mountPoint = NODE_PSYMBOL("MOUNT_POINT");
const Persistent<String> find::syb_reparsePoint_nfs = NODE_PSYMBOL("NFS");
const Persistent<String> find::syb_reparsePoint_placeHolder = NODE_PSYMBOL("PLACE_HOLDER");
const Persistent<String> find::syb_reparsePoint_sis = NODE_PSYMBOL("SIS");
const Persistent<String> find::syb_reparsePoint_symlink = NODE_PSYMBOL("SYMLINK");
const Persistent<String> find::syb_reparsePoint_wim = NODE_PSYMBOL("WIM");