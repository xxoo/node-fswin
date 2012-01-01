#define FSWIN_VERSION "0.1.2012.0101"

#include <node.h>
#pragma comment(lib,"node.lib")
using namespace v8;
using namespace node;

//#include <iostream>//for debug only
//using namespace std;

//dirWatcher requires vista or latter to call GetFinalPathNameByHandleW.
//the API is necessary since the dir we are watching could also be moved to another path.
//and it is the only way to get the new path at that kind of situation.
//however, if you still need to use dirWatcher in winxp, it will work without watching
//the parent dir. and always fire an error at start up.
#ifndef GetFinalPathNameByHandle
	typedef DWORD (WINAPI *GetFinalPathNameByHandle)(__in HANDLE hFile,__out_ecount(cchFilePath) LPWSTR lpszFilePath,__in DWORD cchFilePath,__in DWORD dwFlags);
	static const GetFinalPathNameByHandle GetFinalPathNameByHandleW=(GetFinalPathNameByHandle)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "GetFinalPathNameByHandleW");
#endif

namespace fsWin{
	static const Persistent<String> global_syb_err_wrong_arguments=NODE_PSYMBOL("WRONG_ARGUMENTS");
	static const Persistent<String> global_syb_err_not_a_constructor=NODE_PSYMBOL("THIS_FUNCTION_IS_NOT_A_CONSTRUCTOR");
	static const Persistent<String> global_syb_err_out_of_memory=NODE_PSYMBOL("OUT_OF_MEMORY");
	static const Persistent<String> global_syb_err_initialization_failed=NODE_PSYMBOL("INITIALIZATION_FAILED");

	//this function returns an Object with two properties: parent and name
	//the parent property could be empty if path is a rootdir
	static Handle<Object> splitPath(Handle<String> path){
		HandleScope scope;
		String::Value p1(path);
		Handle<Object> r=Object::New();
		wchar_t *p2=(wchar_t*)*p1,*s=L"\\\\";
		size_t i,j=0,k=0,l=wcslen(p2),m=wcslen(s);
		if(wcsncmp(s,p2,m)==0){//is network path
			for(i=m+1;i<l-1;i++){
				if(p2[i]==L'\\'){
					if(++k==2){
						j=i;
						break;
					}
				}
			}
			if(k==2){
				for(i=l-2;i>j+1;i--){
					if(p2[i]==L'\\'){
						j=i;
						break;
					}
				}
			}
			k=0;
		}else{//is local path
			for(i=l-2;i>1;i--){
				if(p2[i]==L'\\'){
					if(j==0){//perhaps it's a rootdir
						j=i+1;
					}else{//it's not a rootdir
						j-=1;
						k=1;
						break;
					}
				}
			}
		}
		r->Set(String::NewSymbol("parent"),String::New(*p1,j));
		r->Set(String::NewSymbol("name"),String::New(j>0?&(*p1)[j+k]:*p1));
		return scope.Close(r);
	}
	static Handle<Value> splitPathForJs(const Arguments& args){
		HandleScope scope;
		if(args.IsConstructCall()){
			return ThrowException(global_syb_err_not_a_constructor);
		}else{
			if(args.Length()>0&&(args[0]->IsString()||args[0]->IsStringObject())){
				return scope.Close(splitPath(Handle<String>::Cast(args[0])));
			}else{
				return ThrowException(global_syb_err_wrong_arguments);
			}
		}
	}

