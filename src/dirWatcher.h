#pragma once
#include "find.h"
#include "splitPath.h"
constexpr auto SYB_BUFFERSIZE = 64 * 1024;
constexpr auto SYB_EVT_ERR = "ERROR";
constexpr auto SYB_EVT_ERR_INITIALIZATION_FAILED = L"INITIALIZATION_FAILED";
constexpr auto SYB_EVT_ERR_UNABLE_TO_CONTINUE_WATCHING = L"UNABLE_TO_CONTINUE_WATCHING";
constexpr auto SYB_EVT_ERR_UNABLE_TO_WATCH_SELF = L"UNABLE_TO_WATCH_SELF";
constexpr auto SYB_EVT_RENAMED = "RENAMED";
constexpr auto SYB_EVT_RENAMED_OLD_NAME = "OLD_NAME";
constexpr auto SYB_EVT_RENAMED_NEW_NAME = "NEW_NAME";
constexpr auto SYB_OPT_WATCH_SUB_DIRECTORIES = "WATCH_SUB_DIRECTORIES";
constexpr auto SYB_OPT_CHANGE_FILE_SIZE = "CHANGE_FILE_SIZE";
constexpr auto SYB_OPT_CHANGE_LAST_WRITE = "CHANGE_LAST_WRITE";
constexpr auto SYB_OPT_CHANGE_LAST_ACCESS = "CHANGE_LAST_ACCESS";
constexpr auto SYB_OPT_CHANGE_CREATION = "CHANGE_CREATION";
constexpr auto SYB_OPT_CHANGE_ATTRIBUTES = "CHANGE_ATTRIBUTES";
constexpr auto SYB_OPT_CHANGE_SECURITY = "CHANGE_SECURITY";

class dirWatcher {
public:
	static napi_value init(napi_env env) {
		napi_property_descriptor properties[] = {
			{"close", NULL, close, NULL, NULL, NULL, napi_default, NULL}
		};
		napi_value result;
		napi_define_class(env, NULL, 0, Create, NULL, 1, properties, &result);
		napi_create_reference(env, result, 1, &constructor);
		return result;
	}
private:
	const struct msg {
		const char *type;
		const wchar_t *content;
		msg* next;
	};
	static napi_ref constructor;

