#pragma once
#include "find.h"
#include "splitPath.h"

#define SYB_OPT_SUBDIRS "WATCH_SUB_DIRECTORIES"
#define SYB_OPT_FILESIZE "CHANGE_FILE_SIZE"
#define SYB_OPT_LASTWRITE "CHANGE_LAST_WRITE"
#define SYB_OPT_LASTACCESS "CHANGE_LAST_ACCESS"
#define SYB_OPT_CREATION "CHANGE_CREATION"
#define SYB_OPT_ATTRIBUTES "CHANGE_ATTRIBUTES"
#define SYB_OPT_SECURITY "CHANGE_SECUTITY"
#define SYB_EVT_STA "STARTED"
#define SYB_EVT_NEW "ADDED"
#define SYB_EVT_DEL "REMOVED"
#define SYB_EVT_CHG "MODIFIED"
#define SYB_EVT_REN "RENAMED"
#define SYB_EVT_MOV "MOVED"
#define SYB_EVT_REN_OLDNAME "OLD_NAME"
#define SYB_EVT_REN_NEWNAME "NEW_NAME"
#define SYB_ERR_UNABLE_TO_WATCH_SELF "UNABLE_TO_WATCH_SELF"
#define SYB_ERR_UNABLE_TO_CONTINUE_WATCHING "UNABLE_TO_CONTINUE_WATCHING"

#define SYB_BUFFERSIZE 64 * 1024