	class convertPath{
	public:
		static wchar_t* basic(const wchar_t* path,bool islong){//you need to free the result yourself if it is not NULL
			wchar_t *tpath;
			DWORD sz=islong?GetLongPathNameW(path,NULL,0):GetShortPathNameW(path,NULL,0);
			if(sz>0){
				tpath=(wchar_t*)malloc(sz*sizeof(wchar_t));
				islong?GetLongPathNameW(path,tpath,sz):GetShortPathNameW(path,tpath,sz);
			}else{
				tpath=NULL;
			}
			return tpath;
		}
		static Handle<String> js(Handle<String> path,bool islong){
			HandleScope scope;
			Handle<String> r;
			String::Value spath(path);
			wchar_t* tpath=basic((wchar_t*)*spath,islong);
			if(tpath){
				r=String::New((uint16_t*)tpath);
				free(tpath);
			}else{
				r=String::New("");
			}
			return scope.Close(r);
		}
		static Handle<Value> jsSync(const Arguments& args){
			HandleScope scope;
			if(args.IsConstructCall()){
				return ThrowException(global_syb_err_not_a_constructor);
			}else{
				if(args.Length()>0&&(args[0]->IsString()||args[0]->IsStringObject())){
					return scope.Close(js(Handle<String>::Cast(args[0]),args[1]->ToBoolean()->IsTrue()));
				}else{
					return ThrowException(global_syb_err_wrong_arguments);
				}
			}
		}
		static Handle<Value> jsAsync(const Arguments& args){
			HandleScope scope;
			if(args.IsConstructCall()){
				return ThrowException(global_syb_err_not_a_constructor);
			}else{
				if(args.Length()>1&&(args[0]->IsString()||args[0]->IsStringObject())&&args[1]->IsFunction()){
					workdata *data=new workdata;
					data->req.data=data;
					data->self=Persistent<Object>::New(args.This());
					data->func=Persistent<Function>::New(Handle<Function>::Cast(args[1]));
					data->islong=args[2]->ToBoolean()->IsTrue();
					data->path=_wcsdup((wchar_t*)*String::Value(Local<String>::Cast(args[0])));
					return uv_queue_work(uv_default_loop(),&data->req,beginWork,afterWork)==0?True():False();
				}else{
					return ThrowException(global_syb_err_wrong_arguments);
				}
			}
		}
	private:
		static struct workdata{
			uv_work_t req;
			Persistent<Object> self;
			Persistent<Function> func;
			wchar_t* path;
			bool islong;
		};
		static void beginWork(uv_work_t *req){
			workdata *data=(workdata*)req->data;
			wchar_t* p=basic(data->path,data->islong);
			free(data->path);
			data->path=p;
		}
		static void afterWork(uv_work_t *req){
			HandleScope scope;
			workdata *data=(workdata*)req->data;
			Handle<Value> p;
			if(data->path){
				p=String::New((uint16_t*)data->path);
				free(data->path);
			}else{
				p=String::New("");
			}
			data->func->Call(data->self,1,&p);
			data->func.Dispose();
			data->self.Dispose();
			delete data;
		}
	};