	static napi_value Create(napi_env env, napi_callback_info info) {
		napi_value result, target, argv[3];
		size_t argc = 3;
		napi_get_new_target(env, info, &target);
		napi_get_cb_info(env, info, &argc, argv, &result, NULL);
		if (target) {
			if (argc < 2) {
				napi_throw_error(env, SYB_EXP_INVAL, SYB_ERR_WRONG_ARGUMENTS);
			} else {
				napi_valuetype t;
				napi_typeof(env, argv[1], &t);
				if (t == napi_function) {
					dirWatcher* self = new dirWatcher();
					self->watchingParent = self->watchingPath = 0;
					self->pathmsg = self->parentmsg = NULL;
					self->oldName = self->newName = self->longName = self->shortName = NULL;
					self->parenthnd = self->pathhnd = INVALID_HANDLE_VALUE;
					self->options = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE;
					self->subDirs = true;
					napi_value tmp;
					if (argc > 2 && napi_coerce_to_object(env, argv[2], &tmp) == napi_ok) {
						napi_value v;
						bool b;
						napi_has_named_property(env, tmp, SYB_OPT_WATCH_SUB_DIRECTORIES, &b);
						if (b) {
							napi_get_named_property(env, tmp, SYB_OPT_WATCH_SUB_DIRECTORIES, &v);
							napi_get_value_bool(env, v, &self->subDirs);
						}
						napi_has_named_property(env, tmp, SYB_OPT_CHANGE_FILE_SIZE, &b);
						if (b) {
							napi_get_named_property(env, tmp, SYB_OPT_CHANGE_FILE_SIZE, &v);
							napi_get_value_bool(env, v, &b);
							if (!b) {
								self->options ^= FILE_NOTIFY_CHANGE_SIZE;
							}
						}
						napi_has_named_property(env, tmp, SYB_OPT_CHANGE_LAST_WRITE, &b);
						if (b) {
							napi_get_named_property(env, tmp, SYB_OPT_CHANGE_LAST_WRITE, &v);
							napi_get_value_bool(env, v, &b);
							if (!b) {
								self->options ^= FILE_NOTIFY_CHANGE_LAST_WRITE;
							}
						}
						napi_has_named_property(env, tmp, SYB_OPT_CHANGE_LAST_ACCESS, &b);
						if (b) {
							napi_get_named_property(env, tmp, SYB_OPT_CHANGE_LAST_ACCESS, &v);
							napi_get_value_bool(env, v, &b);
							if (b) {
								self->options |= FILE_NOTIFY_CHANGE_LAST_ACCESS;
							}
						}
						napi_has_named_property(env, tmp, SYB_OPT_CHANGE_CREATION, &b);
						if (b) {
							napi_get_named_property(env, tmp, SYB_OPT_CHANGE_CREATION, &v);
							napi_get_value_bool(env, v, &b);
							if (b) {
								self->options |= FILE_NOTIFY_CHANGE_CREATION;
							}
						}
						napi_has_named_property(env, tmp, SYB_OPT_CHANGE_ATTRIBUTES, &b);
						if (b) {
							napi_get_named_property(env, tmp, SYB_OPT_CHANGE_ATTRIBUTES, &v);
							napi_get_value_bool(env, v, &b);
							if (b) {
								self->options |= FILE_NOTIFY_CHANGE_ATTRIBUTES;
							}
						}
						napi_has_named_property(env, tmp, SYB_OPT_CHANGE_SECURITY, &b);
						if (b) {
							napi_get_named_property(env, tmp, SYB_OPT_CHANGE_SECURITY, &v);
							napi_get_value_bool(env, v, &b);
							if (b) {
								self->options |= FILE_NOTIFY_CHANGE_SECURITY;
							}
						}
					}
					napi_coerce_to_string(env, argv[0], &tmp);
					size_t str_len;
					napi_get_value_string_utf16(env, tmp, NULL, 0, &str_len);
					str_len += 1;
					self->path = (wchar_t*)malloc(sizeof(wchar_t) * str_len);
					napi_get_value_string_utf16(env, tmp, (char16_t*)self->path, str_len, NULL);
					napi_value resname;
					napi_create_string_latin1(env, "fswin.dirWatcher", NAPI_AUTO_LENGTH, &resname);
					napi_wrap(env, result, self, Destroy, NULL, &self->wrapper);
					napi_create_reference(env, argv[1], 1, &self->callback);
					napi_create_async_work(env, tmp, resname, beginWatchingPath, finishWatchingPath, self, &self->pathwork);
					napi_create_async_work(env, tmp, resname, beginWatchingParent, finishWatchingParent, self, &self->parentwork);
					self->watchPath(env);
				} else {
					napi_throw_error(env, SYB_EXP_INVAL, SYB_ERR_WRONG_ARGUMENTS);
				}
			}
		} else {
			napi_value cons;
			napi_get_reference_value(env, constructor, &cons);
			napi_new_instance(env, cons, argc, (napi_value*)&argv, &result);
		}
		return result;
	}
	static void Destroy(napi_env env, void* nativeObject, void* finalize_hint) {
		dirWatcher *self = (dirWatcher*)nativeObject;
		napi_delete_reference(env, self->wrapper);
		napi_delete_reference(env, self->callback);
		napi_delete_async_work(env, self->pathwork);
		napi_delete_async_work(env, self->parentwork);
		free(self->path);
		delete self;
	}
	static napi_value close(napi_env env, napi_callback_info info) {//this method returns false if dirWatcher is failed to create or already closed
		napi_value that, result;
		dirWatcher *self;
		napi_get_cb_info(env, info, NULL, NULL, &that, NULL);
		napi_unwrap(env, that, (void**)&self);
		if (self->watchingPath == 0 && self->pathhnd == INVALID_HANDLE_VALUE) {
			napi_get_boolean(env, false, &result);
		} else {
			self->stopWatching(env);
			napi_get_boolean(env, true, &result);
		}
		return result;
	}
	static void beginWatchingPath(napi_env env, void *data) {
		dirWatcher *self = (dirWatcher*)data;
		self->watchingPath = 2;
		if (self->pathhnd == INVALID_HANDLE_VALUE) {
			self->pathhnd = CreateFileW(self->path, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
			if (self->pathhnd == INVALID_HANDLE_VALUE) {
				self->pathmsg = (msg*)malloc(sizeof(msg));
				self->pathmsg->type = SYB_EVT_ERR;
				self->pathmsg->content = SYB_EVT_ERR_INITIALIZATION_FAILED;
				self->pathmsg->next = NULL;
			} else if (self->parenthnd == INVALID_HANDLE_VALUE && self->parentwork) {
				self->savePath(env, true);
			}
		} else {
			self->pathbuffer = malloc(SYB_BUFFERSIZE);
			DWORD b;
			self->watchingPathResult = ReadDirectoryChangesW(self->pathhnd, self->pathbuffer, SYB_BUFFERSIZE, self->subDirs, self->options, &b, NULL, NULL);
		}
	}
	static void finishWatchingPath(napi_env env, napi_status status, void *data) {
		dirWatcher *self = (dirWatcher*)data;
		self->watchingPath = 0;
		if (status == napi_ok) {
			if (self->pathmsg) {
				while (self->pathmsg) {
					napi_value v;
					napi_create_string_utf16(env, (char16_t*)self->pathmsg->content, NAPI_AUTO_LENGTH, &v);
					self->callCb(env, self->pathmsg->type, v);
					msg* m = self->pathmsg;
					self->pathmsg = m->next;
					free(m);
				}
				if (self->pathhnd == INVALID_HANDLE_VALUE) {
					self->checkWatchingStoped(env);
				} else {
					self->watchPath(env);
				}
			} else {
				if (self->pathhnd == INVALID_HANDLE_VALUE) {
					free(self->pathbuffer);
					self->checkWatchingStoped(env);
				} else {
					if (self->watchingPathResult) {
						void *buffer = self->pathbuffer;
						self->watchPath(env);
						FILE_NOTIFY_INFORMATION *pInfo;
						DWORD d = 0;
						do {
							pInfo = (FILE_NOTIFY_INFORMATION*)((ULONG_PTR)buffer + d);
							d += pInfo->NextEntryOffset;
							napi_value filename;
							napi_create_string_utf16(env, (char16_t*)pInfo->FileName, pInfo->FileNameLength / sizeof(wchar_t), &filename);
							if (pInfo->Action == FILE_ACTION_ADDED) {
								self->callCb(env, "ADDED", filename);
							} else if (pInfo->Action == FILE_ACTION_REMOVED) {
								self->callCb(env, "REMOVED", filename);
							} else if (pInfo->Action == FILE_ACTION_MODIFIED) {
								self->callCb(env, "MODIFIED", filename);
							} else {
								if (pInfo->Action == FILE_ACTION_RENAMED_OLD_NAME) {
									if (self->newName) {
										napi_value arg, tmp;
										napi_create_object(env, &arg);
										napi_set_named_property(env, arg, SYB_EVT_RENAMED_OLD_NAME, filename);
										napi_create_string_utf16(env, (char16_t*)self->newName, NAPI_AUTO_LENGTH, &tmp);
										napi_set_named_property(env, arg, SYB_EVT_RENAMED_NEW_NAME, tmp);
										free(self->newName);
										self->newName = NULL;
										self->callCb(env, SYB_EVT_RENAMED, arg);
									} else {
										size_t sz = pInfo->FileNameLength + 1;
										self->oldName = (wchar_t*)malloc(sizeof(wchar_t) * sz);
										wcscpy_s(self->oldName, sz, pInfo->FileName);
									}
								} else if (pInfo->Action == FILE_ACTION_RENAMED_NEW_NAME) {
									if (self->oldName) {
										napi_value arg, tmp;
										napi_create_object(env, &arg);
										napi_set_named_property(env, arg, SYB_EVT_RENAMED_NEW_NAME, filename);
										napi_create_string_utf16(env, (char16_t*)self->oldName, NAPI_AUTO_LENGTH, &tmp);
										napi_set_named_property(env, arg, SYB_EVT_RENAMED_OLD_NAME, tmp);
										free(self->oldName);
										self->oldName = NULL;
										self->callCb(env, SYB_EVT_RENAMED, arg);
									} else {
										size_t sz = pInfo->FileNameLength + 1;
										self->newName = (wchar_t*)malloc(sizeof(wchar_t) * sz);
										wcscpy_s(self->newName, sz, pInfo->FileName);
									}
								}
							}
						} while (pInfo->NextEntryOffset > 0);
						free(buffer);
					} else {
						free(self->pathbuffer);
						napi_value v;
						napi_create_string_utf16(env, (char16_t*)SYB_EVT_ERR_UNABLE_TO_CONTINUE_WATCHING, NAPI_AUTO_LENGTH, &v);
						self->callCb(env, SYB_EVT_ERR, v);
						self->stopWatching(env);
					}
				}
			}
		} else {
			self->checkWatchingStoped(env);
		}
	}
	static void beginWatchingParent(napi_env env, void *data) {
		dirWatcher *self = (dirWatcher*)data;
		self->watchingParent = 2;
		if (self->parenthnd == INVALID_HANDLE_VALUE) {
			self->savePath(env);
		} else {
			DWORD d;
			self->watchingParentResult = ReadDirectoryChangesW(self->parenthnd, self->parentbuffer, SYB_BUFFERSIZE, FALSE, FILE_NOTIFY_CHANGE_DIR_NAME, &d, NULL, NULL);
		}
	}
	static void finishWatchingParent(napi_env env, napi_status status, void *data) {
		dirWatcher *self = (dirWatcher*)data;
		self->watchingParent = 0;
		if (status == napi_ok) {
			if (self->parentmsg) {
				while (self->parentmsg) {
					napi_value v;
					napi_create_string_utf16(env, (char16_t*)self->parentmsg->content, NAPI_AUTO_LENGTH, &v);
					self->callCb(env, self->parentmsg->type, v);
					msg* m = self->parentmsg;
					self->parentmsg = m->next;
					free(m);
				}
				if (self->parenthnd == INVALID_HANDLE_VALUE) {
					self->checkWatchingStoped(env);
				} else {
					self->watchParent(env);
				}
			} else {
				if (self->parenthnd == INVALID_HANDLE_VALUE) {
					self->checkWatchingStoped(env);
				} else {
					if (self->watchingParentResult) {
						FILE_NOTIFY_INFORMATION *pInfo;
						DWORD d = 0;
						do {
							pInfo = (FILE_NOTIFY_INFORMATION*)((ULONG_PTR)self->parentbuffer + d);
							d += pInfo->NextEntryOffset;
							if ((pInfo->Action == FILE_ACTION_RENAMED_OLD_NAME || pInfo->Action == FILE_ACTION_REMOVED) && wcsncmp(self->longName, pInfo->FileName, MAX(pInfo->FileNameLength / sizeof(wchar_t), wcslen(self->longName))) == 0 || (self->shortName && wcsncmp(self->shortName, pInfo->FileName, MAX(pInfo->FileNameLength / sizeof(wchar_t), wcslen(self->shortName))) == 0)) {
								CloseHandle(self->parenthnd);
								self->parenthnd = INVALID_HANDLE_VALUE;
								break;
							}
						} while (pInfo->NextEntryOffset > 0);
						self->watchParent(env);
					} else {
						napi_value v;
						napi_create_string_utf16(env, (char16_t*)SYB_EVT_ERR_UNABLE_TO_WATCH_SELF, NAPI_AUTO_LENGTH, &v);
						self->callCb(env, SYB_EVT_ERR, v);
						self->stopWatchingParent(env);
					}
				}
			}
		} else {
			self->checkWatchingStoped(env);
		}
	}

	msg *pathmsg;
	msg *parentmsg;
	HANDLE pathhnd;
	HANDLE parenthnd;
	napi_async_work pathwork;
	napi_async_work parentwork;
	BOOL watchingPathResult;
	BOOL watchingParentResult;
	BYTE watchingPath;
	BYTE watchingParent;
	bool subDirs;
	DWORD options;
	wchar_t *path;
	wchar_t *oldName;
	wchar_t *newName;
	wchar_t *shortName;
	wchar_t *longName;
	void *pathbuffer;
	BYTE *parentbuffer[SYB_BUFFERSIZE];
	napi_ref wrapper;
	napi_ref callback;

	void watchPath(napi_env env) {
		if (napi_queue_async_work(env, this->pathwork) == napi_ok) {
			this->watchingPath = 1;
		} else {
			napi_value v;
			napi_create_string_utf16(env, (char16_t*)(this->parenthnd == INVALID_HANDLE_VALUE && this->parentwork ? SYB_EVT_ERR_INITIALIZATION_FAILED : SYB_EVT_ERR_UNABLE_TO_CONTINUE_WATCHING), NAPI_AUTO_LENGTH, &v);
			this->callCb(env, SYB_EVT_ERR, v);
			this->stopWatching(env);
		}
	}
	void watchParent(napi_env env) {
		if (napi_queue_async_work(env, this->parentwork) == napi_ok) {
			this->watchingParent = 1;
		} else {
			napi_value v;
			napi_create_string_utf16(env, (char16_t*)SYB_EVT_ERR_UNABLE_TO_WATCH_SELF, NAPI_AUTO_LENGTH, &v);
			this->callCb(env, SYB_EVT_ERR, v);
			this->stopWatchingParent(env);
		}
	}
	void savePath(napi_env env, bool starting = false) {
		wchar_t *realPath = getCurrentPathByHandle(this->pathhnd);
		bool e = false;
		if (realPath) {
			free(this->path);
			this->path = realPath;
			splitPath::splitedPath *sp = splitPath::func(this->path);
			if (sp->parentLen > 0) {
				if (this->shortName) {
					free(this->shortName);
					this->shortName = NULL;
				}
				if (this->longName) {
					free(this->longName);
					this->longName = NULL;
				}
				find::resultData *fr = find::func(this->path);
				if (fr) {
					this->longName = _wcsdup(fr->data.cFileName);
					if (wcslen(fr->data.cAlternateFileName) > 0 && wcscmp(fr->data.cFileName, fr->data.cAlternateFileName) != 0) {
						this->shortName = _wcsdup(fr->data.cAlternateFileName);
					}
					free(fr);
				}
				if (this->longName) {
					wchar_t *parent = (wchar_t*)malloc(sizeof(wchar_t) * (sp->parentLen + 1));
					wcsncpy_s(parent, sp->parentLen + 1, realPath, sp->parentLen);
					this->parenthnd = CreateFileW(parent, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
					free(parent);
					if (this->parenthnd == INVALID_HANDLE_VALUE) {
						e = true;
					} else if (starting) {
						if (napi_queue_async_work(env, this->parentwork) == napi_ok) {
							this->watchingParent = 1;
						} else {
							e = true;
						}
					}
				}
			} else {
				this->stopWatchingParent(env);
			}
			free(sp);
		} else {
			e = true;
		}
		msg *m = (msg*)malloc(sizeof(msg));
		m->content = this->path;
		if (starting) {
			m->type = "STARTED";
			this->pathmsg = m;
		} else {
			m->type = "MOVED";
			this->parentmsg = m;
		}
		if (e) {
			m->next = (msg*)malloc(sizeof(msg));
			m->next->type = SYB_EVT_ERR;
			m->next->content = SYB_EVT_ERR_UNABLE_TO_WATCH_SELF;
			m->next->next = NULL;
			this->stopWatchingParent(env);
		} else {
			m->next = NULL;
		}
	}
	void stopWatching(napi_env env) {
		if (this->pathhnd != INVALID_HANDLE_VALUE) {
			CloseHandle(this->pathhnd);
			this->pathhnd = INVALID_HANDLE_VALUE;
		}
		if (this->newName) {
			free(this->newName);
			this->newName = NULL;
		}
		if (this->oldName) {
			free(this->oldName);
			this->oldName = NULL;
		}
		if (this->watchingPath == 1) {
			napi_cancel_async_work(env, this->pathwork);
		}
		this->stopWatchingParent(env);
		this->checkWatchingStoped(env);
	}
	void stopWatchingParent(napi_env env) {
		if (this->parenthnd != INVALID_HANDLE_VALUE) {
			CloseHandle(this->parenthnd);
			this->parenthnd = INVALID_HANDLE_VALUE;
		}
		if (this->longName) {
			free(this->longName);
			this->longName = NULL;
		}
		if (this->shortName) {
			free(this->shortName);
			this->shortName = NULL;
		}
		if (this->watchingParent == 1) {
			napi_cancel_async_work(env, this->parentwork);
		}
	}
	void checkWatchingStoped(napi_env env) {
		if (this->watchingPath == 0 && this->watchingParent == 0) {
			napi_value v;
			napi_get_undefined(env, &v);
			this->callCb(env, "ENDED", v);
		}
	}
	void callCb(napi_env env, const char *evt_type, napi_value content) {
		napi_value argv[2], func, that;
		napi_create_string_latin1(env, evt_type, NAPI_AUTO_LENGTH, &argv[0]);
		argv[1] = content;
		napi_get_reference_value(env, this->callback, &func);
		napi_get_reference_value(env, this->wrapper, &that);
		napi_call_function(env, that, func, 2, &argv[0], NULL);
	}
};
napi_ref dirWatcher::constructor;