//dirWatcher requires vista or latter to call GetFinalPathNameByHandleW.
//the API is necessary since the dir we are watching could also be moved to another path.
//and it is the only way to get the new path at that kind of situation.
//however, if you still need to use dirWatcher in winxp, it will work without watching
//the parent dir. and always fire an error at start up.
class dirWatcher:ObjectWrap {
private:
	HANDLE pathhnd;
	HANDLE parenthnd;
	uv_async_t uvpathhnd;
	uv_async_t uvparenthnd;
	BOOL watchingParent;
	BOOL watchingPath;
	bool subDirs;
	DWORD options;
	wchar_t *oldName;
	wchar_t *newName;
	wchar_t *shortName;
	wchar_t *longName;
	Persistent<Function> callback;
	void *pathbuffer;
	BYTE parentbuffer[SYB_BUFFERSIZE];
public:
	dirWatcher(Handle<Object> handle, wchar_t *spath, Handle<Function> cb, bool watchSubDirs, DWORD opts):ObjectWrap() {
		ISOLATE_NEW;
		SCOPE;
		Wrap(handle);
		Ref();
		bool mute = false;
		watchingParent = watchingPath = FALSE;
		uvpathhnd = uvparenthnd = {0};
		wchar_t *realPath = oldName = newName = longName = shortName = NULL;
		parenthnd = INVALID_HANDLE_VALUE;
		PERSISTENT_NEW(callback, cb, Function);
		pathhnd = CreateFileW(spath, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
		if (pathhnd != INVALID_HANDLE_VALUE) {
			uv_loop_t *loop = uv_default_loop();
			if (CreateIoCompletionPort(pathhnd, loop->iocp, (ULONG_PTR)pathhnd, 0)) {
				subDirs = watchSubDirs;
				options = opts;
				uv_async_init(loop, &uvpathhnd, finishWatchingPath);
				//uvpathhnd.async_req.type = UV_WAKEUP;
				//std::wcout << (uvpathhnd.async_req.data) << std::endl;
				uvpathhnd.data = this;
				beginWatchingPath(this);
				if (watchingPath) {
					uv_async_init(loop, &uvparenthnd, finishWatchingParent);
					uvparenthnd.data = this;
					realPath = getCurrentPathByHandle(pathhnd);
					if (realPath) {
						mute = watchParent(this, realPath);
						if (parenthnd != INVALID_HANDLE_VALUE) {
							beginWatchingParent(this);
						}
					}
				}
			}
		}
		if (watchingPath) {
			if (realPath) {
				callJs(this, SYB_EVT_STA, NEWSTRING_TWOBYTES(realPath));
				free(realPath);
			} else {
				callJs(this, SYB_EVT_STA, NEWSTRING_TWOBYTES(spath));
			}
			if (!mute && !watchingParent) {
				callJs(this, SYB_EVT_ERR, NEWSTRING(SYB_ERR_UNABLE_TO_WATCH_SELF));
			}
		} else {
			stopWatching(this, true);
			callJs(this, SYB_EVT_ERR, NEWSTRING(SYB_ERR_INITIALIZATION_FAILED));
		}
	}
	virtual ~dirWatcher() {
		PERSISTENT_RELEASE(callback);
		Unref();
	}
	static Handle<Function> functionRegister() {
		ISOLATE_NEW;
		SCOPE_ESCAPABLE;
		RETURNTYPE<String> tmp;
		RETURNTYPE<FunctionTemplate> ft = FunctionTemplate::New(ISOLATE_C New);
		ft->InstanceTemplate()->SetInternalFieldCount(1);
		RETURNTYPE<Function> t = ft->GetFunction();
		//set methods
		SETWITHATTR(t->Get(NEWSTRING(SYB_PROTOTYPE))->ToObject(), NEWSTRING("close"), NEWFUNCTION(close), SYB_ATTR_CONST);

		//set error messages
		RETURNTYPE<Object> errmsgs = Object::New(ISOLATE);
		tmp = NEWSTRING(SYB_ERR_UNABLE_TO_WATCH_SELF);
		SETWITHATTR(errmsgs, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_ERR_UNABLE_TO_CONTINUE_WATCHING);
		SETWITHATTR(errmsgs, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_ERR_INITIALIZATION_FAILED);
		SETWITHATTR(errmsgs, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_ERR_WRONG_ARGUMENTS);
		SETWITHATTR(errmsgs, tmp, tmp, SYB_ATTR_CONST);
		SETWITHATTR(t, NEWSTRING(SYB_ERRORS), errmsgs, SYB_ATTR_CONST);

		//set events
		RETURNTYPE<Object> evts = Object::New(ISOLATE);
		tmp = NEWSTRING(SYB_EVT_STA);
		SETWITHATTR(evts, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_EVT_END);
		SETWITHATTR(evts, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_EVT_NEW);
		SETWITHATTR(evts, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_EVT_DEL);
		SETWITHATTR(evts, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_EVT_REN);
		SETWITHATTR(evts, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_EVT_CHG);
		SETWITHATTR(evts, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_EVT_CHG);
		SETWITHATTR(evts, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_EVT_ERR);
		SETWITHATTR(evts, tmp, tmp, SYB_ATTR_CONST);
		SETWITHATTR(t, NEWSTRING(SYB_EVENTS), evts, SYB_ATTR_CONST);

		//set options
		RETURNTYPE<Object> opts = Object::New(ISOLATE);
		tmp = NEWSTRING(SYB_OPT_SUBDIRS);
		SETWITHATTR(opts, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_OPT_FILESIZE);
		SETWITHATTR(opts, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_OPT_LASTWRITE);
		SETWITHATTR(opts, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_OPT_LASTACCESS);
		SETWITHATTR(opts, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_OPT_CREATION);
		SETWITHATTR(opts, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_OPT_ATTRIBUTES);
		SETWITHATTR(opts, tmp, tmp, SYB_ATTR_CONST);
		tmp = NEWSTRING(SYB_OPT_SECURITY);
		SETWITHATTR(opts, tmp, tmp, SYB_ATTR_CONST);
		SETWITHATTR(t, NEWSTRING(SYB_OPTIONS), evts, SYB_ATTR_CONST);

		RETURN_SCOPE(t);
	}
private:
	static JSFUNC(New) {
		ISOLATE_NEW_ARGS;
		SCOPE;
		RETURNTYPE<Value> r;
		if (args.Length() > 1 && args[0]->IsString() || args[0]->IsStringObject() && args[1]->IsFunction()) {
			if (args.IsConstructCall()) {
				DWORD options = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE;
				bool subDirs = true;
				if (args.Length() > 2 && args[2]->IsObject()) {
					RETURNTYPE<Object> iopt = Handle<Object>::Cast(args[2]);
					RETURNTYPE<String> tmp = NEWSTRING(SYB_OPT_SUBDIRS);
					if (iopt->HasOwnProperty(tmp) && iopt->Get(tmp)->ToBoolean()->IsFalse()) {
						subDirs = false;
					}
					tmp = NEWSTRING(SYB_OPT_FILESIZE);
					if (iopt->HasOwnProperty(tmp) && iopt->Get(tmp)->ToBoolean()->IsFalse()) {
						options ^= FILE_NOTIFY_CHANGE_SIZE;
					}
					tmp = NEWSTRING(SYB_OPT_LASTWRITE);
					if (iopt->HasOwnProperty(tmp) && iopt->Get(tmp)->ToBoolean()->IsFalse()) {
						options ^= FILE_NOTIFY_CHANGE_LAST_WRITE;
					}
					if (iopt->Get(NEWSTRING(SYB_OPT_LASTACCESS))->ToBoolean()->IsTrue()) {
						options |= FILE_NOTIFY_CHANGE_LAST_ACCESS;
					}
					if (iopt->Get(NEWSTRING(SYB_OPT_CREATION))->ToBoolean()->IsTrue()) {
						options |= FILE_NOTIFY_CHANGE_CREATION;
					}
					if (iopt->Get(NEWSTRING(SYB_OPT_ATTRIBUTES))->ToBoolean()->IsTrue()) {
						options |= FILE_NOTIFY_CHANGE_ATTRIBUTES;
					}
					if (iopt->Get(NEWSTRING(SYB_OPT_SECURITY))->ToBoolean()->IsTrue()) {
						options |= FILE_NOTIFY_CHANGE_SECURITY;
					}
				}
				String::Value s(args[0]);
				new dirWatcher(args.This(), (wchar_t*)*s, RETURNTYPE<Function>::Cast(args[1]), subDirs, options);
				r = args.This();
			} else {
				if (args.Length() > 2) {
					RETURNTYPE<Value> v[3] = {args[0], args[1], args[2]};
					r = args.Callee()->CallAsConstructor(3, v);
				} else {
					RETURNTYPE<Value> v[2] = {args[0], args[1]};
					r = args.Callee()->CallAsConstructor(2, v);
				}
			}
		} else {
			r = THROWEXCEPTION(SYB_ERR_WRONG_ARGUMENTS);
		}
		RETURN(r);
	}
	static JSFUNC(close) {
		ISOLATE_NEW;
		SCOPE;
		RETURNTYPE<Value> result;
		dirWatcher *self = Unwrap<dirWatcher>(args.This());
		if (self->pathhnd == INVALID_HANDLE_VALUE) {
			result = False(ISOLATE);//this method returns false if dirWatcher is failed to create or already closed
		} else {
			stopWatching(self);
			result = True(ISOLATE);
		}
		RETURN(result);
	}
	static void savePath(dirWatcher *self, wchar_t *realPath) {
		if (self->shortName) {
			free(self->shortName);
			self->shortName = NULL;
		}
		if (self->longName) {
			free(self->longName);
			self->longName = NULL;
		}
		find::resultData *fr = find::basic(realPath);
		if (fr) {
			self->longName = _wcsdup(fr->data.cFileName);
			if (wcslen(fr->data.cAlternateFileName) > 0 && wcscmp(fr->data.cFileName, fr->data.cAlternateFileName) != 0) {
				self->shortName = _wcsdup(fr->data.cAlternateFileName);
			}
			delete fr;
		}
	}
	static bool watchParent(dirWatcher *self, wchar_t *realPath) {//return true means no need to fire UNABLE_TO_WATCH_SELF
		bool result = false;
		if (self->parenthnd != INVALID_HANDLE_VALUE) {
			CloseHandle(self->parenthnd);
			self->parenthnd = INVALID_HANDLE_VALUE;
		}
		splitPath::splitedPath *sp = splitPath::basic(realPath);
		if (sp) {
			if (sp->parentLen == 0) {
				result = true;
			} else {
				savePath(self, realPath);
				if (self->longName) {
					wchar_t *parent = new wchar_t[sp->parentLen + 1];
					wcsncpy_s(parent, sp->parentLen + 1, realPath, sp->parentLen);
					delete sp;
					self->parenthnd = CreateFileW(parent, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
					delete parent;
					if (self->parenthnd != INVALID_HANDLE_VALUE && !CreateIoCompletionPort(self->parenthnd, uv_default_loop()->iocp, (ULONG_PTR)self->parenthnd, 0)) {
						CloseHandle(self->parenthnd);
						self->parenthnd = INVALID_HANDLE_VALUE;
					}
				}
			}
		}
		return result;
	}
	static void beginWatchingParent(dirWatcher *self) {
		self->watchingParent = ReadDirectoryChangesW(self->parenthnd, self->parentbuffer, SYB_BUFFERSIZE, FALSE, FILE_NOTIFY_CHANGE_DIR_NAME, NULL, &self->uvparenthnd.async_req.THEASYNCOVERLAP, NULL);
	}
	static void beginWatchingPath(dirWatcher *self) {
		self->pathbuffer = malloc(SYB_BUFFERSIZE);
		if (self->pathbuffer) {
			self->watchingPath = ReadDirectoryChangesW(self->pathhnd, self->pathbuffer, SYB_BUFFERSIZE, self->subDirs, self->options, NULL, &self->uvpathhnd.async_req.THEASYNCOVERLAP, NULL);
			if (!self->watchingPath) {
				free(self->pathbuffer);
			}
		}
	}
	static ASYNCCB(finishWatchingParent) {
		dirWatcher *self = (dirWatcher*)hnd->data;
		self->watchingParent = FALSE;
		if (self->parenthnd == INVALID_HANDLE_VALUE) {
			uv_close((uv_handle_t*)hnd, NULL);
			checkWatchingStoped(self);
		} else {
			if (hnd->async_req.THEASYNCOVERLAP.Internal == ERROR_SUCCESS) {
				wchar_t *newpath = NULL;
				FILE_NOTIFY_INFORMATION *pInfo;
				DWORD d = 0;
				do {
					pInfo = (FILE_NOTIFY_INFORMATION*)((ULONG_PTR)self->parentbuffer + d);
					if ((pInfo->Action == FILE_ACTION_RENAMED_OLD_NAME || pInfo->Action == FILE_ACTION_REMOVED) && wcsncmp(self->longName, pInfo->FileName, MAX(pInfo->FileNameLength / sizeof(wchar_t), wcslen(self->longName))) == 0 || (self->shortName && wcsncmp(self->shortName, pInfo->FileName, MAX(pInfo->FileNameLength / sizeof(wchar_t), wcslen(self->shortName))) == 0)) {
						newpath = getCurrentPathByHandle(self->pathhnd);
						if (newpath) {
							if (pInfo->Action == FILE_ACTION_RENAMED_OLD_NAME) {
								savePath(self, newpath);
								if (!self->longName) {
									CloseHandle(self->parenthnd);
									self->parenthnd = INVALID_HANDLE_VALUE;
								}
							} else {
								watchParent(self, newpath);
							}
						} else {
							CloseHandle(self->parenthnd);
							self->parenthnd = INVALID_HANDLE_VALUE;
						}
						break;
					}
					d += pInfo->NextEntryOffset;
				} while (pInfo->NextEntryOffset > 0);
				if (self->parenthnd != INVALID_HANDLE_VALUE) {
					beginWatchingParent(self);
				}
				if (newpath) {
					ISOLATE_NEW;
					SCOPE;
					callJs(self, SYB_EVT_MOV, NEWSTRING_TWOBYTES(newpath));
					free(newpath);
				}
			}
			if (!self->watchingParent) {
				stopWatchingParent(self);
				ISOLATE_NEW;
				SCOPE;
				callJs(self, SYB_EVT_ERR, NEWSTRING(SYB_ERR_UNABLE_TO_WATCH_SELF));
			}
		}
	}
	static ASYNCCB(finishWatchingPath) {
		dirWatcher *self = (dirWatcher*)hnd->data;
		self->watchingPath = FALSE;
		if (self->pathhnd == INVALID_HANDLE_VALUE) {
			uv_close((uv_handle_t*)hnd, NULL);
			free(self->pathbuffer);
			checkWatchingStoped(self);
		} else {
			ISOLATE_NEW;
			SCOPE;
			bool e = false;
			if (hnd->async_req.THEASYNCOVERLAP.Internal == ERROR_SUCCESS) {
				FILE_NOTIFY_INFORMATION *pInfo;
				void *buffer = self->pathbuffer;
				beginWatchingPath(self);
				DWORD d = 0;
				do {
					pInfo = (FILE_NOTIFY_INFORMATION*)((ULONG_PTR)buffer + d);
					RETURNTYPE<String> filename = NEWSTRING_TWOBYTES_LEN(pInfo->FileName, pInfo->FileNameLength / sizeof(wchar_t));
					if (pInfo->Action == FILE_ACTION_ADDED) {
						callJs(self, SYB_EVT_NEW, filename);
					} else if (pInfo->Action == FILE_ACTION_REMOVED) {
						callJs(self, SYB_EVT_DEL, filename);
					} else if (pInfo->Action == FILE_ACTION_MODIFIED) {
						callJs(self, SYB_EVT_CHG, filename);
					} else {
						if (pInfo->Action == FILE_ACTION_RENAMED_OLD_NAME) {
							if (self->newName) {
								RETURNTYPE<Object> arg = Object::New(ISOLATE);
								arg->Set(NEWSTRING(SYB_EVT_REN_OLDNAME), filename);
								arg->Set(NEWSTRING(SYB_EVT_REN_NEWNAME), NEWSTRING_TWOBYTES(self->newName));
								delete self->newName;
								self->newName = NULL;
								callJs(self, SYB_EVT_REN, arg);
							} else {
								size_t sz = pInfo->FileNameLength + 1;
								self->oldName = new wchar_t[sz];
								wcscpy_s(self->oldName, sz, pInfo->FileName);
							}
						} else if (pInfo->Action == FILE_ACTION_RENAMED_NEW_NAME) {
							if (self->oldName) {
								RETURNTYPE<Object> arg = Object::New(ISOLATE);
								arg->Set(NEWSTRING(SYB_EVT_REN_OLDNAME), filename);
								arg->Set(NEWSTRING(SYB_EVT_REN_NEWNAME), NEWSTRING_TWOBYTES(self->oldName));
								delete self->oldName;
								self->oldName = NULL;
								callJs(self, SYB_EVT_REN, arg);
							} else {
								size_t sz = pInfo->FileNameLength + 1;
								self->newName = new wchar_t[sz];
								wcscpy_s(self->newName, sz, pInfo->FileName);
							}
						}
					}
					d += pInfo->NextEntryOffset;
				} while (pInfo->NextEntryOffset > 0);
				free(buffer);
			}
			if (!self->watchingPath) {
				stopWatching(self);
				callJs(self, SYB_EVT_ERR, NEWSTRING(SYB_ERR_UNABLE_TO_CONTINUE_WATCHING));
			}
		}
	}
	static void stopWatching(dirWatcher *self, bool mute = false) {
		if (self->pathhnd != INVALID_HANDLE_VALUE) {
			CloseHandle(self->pathhnd);
			self->pathhnd = INVALID_HANDLE_VALUE;
		}
		if (self->newName) {
			delete self->newName;
			self->newName = NULL;
		}
		if (self->oldName) {
			delete self->oldName;
			self->oldName = NULL;
		}
		if (!self->watchingPath && uv_is_active((uv_handle_t*)&self->uvpathhnd)) {
			uv_close((uv_handle_t*)&self->uvpathhnd, NULL);
		}
		stopWatchingParent(self);
		if (!mute) {
			checkWatchingStoped(self);
		}
	}
	static void stopWatchingParent(dirWatcher *self) {
		if (self->parenthnd != INVALID_HANDLE_VALUE) {
			CloseHandle(self->parenthnd);
			self->parenthnd = INVALID_HANDLE_VALUE;
		}
		if (self->longName) {
			free(self->longName);
			self->longName = NULL;
		}
		if (self->shortName) {
			free(self->shortName);
			self->longName = NULL;
		}
		if (!self->watchingParent && uv_is_active((uv_handle_t*)&self->uvparenthnd)) {
			uv_close((uv_handle_t*)&self->uvparenthnd, NULL);
		}
	}
	static void checkWatchingStoped(dirWatcher *self) {
		if (!uv_is_active((uv_handle_t*)&self->uvpathhnd) && !uv_is_active((uv_handle_t*)&self->uvparenthnd)) {
			ISOLATE_NEW;
			SCOPE;
			callJs(self, SYB_EVT_END, Undefined(ISOLATE));
		}
	}
	static void callJs(dirWatcher *self, char *evt_type, Handle<Value> src) {
		ISOLATE_NEW;
		SCOPE;
		RETURNTYPE<Value> arg[2] = {NEWSTRING(evt_type), src};
		PERSISTENT_CONV(self->callback, Function)->Call(PERSISTENT_CONV(self->OBJ_HANDLE, Object), 2, arg);
	}
};