#pragma once
#include "main.h"
#include "splitPath.h"

//dirWatcher requires vista or latter to call GetFinalPathNameByHandleW.
//the API is necessary since the dir we are watching could also be moved to another path.
//and it is the only way to get the new path at that kind of situation.
//however, if you still need to use dirWatcher in winxp, it will work without watching
//the parent dir. and always fire an error at start up.
class dirWatcher:ObjectWrap {
private:
	HANDLE pathhnd;
	HANDLE parenthnd;
	uv_work_t pathreq;
	uv_work_t *parentreq;
	DWORD pathref;
	DWORD options;
	BOOL subDirs;
	void *pathbuffer;
	void *parentbuffer;
	Persistent<Object> definitions;//to store global v8 types
	static const size_t bufferSize = 64 * 1024;
	static const Persistent<String> syb_path;
	static const Persistent<String> syb_shortName;
	static const Persistent<String> syb_callback;
	static const Persistent<String> syb_opt_subDirs;
	static const Persistent<String> syb_opt_fileSize;
	static const Persistent<String> syb_opt_creation;
	static const Persistent<String> syb_opt_lastWrite;
	static const Persistent<String> syb_opt_lastAccess;
	static const Persistent<String> syb_opt_attributes;
	static const Persistent<String> syb_opt_security;
	static const Persistent<String> syb_evt_sta;
	static const Persistent<String> syb_evt_end;
	static const Persistent<String> syb_evt_new;
	static const Persistent<String> syb_evt_del;
	static const Persistent<String> syb_evt_ren;
	static const Persistent<String> syb_evt_chg;
	static const Persistent<String> syb_evt_mov;
	static const Persistent<String> syb_evt_err;
	static const Persistent<String> syb_evt_ren_oldName;
	static const Persistent<String> syb_evt_ren_newName;
	static const Persistent<String> syb_err_unable_to_watch_parent;
	static const Persistent<String> syb_err_unable_to_continue_watching;
	static const Persistent<String> syb_err_initialization_failed;
	static const Persistent<String> syb_err_wrong_arguments;
public:
	dirWatcher(Handle<Object> handle, Handle<Value> *args, uint32_t argc):ObjectWrap() {
		HandleScope scope;
		if (argc > 1 && (args[0]->IsString() || args[0]->IsStringObject()) && args[1]->IsFunction()) {
			bool e = false;
			Wrap(handle);
			Ref();
			definitions = Persistent<Object>::New(Object::New());
			definitions->Set(syb_callback, args[1]);
			String::Value spath(args[0]);
			pathhnd = CreateFileW((wchar_t*)*spath, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
			if (pathhnd == INVALID_HANDLE_VALUE) {
				e = true;
			} else {
				memset(&pathreq, 0, sizeof(uv_work_t));
				pathreq.loop = uv_default_loop();
				if (CreateIoCompletionPort(pathhnd, pathreq.loop->iocp, (ULONG_PTR)pathhnd, 0)) {
					pathref = 0;
					pathreq.type = UV_WORK;
					pathreq.data = this;
					pathreq.after_work_cb = finishWatchingPath;
					subDirs = TRUE;
					options = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE;
					if (argc > 2 && args[2]->IsObject()) {
						Handle<Object> iopt = Handle<Object>::Cast(args[2]);
						if (iopt->HasOwnProperty(syb_opt_subDirs) && iopt->Get(syb_opt_subDirs)->ToBoolean()->IsFalse()) {
							subDirs = FALSE;
						}
						if (iopt->HasOwnProperty(syb_opt_fileSize) && iopt->Get(syb_opt_fileSize)->ToBoolean()->IsFalse()) {
							options ^= FILE_NOTIFY_CHANGE_SIZE;
						}
						if (iopt->HasOwnProperty(syb_opt_lastWrite) && iopt->Get(syb_opt_lastWrite)->ToBoolean()->IsFalse()) {
							options ^= FILE_NOTIFY_CHANGE_LAST_WRITE;
						}
						if (iopt->Get(syb_opt_lastAccess)->ToBoolean()->IsTrue()) {
							options |= FILE_NOTIFY_CHANGE_LAST_ACCESS;
						}
						if (iopt->Get(syb_opt_creation)->ToBoolean()->IsTrue()) {
							options |= FILE_NOTIFY_CHANGE_CREATION;
						}
						if (iopt->Get(syb_opt_attributes)->ToBoolean()->IsTrue()) {
							options |= FILE_NOTIFY_CHANGE_ATTRIBUTES;
						}
						if (iopt->Get(syb_opt_security)->ToBoolean()->IsTrue()) {
							options |= FILE_NOTIFY_CHANGE_SECURITY;
						}
					}
					if (beginWatchingPath(this)) {
						Handle<String> path = getCurrentPathByHandle(pathhnd);//get the real path, it could be defferent from args[0]
						if (path->Length() > 0) {
							definitions->Set(syb_path, path);
							if (Handle<String>::Cast(splitPath::js(path)->Get(splitPath::syb_return_parent))->Length() > 0) {//path is not a rootdir, so we need to watch its parent to know if the path we are watching has been changed
								definitions->Set(syb_shortName, splitPath::js(convertPath::js(path, false))->Get(splitPath::syb_return_name));//we also need the shortname to makesure the event will be captured
								if (!watchParent(this)) {
									e = true;
								}
							} else {
								parenthnd = INVALID_HANDLE_VALUE;
							}
						} else {
							path = Handle<String>::Cast(args[0]);
							e = true;//the error is always fired in winxp and earlier since we can not get the real path
						}
						callJs(this, syb_evt_sta, path);
						if (e) {
							callJs(this, syb_evt_err, syb_err_unable_to_watch_parent);
							e = false;
						}
					} else {
						e = true;
					}
				} else {
					e = true;
				}
			}
			if (e) {
				callJs(this, syb_evt_err, syb_err_initialization_failed);
				stopWatching(this);
			}
		} else {
			ThrowException(Exception::Error(syb_err_wrong_arguments));
			delete this;
		}
	}
	virtual ~dirWatcher() {
		if (!definitions.IsEmpty()) {
			definitions.Dispose();
			definitions.Clear();
		}
	}
	static Handle<Function> functionRegister() {
		HandleScope scope;
		Handle<FunctionTemplate> t = FunctionTemplate::New(New);
		t->InstanceTemplate()->SetInternalFieldCount(1);
		//set methods
		NODE_SET_PROTOTYPE_METHOD(t, "close", close);

		//set error messages
		Handle<Object> errmsgs = Object::New();
		errmsgs->Set(syb_err_unable_to_watch_parent, syb_err_unable_to_watch_parent, global_syb_attr_const);
		errmsgs->Set(syb_err_unable_to_continue_watching, syb_err_unable_to_continue_watching, global_syb_attr_const);
		errmsgs->Set(syb_err_initialization_failed, syb_err_initialization_failed, global_syb_attr_const);
		errmsgs->Set(syb_err_wrong_arguments, syb_err_wrong_arguments, global_syb_attr_const);
		t->Set(String::NewSymbol("errors"), errmsgs, global_syb_attr_const);

		//set events
		Handle<Object> evts = Object::New();
		evts->Set(syb_evt_sta, syb_evt_sta, global_syb_attr_const);
		evts->Set(syb_evt_end, syb_evt_end, global_syb_attr_const);
		evts->Set(syb_evt_new, syb_evt_new, global_syb_attr_const);
		evts->Set(syb_evt_del, syb_evt_del, global_syb_attr_const);
		evts->Set(syb_evt_ren, syb_evt_ren, global_syb_attr_const);
		evts->Set(syb_evt_chg, syb_evt_chg, global_syb_attr_const);
		evts->Set(syb_evt_mov, syb_evt_mov, global_syb_attr_const);
		evts->Set(syb_evt_err, syb_evt_err, global_syb_attr_const);
		t->Set(String::NewSymbol("events"), evts, global_syb_attr_const);

		//set options
		Handle<Object> opts = Object::New();
		opts->Set(syb_opt_subDirs, syb_opt_subDirs, global_syb_attr_const);
		opts->Set(syb_opt_fileSize, syb_opt_fileSize, global_syb_attr_const);
		opts->Set(syb_opt_lastWrite, syb_opt_lastWrite, global_syb_attr_const);
		opts->Set(syb_opt_lastAccess, syb_opt_lastAccess, global_syb_attr_const);
		opts->Set(syb_opt_creation, syb_opt_creation, global_syb_attr_const);
		opts->Set(syb_opt_attributes, syb_opt_attributes, global_syb_attr_const);
		opts->Set(syb_opt_security, syb_opt_security, global_syb_attr_const);
		t->Set(String::NewSymbol("options"), opts, global_syb_attr_const);

		return scope.Close(t->GetFunction());
	}
private:
	static Handle<Value> New(const Arguments& args) {
		HandleScope scope;
		uint32_t i, l = args.Length();
		Handle<Value> *a = (Handle<Value>*)malloc(sizeof(Local<Value>)*l);
		Handle<Value> r;
		for (i = 0; i < l; i++) {
			a[i] = args[i];
		}
		if (args.IsConstructCall()) {
			Handle<Object> obj = args.This();
			new dirWatcher(obj, a, l);
			r = obj;
		} else {
			r = args.Callee()->CallAsConstructor(l, a);
		}
		free(a);
		return scope.Close(r);
	}
	static Handle<Value> close(const Arguments& args) {
		HandleScope scope;
		Handle<Value> result;
		dirWatcher *self = Unwrap<dirWatcher>(args.This());
		if (self->pathhnd == INVALID_HANDLE_VALUE) {
			result = False();//this method returns false if dirWatcher is failed to create or already closed
		} else {
			stopWatching(self);
			result = True();
		}
		return scope.Close(result);
	}
	static bool watchParent(dirWatcher *self) {
		bool result;
		self->parentreq = (uv_work_t*)malloc(sizeof(uv_work_t));
		if (self->parentreq) {
			memset(self->parentreq, 0, sizeof(uv_work_t));
			self->parentreq->loop = self->pathreq.loop;
			self->parentreq->data = self;
			self->parentreq->after_work_cb = finishWatchingParent;
			self->parentreq->type = UV_WORK;
			String::Value parent(splitPath::js(Handle<String>::Cast(self->definitions->Get(syb_path)))->Get(splitPath::syb_return_parent));
			self->parenthnd = CreateFileW((wchar_t*)*parent, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
			if (self->parenthnd == INVALID_HANDLE_VALUE) {
				result = false;
			} else {
				if (CreateIoCompletionPort(self->parenthnd, self->parentreq->loop->iocp, (ULONG_PTR)self->parenthnd, 0)) {
					result = beginWatchingParent(self);
				} else {
					CloseHandle(self->parenthnd);
					result = false;
				}
			}
		} else {
			self->parenthnd = INVALID_HANDLE_VALUE;
			result = false;
		}
		return result;
	}
	static bool beginWatchingParent(dirWatcher *self) {
		bool result;
		self->parentbuffer = malloc(bufferSize);
		if (self->parentbuffer) {
			if (ReadDirectoryChangesW(self->parenthnd, self->parentbuffer, bufferSize, FALSE, FILE_NOTIFY_CHANGE_DIR_NAME, NULL, &self->parentreq->overlapped, NULL)) {
				ngx_queue_insert_tail(&self->parentreq->loop->active_reqs, &self->parentreq->active_queue);
				result = true;
			} else {
				free(self->parentbuffer);
				result = false;
			}
		} else {
			result = false;
		}
		return result;
	}
	static bool beginWatchingPath(dirWatcher *self) {
		bool result;
		self->pathbuffer = malloc(bufferSize);
		if (self->pathbuffer) {
			if (ReadDirectoryChangesW(self->pathhnd, self->pathbuffer, bufferSize, self->subDirs, self->options, NULL, &self->pathreq.overlapped, NULL)) {
				ngx_queue_insert_tail(&self->pathreq.loop->active_reqs, &self->pathreq.active_queue);
				self->pathref++;//since we've called ngx_queue_insert_tail once
				result = true;
			} else {
				free(self->pathbuffer);
				result = false;
			}
		} else {
			result = false;
		}
		return result;
	}
	static void finishWatchingParent(uv_work_t *req, int status) {
		HandleScope scope;
		dirWatcher *self = (dirWatcher*)req->data;
		if (req != self->parentreq || self->parenthnd == INVALID_HANDLE_VALUE) {//this is the request we need to realase and it is ready to be released now
			free(req);
			if (self->pathhnd != INVALID_HANDLE_VALUE) {
				callJs(self, syb_evt_end, Null());
				self->Unref();
			}
		} else {
			void *buffer = self->parentbuffer;
			if (req->overlapped.Internal == ERROR_SUCCESS) {
				FILE_NOTIFY_INFORMATION *pInfo;
				DWORD d = 0;
				bool e = false;
				if (!beginWatchingParent(self)) {
					e = true;
				}
				Handle<Value> oldname = splitPath::js(Handle<String>::Cast(self->definitions->Get(syb_path)))->Get(splitPath::syb_return_name);
				do {
					pInfo = (FILE_NOTIFY_INFORMATION*)((ULONG_PTR)buffer + d);
					Handle<String> filename = String::New((uint16_t*)pInfo->FileName, pInfo->FileNameLength / sizeof(wchar_t));
					if ((pInfo->Action == FILE_ACTION_REMOVED || pInfo->Action == FILE_ACTION_RENAMED_OLD_NAME) && (filename->StrictEquals(oldname) || filename->StrictEquals(self->definitions->Get(syb_shortName)))) {
						Handle<String> newpath = getCurrentPathByHandle(self->pathhnd);
						self->definitions->Set(syb_path, newpath);
						self->definitions->Set(syb_shortName, splitPath::js(convertPath::js(newpath, false))->Get(splitPath::syb_return_name));
						if (pInfo->Action == FILE_ACTION_REMOVED) {
							CloseHandle(self->parenthnd);
							if (!watchParent(self)) {
								e = true;
							}
						}
						callJs(self, syb_evt_mov, newpath);
						break;//we've already got the new path
					}
					d += pInfo->NextEntryOffset;
				} while (pInfo->NextEntryOffset > 0);
				if (e) {
					callJs(self, syb_evt_err, syb_err_unable_to_watch_parent);
				}
			} else {
				callJs(self, syb_evt_err, syb_err_unable_to_watch_parent);
				CloseHandle(self->parenthnd);
				self->parenthnd = INVALID_HANDLE_VALUE;
			}
			free(buffer);
		}
	}
	static void finishWatchingPath(uv_work_t *req, int status) {
		HandleScope scope;
		dirWatcher *self = (dirWatcher*)req->data;
		void *buffer = self->pathbuffer;
		self->pathref--;//ngx_queue_remove will be called when this function ends if there's no crash
		if (req->overlapped.Internal == ERROR_SUCCESS) {
			FILE_NOTIFY_INFORMATION *pInfo;
			DWORD d = 0;
			if (!beginWatchingPath(self)) {
				callJs(self, syb_evt_err, syb_err_unable_to_continue_watching);
			}
			do {
				pInfo = (FILE_NOTIFY_INFORMATION*)((ULONG_PTR)buffer + d);
				Handle<String> filename = String::New((uint16_t*)pInfo->FileName, pInfo->FileNameLength / sizeof(wchar_t));
				if (pInfo->Action == FILE_ACTION_ADDED) {
					callJs(self, syb_evt_new, filename);
				} else if (pInfo->Action == FILE_ACTION_REMOVED) {
					callJs(self, syb_evt_del, filename);
				} else if (pInfo->Action == FILE_ACTION_MODIFIED) {
					callJs(self, syb_evt_chg, filename);
				} else if (pInfo->Action == FILE_ACTION_RENAMED_OLD_NAME) {
					if (self->definitions->HasOwnProperty(syb_evt_ren_newName)) {
						Handle<Object> arg = Object::New();
						arg->Set(syb_evt_ren_oldName, filename);
						arg->Set(syb_evt_ren_newName, self->definitions->Get(syb_evt_ren_newName));
						self->definitions->Delete(syb_evt_ren_newName);
						callJs(self, syb_evt_ren, arg);
					} else {
						self->definitions->Set(syb_evt_ren_oldName, filename);
					}
				} else if (pInfo->Action == FILE_ACTION_RENAMED_NEW_NAME) {
					if (self->definitions->HasOwnProperty(syb_evt_ren_oldName)) {
						Handle<Object> arg = Object::New();
						arg->Set(syb_evt_ren_oldName, self->definitions->Get(syb_evt_ren_oldName));
						arg->Set(syb_evt_ren_newName, filename);
						self->definitions->Delete(syb_evt_ren_oldName);
						callJs(self, syb_evt_ren, arg);
					} else {
						self->definitions->Set(syb_evt_ren_newName, filename);
					}
				}
				d += pInfo->NextEntryOffset;
			} while (pInfo->NextEntryOffset > 0);
		} else {
			callJs(self, syb_evt_err, syb_err_unable_to_continue_watching);
			stopWatching(self);
		}
		free(buffer);
	}
	static void stopWatching(dirWatcher *self) {
		if (self->pathhnd != INVALID_HANDLE_VALUE) {
			CloseHandle(self->pathhnd);
			self->pathhnd = INVALID_HANDLE_VALUE;
		}
		if (self->pathref > 0) {
			self->pathreq.type = UV_UNKNOWN_REQ;//mute this request
			ngx_queue_remove(&self->pathreq.active_queue);
			self->pathref--;
		}
		if (self->parenthnd != INVALID_HANDLE_VALUE) {
			CloseHandle(self->parenthnd);
			self->parenthnd = INVALID_HANDLE_VALUE;
		} else {
			callJs(self, syb_evt_end, Null());
			self->Unref();
		}
	}
	static void callJs(dirWatcher *self, Persistent<String> evt_type, Handle<Value> src) {
		HandleScope scope;
		Handle<Value> arg[2] = {evt_type->ToString(), src};
		Handle<Function> callback = Handle<Function>::Cast(self->definitions->Get(syb_callback));
		callback->Call(self->handle_, 2, arg);
	}
};
const Persistent<String> dirWatcher::syb_path = NODE_PSYMBOL("PATH");
const Persistent<String> dirWatcher::syb_shortName = NODE_PSYMBOL("SHORT_NAME");
const Persistent<String> dirWatcher::syb_callback = NODE_PSYMBOL("CALLBACK");
const Persistent<String> dirWatcher::syb_opt_subDirs = NODE_PSYMBOL("WATCH_SUB_DIRECTORIES");
const Persistent<String> dirWatcher::syb_opt_fileSize = NODE_PSYMBOL("CHANGE_FILE_SIZE");
const Persistent<String> dirWatcher::syb_opt_lastWrite = NODE_PSYMBOL("CHANGE_LAST_WRITE");
const Persistent<String> dirWatcher::syb_opt_lastAccess = NODE_PSYMBOL("CHANGE_LAST_ACCESS");
const Persistent<String> dirWatcher::syb_opt_creation = NODE_PSYMBOL("CHANGE_CREATION");
const Persistent<String> dirWatcher::syb_opt_attributes = NODE_PSYMBOL("CHANGE_ATTRIBUTES");
const Persistent<String> dirWatcher::syb_opt_security = NODE_PSYMBOL("CHANGE_SECUTITY");
const Persistent<String> dirWatcher::syb_evt_sta = NODE_PSYMBOL("STARTED");
const Persistent<String> dirWatcher::syb_evt_new = NODE_PSYMBOL("ADDED");
const Persistent<String> dirWatcher::syb_evt_del = NODE_PSYMBOL("REMOVED");
const Persistent<String> dirWatcher::syb_evt_chg = NODE_PSYMBOL("MODIFIED");
const Persistent<String> dirWatcher::syb_evt_ren = NODE_PSYMBOL("RENAMED");
const Persistent<String> dirWatcher::syb_evt_mov = NODE_PSYMBOL("MOVED");
const Persistent<String> dirWatcher::syb_evt_end = global_syb_evt_end;
const Persistent<String> dirWatcher::syb_evt_err = global_syb_evt_err;
const Persistent<String> dirWatcher::syb_evt_ren_oldName = NODE_PSYMBOL("OLD_NAME");
const Persistent<String> dirWatcher::syb_evt_ren_newName = NODE_PSYMBOL("NEW_NAME");
const Persistent<String> dirWatcher::syb_err_unable_to_watch_parent = NODE_PSYMBOL("UNABLE_TO_WATCH_PARENT");
const Persistent<String> dirWatcher::syb_err_unable_to_continue_watching = NODE_PSYMBOL("UNABLE_TO_CONTINUE_WATCHING");
const Persistent<String> dirWatcher::syb_err_initialization_failed = global_syb_err_initialization_failed;
const Persistent<String> dirWatcher::syb_err_wrong_arguments = global_syb_err_wrong_arguments;