	class dirWatcher:ObjectWrap{
	public:
		static Handle<Function> init(Handle<String> classname){
			HandleScope scope;
			Handle<FunctionTemplate> t=FunctionTemplate::New(New);
			t->InstanceTemplate()->SetInternalFieldCount(1);
			//set methods
			NODE_SET_PROTOTYPE_METHOD(t,"close",close);

			//set error messages
			Handle<Object> errmsgs=Object::New();
			errmsgs->Set(syb_err_unable_to_watch_parent,syb_err_unable_to_watch_parent,(PropertyAttribute)(ReadOnly|DontDelete));
			errmsgs->Set(syb_err_unable_to_continue_watching,syb_err_unable_to_continue_watching,(PropertyAttribute)(ReadOnly|DontDelete));
			errmsgs->Set(syb_err_initialization_failed,syb_err_initialization_failed,(PropertyAttribute)(ReadOnly|DontDelete));
			errmsgs->Set(syb_err_wrong_arguments,syb_err_wrong_arguments,(PropertyAttribute)(ReadOnly|DontDelete));
			t->Set(String::NewSymbol("errors"),errmsgs,(PropertyAttribute)(ReadOnly|DontDelete));

			//set events
			Handle<Object> evts=Object::New();
			evts->Set(syb_evt_sta,syb_evt_sta,(PropertyAttribute)(ReadOnly|DontDelete));
			evts->Set(syb_evt_end,syb_evt_end,(PropertyAttribute)(ReadOnly|DontDelete));
			evts->Set(syb_evt_new,syb_evt_new,(PropertyAttribute)(ReadOnly|DontDelete));
			evts->Set(syb_evt_del,syb_evt_del,(PropertyAttribute)(ReadOnly|DontDelete));
			evts->Set(syb_evt_ren,syb_evt_ren,(PropertyAttribute)(ReadOnly|DontDelete));
			evts->Set(syb_evt_chg,syb_evt_chg,(PropertyAttribute)(ReadOnly|DontDelete));
			evts->Set(syb_evt_mov,syb_evt_mov,(PropertyAttribute)(ReadOnly|DontDelete));
			evts->Set(syb_evt_err,syb_evt_err,(PropertyAttribute)(ReadOnly|DontDelete));
			t->Set(String::NewSymbol("events"),evts,(PropertyAttribute)(ReadOnly|DontDelete));

			//set options
			Handle<Object> opts=Object::New();
			opts->Set(syb_opt_subDirs,syb_opt_subDirs,(PropertyAttribute)(ReadOnly|DontDelete));
			opts->Set(syb_opt_fileSize,syb_opt_fileSize,(PropertyAttribute)(ReadOnly|DontDelete));
			opts->Set(syb_opt_lastWrite,syb_opt_lastWrite,(PropertyAttribute)(ReadOnly|DontDelete));
			opts->Set(syb_opt_lastAccess,syb_opt_lastAccess,(PropertyAttribute)(ReadOnly|DontDelete));
			opts->Set(syb_opt_creation,syb_opt_creation,(PropertyAttribute)(ReadOnly|DontDelete));
			opts->Set(syb_opt_attributes,syb_opt_attributes,(PropertyAttribute)(ReadOnly|DontDelete));
			opts->Set(syb_opt_security,syb_opt_security,(PropertyAttribute)(ReadOnly|DontDelete));
			t->Set(String::NewSymbol("options"),opts,(PropertyAttribute)(ReadOnly|DontDelete));

			t->SetClassName(classname);
			return scope.Close(t->GetFunction());
		}
		static Handle<Value> New(const Arguments& args){
			HandleScope scope;
			uint32_t i,l=args.Length();
			Handle<Value> *a=(Handle<Value>*)malloc(sizeof(Local<Value>)*l);
			Handle<Value> r;
			for(i=0;i<l;i++){
				a[i]=args[i];
			}
			if(args.IsConstructCall()){
				Handle<Object> obj=args.This();
				new dirWatcher(obj,a,l);
				r=obj;
			}else{
				r=args.Callee()->CallAsConstructor(l,a);
			}
			free(a);
			return scope.Close(r);
		}
		dirWatcher(Handle<Object> handle,Handle<Value> *args,uint32_t argc):ObjectWrap(){
			HandleScope scope;
			if(argc>1&&(args[0]->IsString()||args[0]->IsStringObject())&&args[1]->IsFunction()){
				bool e=false;
				Wrap(handle);
				Ref();
				definitions=Persistent<Object>::New(Object::New());
				definitions->Set(syb_callback,args[1]);
				String::Value spath(args[0]);
				pathhnd=CreateFileW((wchar_t*)*spath,FILE_LIST_DIRECTORY,FILE_SHARE_READ|FILE_SHARE_DELETE|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OVERLAPPED,NULL);
				if(pathhnd){
					ZeroMemory(&pathreq,sizeof(pathreq));
					pathreq.loop=uv_default_loop();
					if(CreateIoCompletionPort(pathhnd,pathreq.loop->iocp,(ULONG_PTR)pathhnd,0)){
						pathref=0;
						pathreq.type=UV_WORK;
						pathreq.data=this;
						pathreq.after_work_cb=finishWatchingPath;
						subDirs=TRUE;
						options=FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_DIR_NAME|FILE_NOTIFY_CHANGE_SIZE|FILE_NOTIFY_CHANGE_LAST_WRITE;
						if(argc>2&&args[2]->IsObject()){
							Handle<Object> iopt=Handle<Object>::Cast(args[2]);
							if(iopt->HasOwnProperty(syb_opt_subDirs)&&iopt->Get(syb_opt_subDirs)->ToBoolean()->IsFalse()){
								subDirs=FALSE;
							}
							if(iopt->HasOwnProperty(syb_opt_fileSize)&&iopt->Get(syb_opt_fileSize)->ToBoolean()->IsFalse()){
								options^=FILE_NOTIFY_CHANGE_SIZE;
							}
							if(iopt->HasOwnProperty(syb_opt_lastWrite)&&iopt->Get(syb_opt_lastWrite)->ToBoolean()->IsFalse()){
								options^=FILE_NOTIFY_CHANGE_LAST_WRITE;
							}
							if(iopt->Get(syb_opt_lastAccess)->ToBoolean()->IsTrue()){
								options|=FILE_NOTIFY_CHANGE_LAST_ACCESS;
							}
							if(iopt->Get(syb_opt_creation)->ToBoolean()->IsTrue()){
								options|=FILE_NOTIFY_CHANGE_CREATION;
							}
							if(iopt->Get(syb_opt_attributes)->ToBoolean()->IsTrue()){
								options|=FILE_NOTIFY_CHANGE_ATTRIBUTES;
							}
							if(iopt->Get(syb_opt_security)->ToBoolean()->IsTrue()){
								options|=FILE_NOTIFY_CHANGE_SECURITY;
							}
						}
						if(beginWatchingPath(this)){
							Handle<String> path;
							if((void*)GetFinalPathNameByHandleW){//check if GetFinalPathNameByHandleW is supported
								path=getCurrentPathByHandle(pathhnd);//get the real path, it could be defferent from args[0]
								definitions->Set(syb_path,path);
								if(Handle<String>::Cast(splitPath(path)->Get(String::NewSymbol("parent")))->Length()>0){//path is not a rootdir, so we need to watch its parent to know if the path we are watching has been changed
									definitions->Set(syb_shortName,splitPath(convertPath::js(path,false))->Get(String::NewSymbol("name")));//we also need the shortname to makesure the event will be captured
									if(!watchParent(this)){
										e=true;
									}
								}else{
									parenthnd=NULL;
								}
							}else{
								path=Handle<String>::Cast(args[0]);
								e=true;//the error is always fired in winxp and earlier since we can not get the real path
							}
							callJs(this,syb_evt_sta,path);
							if(e){
								callJs(this,syb_evt_err,syb_err_unable_to_watch_parent);
								e=false;
							}
						}else{
							e=true;
						}
					}else{
						e=true;
					}
				}else{
					e=true;
				}
				if(e){
					callJs(this,syb_evt_err,syb_err_initialization_failed);
					stopWatching(this);
				}
			}else{
				ThrowException(Exception::Error(syb_err_wrong_arguments));
				delete this;
			}
		}
		virtual ~dirWatcher(){
			definitions.Dispose();
			definitions.Clear();
		}
		static Handle<Value> close(const Arguments& args){
			HandleScope scope;
			dirWatcher* self=ObjectWrap::Unwrap<dirWatcher>(args.This());
			if(self->pathhnd){//this method returns false if dirWatcher is failed to create or already closed
				stopWatching(self);
				return True();
			}else{
				return False();
			}
		}
	private:
		static bool watchParent(dirWatcher *self){
			bool result;
			self->parentreq=(uv_work_t*)malloc(sizeof(uv_work_t));
			if(self->parentreq){
				ZeroMemory(self->parentreq,sizeof(uv_work_t));
				self->parentreq->loop=self->pathreq.loop;
				self->parentreq->data=self;
				self->parentreq->after_work_cb=finishWatchingParent;
				self->parentreq->type=UV_WORK;
				String::Value parent(splitPath(Handle<String>::Cast(self->definitions->Get(syb_path)))->Get(String::NewSymbol("parent")));
				self->parenthnd=CreateFileW((wchar_t*)*parent,FILE_LIST_DIRECTORY,FILE_SHARE_READ|FILE_SHARE_DELETE|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OVERLAPPED,NULL);
				if(self->parenthnd){
					if(CreateIoCompletionPort(self->parenthnd,self->parentreq->loop->iocp,(ULONG_PTR)self->parenthnd,0)){
						result=beginWatchingParent(self);
					}else{
						CloseHandle(self->parenthnd);
						result=false;
					}
				}else{
					result=false;
				}
			}else{
				self->parenthnd=NULL;
				result=false;
			}
			return result;
		}
		static bool beginWatchingParent(dirWatcher *self){
			bool result;
			self->parentbuffer=malloc(bufferSize);
			if(self->parentbuffer){
				if(ReadDirectoryChangesW(self->parenthnd,self->parentbuffer,bufferSize,FALSE,FILE_NOTIFY_CHANGE_DIR_NAME,NULL,&self->parentreq->overlapped,NULL)){
					uv_ref(self->parentreq->loop);
					result=true;
				}else{
					free(self->parentbuffer);
					result=false;
				}
			}else{
				result=false;
			}
			return result;
		}
		static bool beginWatchingPath(dirWatcher *self){
			bool result;
			self->pathbuffer=malloc(bufferSize);
			if(self->pathbuffer){
				if(ReadDirectoryChangesW(self->pathhnd,self->pathbuffer,bufferSize,self->subDirs,self->options,NULL,&self->pathreq.overlapped,NULL)){
					uv_ref(self->pathreq.loop);
					self->pathref++;//since we've called uv_ref one time
					result=true;
				}else{
					free(self->pathbuffer);
					result=false;
				}
			}else{
				result=false;
			}
			return result;
		}
		static void finishWatchingParent(uv_work_t *req){
			HandleScope scope;
			dirWatcher* self=(dirWatcher*)req->data;
			if(req!=self->parentreq||self->parenthnd==NULL){//this is the request we need to realase and it is ready to be released now
				free(req);
				if(self->pathhnd==NULL){
					callJs(self,syb_evt_end,Null());
					self->Unref();
				}
			}else{
				void* buffer=self->parentbuffer;
				if(req->overlapped.Internal==ERROR_SUCCESS){
					FILE_NOTIFY_INFORMATION* pInfo;
					DWORD d=0;
					bool e=false;
					if(!beginWatchingParent(self)){
						e=true;
					}
					Handle<Value> oldname=splitPath(Handle<String>::Cast(self->definitions->Get(syb_path)))->Get(String::NewSymbol("name"));
					do{
						pInfo=(FILE_NOTIFY_INFORMATION*)((ULONG_PTR)buffer+d);
						Handle<String> filename=String::New((uint16_t*)pInfo->FileName,pInfo->FileNameLength/sizeof(wchar_t));
						if((pInfo->Action==FILE_ACTION_REMOVED||pInfo->Action==FILE_ACTION_RENAMED_OLD_NAME)&&(filename->StrictEquals(oldname)||filename->StrictEquals(self->definitions->Get(syb_shortName)))){
							Handle<String> newpath=getCurrentPathByHandle(self->pathhnd);
							self->definitions->Set(syb_path,newpath);
							self->definitions->Set(syb_shortName,splitPath(convertPath::js(newpath,false))->Get(String::NewSymbol("name")));
							if(pInfo->Action==FILE_ACTION_REMOVED){
								CloseHandle(self->parenthnd);
								if(!watchParent(self)){
									e=true;
								}
							}
							callJs(self,syb_evt_mov,newpath);
							break;//we've already got the new path
						}
						d+=pInfo->NextEntryOffset;
					}while(pInfo->NextEntryOffset>0);
					if(e){
						callJs(self,syb_evt_err,syb_err_unable_to_watch_parent);
					}
				}else{
					callJs(self,syb_evt_err,syb_err_unable_to_watch_parent);
					CloseHandle(self->parenthnd);
					self->parenthnd=NULL;
				}
				free(buffer);
			}
		}
		static void finishWatchingPath(uv_work_t *req){
			HandleScope scope;
			dirWatcher* self=(dirWatcher*)req->data;
			void* buffer=self->pathbuffer;
			self->pathref--;//uv_unref will be called when this function ends if there's no crash
			if(req->overlapped.Internal==ERROR_SUCCESS){
				FILE_NOTIFY_INFORMATION* pInfo;
				DWORD d=0;
				if(!beginWatchingPath(self)){
					callJs(self,syb_evt_err,syb_err_unable_to_continue_watching);
				}
				do{
					pInfo=(FILE_NOTIFY_INFORMATION*)((ULONG_PTR)buffer+d);
					Handle<String> filename=String::New((uint16_t*)pInfo->FileName,pInfo->FileNameLength/sizeof(wchar_t));
					if(pInfo->Action==FILE_ACTION_ADDED){
						callJs(self,syb_evt_new,filename);
					}else if(pInfo->Action==FILE_ACTION_REMOVED){
						callJs(self,syb_evt_del,filename);
					}else if(pInfo->Action==FILE_ACTION_MODIFIED){
						callJs(self,syb_evt_chg,filename);
					}else if(pInfo->Action==FILE_ACTION_RENAMED_OLD_NAME){
						if(self->definitions->HasOwnProperty(syb_evt_ren_newName)){
							Handle<Object> arg=Object::New();
							arg->Set(syb_evt_ren_oldName,filename);
							arg->Set(syb_evt_ren_newName,self->definitions->Get(syb_evt_ren_newName));
							self->definitions->Delete(syb_evt_ren_newName);
							callJs(self,syb_evt_ren,arg);
						}else{
							self->definitions->Set(syb_evt_ren_oldName,filename);
						}
					}else if(pInfo->Action==FILE_ACTION_RENAMED_NEW_NAME){
						if(self->definitions->HasOwnProperty(syb_evt_ren_oldName)){
							Handle<Object> arg=Object::New();
							arg->Set(syb_evt_ren_oldName,self->definitions->Get(syb_evt_ren_oldName));
							arg->Set(syb_evt_ren_newName,filename);
							self->definitions->Delete(syb_evt_ren_oldName);
							callJs(self,syb_evt_ren,arg);
						}else{
							self->definitions->Set(syb_evt_ren_newName,filename);
						}
					}
					d+=pInfo->NextEntryOffset;
				}while(pInfo->NextEntryOffset>0);
			}else{
				callJs(self,syb_evt_err,syb_err_unable_to_continue_watching);
				stopWatching(self);
			}
			free(buffer);
		}
		static void stopWatching(dirWatcher *self){
			if(self->pathhnd){
				CloseHandle(self->pathhnd);
				self->pathhnd=NULL;
			}
			if(self->pathref>0){
				self->pathreq.type=UV_UNKNOWN_REQ;//mute this request
				uv_unref(self->pathreq.loop);
				self->pathref--;
			}
			if(self->parenthnd){
				CloseHandle(self->parenthnd);
				self->parenthnd=NULL;
			}else{
				callJs(self,syb_evt_end,Null());
				self->Unref();
			}
		}
		static Handle<String> getCurrentPathByHandle(HANDLE hnd){
			HandleScope scope;
			size_t sz=GetFinalPathNameByHandleW(hnd,NULL,0,FILE_NAME_NORMALIZED);
			wchar_t* s=(wchar_t*)malloc(sizeof(wchar_t)*sz);
			wchar_t* s1=L"\\\\?\\UNC\\";//for network paths
			wchar_t* s2=L"\\\\?\\";//for local paths
			size_t sz1=wcslen(s1);
			size_t sz2=wcslen(s2);
			GetFinalPathNameByHandleW(hnd,s,sz,FILE_NAME_NORMALIZED);
			if(wcsncmp(s,s1,sz1)==0){
				sz=sz1-2;
				s[sz]=L'\\';
			}else if(wcsncmp(s,s2,sz2)==0&&s[sz2+1]==L':'){
				sz=wcslen(s2);
			}else{
				sz=0;
			}
			Handle<String> r=String::New((uint16_t*)(sz>0?&s[sz]:s));
			free(s);
			return scope.Close(r);
		}
		static void callJs(dirWatcher *self,Persistent<String> evt_type,Handle<Value> src){
			HandleScope scope;
			Handle<Value> arg[2]={evt_type->ToString(),src};
			Handle<Function> callback=Handle<Function>::Cast(self->definitions->Get(syb_callback));
			callback->Call(self->handle_,2,arg);
		}
		HANDLE pathhnd;
		HANDLE parenthnd;
		uv_work_t pathreq;
		uv_work_t *parentreq;
		DWORD pathref;
		DWORD options;
		BOOL subDirs;
		void *pathbuffer;
		void *parentbuffer;
		Persistent<Object> definitions;//to store glbal v8 types
		static const size_t bufferSize=64*1024;
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
	};
	const Persistent<String> dirWatcher::syb_path=NODE_PSYMBOL("path");
	const Persistent<String> dirWatcher::syb_shortName=NODE_PSYMBOL("shortName");
	const Persistent<String> dirWatcher::syb_callback=NODE_PSYMBOL("callback");
	const Persistent<String> dirWatcher::syb_opt_subDirs=NODE_PSYMBOL("subDirs");
	const Persistent<String> dirWatcher::syb_opt_fileSize=NODE_PSYMBOL("fileSize");
	const Persistent<String> dirWatcher::syb_opt_lastWrite=NODE_PSYMBOL("lastWrite");
	const Persistent<String> dirWatcher::syb_opt_lastAccess=NODE_PSYMBOL("lastAccess");
	const Persistent<String> dirWatcher::syb_opt_creation=NODE_PSYMBOL("creation");
	const Persistent<String> dirWatcher::syb_opt_attributes=NODE_PSYMBOL("attributes");
	const Persistent<String> dirWatcher::syb_opt_security=NODE_PSYMBOL("security");
	const Persistent<String> dirWatcher::syb_evt_sta=NODE_PSYMBOL("started");
	const Persistent<String> dirWatcher::syb_evt_end=NODE_PSYMBOL("ended");
	const Persistent<String> dirWatcher::syb_evt_new=NODE_PSYMBOL("added");
	const Persistent<String> dirWatcher::syb_evt_del=NODE_PSYMBOL("removed");
	const Persistent<String> dirWatcher::syb_evt_ren=NODE_PSYMBOL("renamed");
	const Persistent<String> dirWatcher::syb_evt_chg=NODE_PSYMBOL("modified");
	const Persistent<String> dirWatcher::syb_evt_mov=NODE_PSYMBOL("moved");
	const Persistent<String> dirWatcher::syb_evt_err=NODE_PSYMBOL("error");
	const Persistent<String> dirWatcher::syb_evt_ren_oldName=NODE_PSYMBOL("oldName");
	const Persistent<String> dirWatcher::syb_evt_ren_newName=NODE_PSYMBOL("newName");
	const Persistent<String> dirWatcher::syb_err_unable_to_watch_parent=NODE_PSYMBOL("UNABLE_TO_WATCH_PARENT");
	const Persistent<String> dirWatcher::syb_err_unable_to_continue_watching=NODE_PSYMBOL("UNABLE_TO_CONTINUE_WATCHING");
	const Persistent<String> dirWatcher::syb_err_initialization_failed=global_syb_err_initialization_failed;
	const Persistent<String> dirWatcher::syb_err_wrong_arguments=global_syb_err_wrong_arguments;

	extern "C"{
		static void init(Handle<Object> target){
			HandleScope scope;
			Handle<String> classname=String::NewSymbol("fsWin");
			target->Set(String::NewSymbol("dirWatcher"),dirWatcher::init(classname));
			NODE_SET_METHOD(target,"splitPath",splitPathForJs);
			NODE_SET_METHOD(target,"convertPath",convertPath::jsAsync);
			NODE_SET_METHOD(target,"convertPathSync",convertPath::jsSync);

			target->Set(String::NewSymbol("version"),String::NewSymbol(FSWIN_VERSION));
		}
		NODE_MODULE(fsWin,init);
	};
};