#define FSWIN_VERSION "0.1.2012.118"

#define UNICODE
#include <node.h>
#pragma comment(lib,"node.lib")
using namespace v8;
using namespace node;

//#include <iostream>//for debug only
//using namespace std;

//global constants are common messages that will be used in different classes to make syncing easier
static const PropertyAttribute global_syb_attr_const=(PropertyAttribute)(ReadOnly|DontDelete);
static const Persistent<String> global_syb_err_wrong_arguments=NODE_PSYMBOL("WRONG_ARGUMENTS");
static const Persistent<String> global_syb_err_not_a_constructor=NODE_PSYMBOL("THIS_FUNCTION_IS_NOT_A_CONSTRUCTOR");
static const Persistent<String> global_syb_err_initialization_failed=NODE_PSYMBOL("INITIALIZATION_FAILED");
static const Persistent<String> global_syb_evt_err=NODE_PSYMBOL("ERROR");
static const Persistent<String> global_syb_evt_end=NODE_PSYMBOL("ENDED");
static const Persistent<String> global_syb_evt_succeeded=NODE_PSYMBOL("SUCCEEDED");
static const Persistent<String> global_syb_evt_failed=NODE_PSYMBOL("FAILED");
static const Persistent<String> global_syb_fileAttr_isArchived=NODE_PSYMBOL("IS_ARCHIVED");
static const Persistent<String> global_syb_fileAttr_isHidden=NODE_PSYMBOL("IS_HIDDEN");
static const Persistent<String> global_syb_fileAttr_isNotContentIndexed=NODE_PSYMBOL("IS_NOT_CONTENT_INDEXED");
static const Persistent<String> global_syb_fileAttr_isOffline=NODE_PSYMBOL("IS_OFFLINE");
static const Persistent<String> global_syb_fileAttr_isReadOnly=NODE_PSYMBOL("IS_READ_ONLY");
static const Persistent<String> global_syb_fileAttr_isSystem=NODE_PSYMBOL("IS_SYSTEM");
static const Persistent<String> global_syb_fileAttr_isTemporary=NODE_PSYMBOL("IS_TEMPORARY");
	
//dirWatcher requires vista or latter to call GetFinalPathNameByHandleW.
//the API is necessary since the dir we are watching could also be moved to another path.
//and it is the only way to get the new path at that kind of situation.
//however, if you still need to use dirWatcher in winxp, it will work without watching
//the parent dir. and always fire an error at start up.
#ifndef GetFinalPathNameByHandle
typedef DWORD (WINAPI *GetFinalPathNameByHandle)(__in HANDLE hFile,__out_ecount(cchFilePath) LPWSTR lpszFilePath,__in DWORD cchFilePath,__in DWORD dwFlags);
static const GetFinalPathNameByHandle GetFinalPathNameByHandleW=(GetFinalPathNameByHandle)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "GetFinalPathNameByHandleW");
#endif//#ifndef GetFinalPathNameByHandle
static Handle<String> getCurrentPathByHandle(HANDLE hnd){
	HandleScope scope;
	Handle<String> r;
	if((void*)GetFinalPathNameByHandleW){//check if GetFinalPathNameByHandleW is supported
		size_t sz=GetFinalPathNameByHandleW(hnd,NULL,0,FILE_NAME_NORMALIZED);
		if(sz>0){
			wchar_t *s=(wchar_t*)malloc(sizeof(wchar_t)*sz);
			wchar_t *s1=L"\\\\?\\UNC\\";//for network paths
			wchar_t *s2=L"\\\\?\\";//for local paths
			size_t sz1=wcslen(s1);
			size_t sz2=wcslen(s2);
			GetFinalPathNameByHandleW(hnd,s,sz,FILE_NAME_NORMALIZED);
			if(wcsncmp(s,s1,sz1)==0){
				sz=sz1-2;
				s[sz]=L'\\';
			}else if(wcsncmp(s,s2,sz2)==0&&((s[sz2]>=L'a'&&s[sz2]<=L'z')||(s[sz2]>=L'A'&&s[sz2]<=L'Z'))&&s[sz2+1]==L':'){
				sz=wcslen(s2);
			}else{
				sz=0;
			}
			r=String::New((uint16_t*)(sz>0?&s[sz]:s));
			free(s);
		}
	}
	if(r.IsEmpty()){
		r=String::NewSymbol("");
	}
	return scope.Close(r);
}

static bool ensurePrivilege(const wchar_t *privilegeName){
	bool result=false;
	HANDLE hToken;
	if(OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,&hToken)){
		LUID tkid;
		if(LookupPrivilegeValueW(NULL,privilegeName,&tkid)){
			PRIVILEGE_SET ps;
			ps.PrivilegeCount=1;
			ps.Control=PRIVILEGE_SET_ALL_NECESSARY;
			ps.Privilege[0].Luid=tkid;
			ps.Privilege[0].Attributes=SE_PRIVILEGE_ENABLED;
			BOOL chkresult;
			if(PrivilegeCheck(hToken,&ps,&chkresult)){
				if(chkresult){
					result=true;
				}else{
					TOKEN_PRIVILEGES tp;
					tp.PrivilegeCount=1;
					tp.Privileges[0]=ps.Privilege[0];
					result=AdjustTokenPrivileges(hToken,FALSE,&tp,sizeof(tp),NULL,NULL)?true:false;
				}
			}
		}
	}
	CloseHandle(hToken);
	return result;
}
	
static ULONGLONG combineHiLow(const DWORD hi,const DWORD low){
	ULARGE_INTEGER ul;
	ul.HighPart=hi;
	ul.LowPart=low;
	return ul.QuadPart;
}

static double fileTimeToJsDateVal(const FILETIME *ft){//Date::New(fileTimeToJsDateVal(&filetime)) converts FILETIME to javascript date
	double ns=(double)combineHiLow(ft->dwHighDateTime,ft->dwLowDateTime);
	return ns/10000-11644473600000;
}

class find{
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
	static const Persistent<String> syb_reparsePoint_dfs;
	static const Persistent<String> syb_reparsePoint_dfsr;
	static const Persistent<String> syb_reparsePoint_hsm;
	static const Persistent<String> syb_reparsePoint_hsm2;
	static const Persistent<String> syb_reparsePoint_mountPoint;
	static const Persistent<String> syb_reparsePoint_sis;
	static const Persistent<String> syb_reparsePoint_symlink;
	static const struct resultData{//this is a linked table
		WIN32_FIND_DATAW data;
		resultData *next;
	};
	//progressive callback type, if this callback returns true, the search will stop immediately. the contents of info will be rewrited or released after the callback returns, so make a copy before starting a new thread if you still need to use it
	typedef bool (*findResultCall)(const WIN32_FIND_DATAW *info,void *data);
private:
	static const Persistent<String> syb_err_wrong_arguments;
	static const Persistent<String> syb_err_not_a_constructor;
	static const Persistent<String> syb_evt_found;
	static const Persistent<String> syb_evt_succeeded;
	static const Persistent<String> syb_evt_failed;
	static const Persistent<String> syb_evt_interrupted;
	static const struct jsCallbackData{
		Handle<Object> self;
		Handle<Function> func;
	};
	static const struct workdata{
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
	static resultData *basic(const wchar_t *path){//you have to delete every linked data yourself if it is not NULL
		resultData *result=new resultData;
		HANDLE hnd=FindFirstFileExW(path,FindExInfoStandard,&result->data,FindExSearchNameMatch,NULL,NULL);
		if(hnd==INVALID_HANDLE_VALUE){
			delete result;
			result=NULL;
		}else{
			resultData *resultnew,*resultold;
			if(isValidInfo(&result->data)){
				resultnew=new resultData;
				resultold=result;
			}else{
				resultnew=result;
				resultold=NULL;
				result=NULL;
			}
			while(FindNextFileW(hnd,&resultnew->data)){
				if(isValidInfo(&resultnew->data)){
					if(resultold){
						resultold->next=resultnew;
					}else{
						result=resultnew;
					}
					resultold=resultnew;
					resultnew=new resultData;
				}
			}
			resultold->next=NULL;
			FindClose(hnd);
			if(resultnew!=result){
				delete resultnew;
			}
		}
		return result;
	}
	static size_t basicWithCallback(const wchar_t *path,const findResultCall callback,void *data){//data could be anything that will directly pass to the callback
		WIN32_FIND_DATAW info;
		HANDLE hnd=FindFirstFileExW(path,FindExInfoStandard,&info,FindExSearchNameMatch,NULL,NULL);
		size_t result=0;
		bool stop=false;
		if(hnd!=INVALID_HANDLE_VALUE){
			if(isValidInfo(&info)){
				stop=callback(&info,data);
				result++;
			}
			while(!stop&&FindNextFileW(hnd,&info)){
				if(isValidInfo(&info)){
					stop=callback(&info,data);
					result++;
				}
			}
			FindClose(hnd);
		}
		return result;
	}
	static Handle<Object> fileInfoToJs(const WIN32_FIND_DATAW *info){//this function does not check whether info is NULL, make sure it is not before calling
		HandleScope scope;
		Handle<Object> o=Object::New();
		o->Set(syb_returns_longName,String::New((uint16_t*)info->cFileName));
		o->Set(syb_returns_shortName,String::New((uint16_t*)info->cAlternateFileName));
		o->Set(syb_returns_creationTime,Date::New(fileTimeToJsDateVal(&info->ftCreationTime)));
		o->Set(syb_returns_lastAccessTime,Date::New(fileTimeToJsDateVal(&info->ftLastAccessTime)));
		o->Set(syb_returns_lastWriteTime,Date::New(fileTimeToJsDateVal(&info->ftLastWriteTime)));
		o->Set(syb_returns_size,Number::New((double)combineHiLow(info->nFileSizeHigh,info->nFileSizeLow)));
		if(info->dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT){
			if(info->dwReserved0==IO_REPARSE_TAG_DFS){
				o->Set(syb_returns_isTemporary,syb_reparsePoint_dfs);
			}else if(info->dwReserved0==IO_REPARSE_TAG_DFSR){
				o->Set(syb_returns_isTemporary,syb_reparsePoint_dfsr);
			}else if(info->dwReserved0==IO_REPARSE_TAG_HSM){
				o->Set(syb_returns_isTemporary,syb_reparsePoint_hsm);
			}else if(info->dwReserved0==IO_REPARSE_TAG_HSM2){
				o->Set(syb_returns_isTemporary,syb_reparsePoint_hsm2);
			}else if(info->dwReserved0==IO_REPARSE_TAG_MOUNT_POINT){
				o->Set(syb_returns_isTemporary,syb_reparsePoint_mountPoint);
			}else if(info->dwReserved0==IO_REPARSE_TAG_SIS){
				o->Set(syb_returns_isTemporary,syb_reparsePoint_sis);
			}else if(info->dwReserved0==IO_REPARSE_TAG_SYMLINK){
				o->Set(syb_returns_isTemporary,syb_reparsePoint_symlink);
			}else{
				o->Set(syb_returns_isTemporary,syb_reparsePoint_unknown);
			}
		}else{
			o->Set(syb_returns_isTemporary,String::NewSymbol(""));
		}
		o->Set(syb_returns_isArchived,info->dwFileAttributes&FILE_ATTRIBUTE_ARCHIVE?True():False());
		o->Set(syb_returns_isCompressed,info->dwFileAttributes&FILE_ATTRIBUTE_COMPRESSED?True():False());
		o->Set(syb_returns_isDirectory,info->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY?True():False());
		o->Set(syb_returns_isEncrypted,info->dwFileAttributes&FILE_ATTRIBUTE_ENCRYPTED?True():False());
		o->Set(syb_returns_isHidden,info->dwFileAttributes&FILE_ATTRIBUTE_HIDDEN?True():False());
		o->Set(syb_returns_isNotContentIndexed,info->dwFileAttributes&FILE_ATTRIBUTE_NOT_CONTENT_INDEXED?True():False());
		o->Set(syb_returns_isOffline,info->dwFileAttributes&FILE_ATTRIBUTE_OFFLINE?True():False());
		o->Set(syb_returns_isReadOnly,info->dwFileAttributes&FILE_ATTRIBUTE_READONLY?True():False());
		o->Set(syb_returns_isSparseFile,info->dwFileAttributes&FILE_ATTRIBUTE_SPARSE_FILE?True():False());
		o->Set(syb_returns_isSystem,info->dwFileAttributes&FILE_ATTRIBUTE_SYSTEM?True():False());
		o->Set(syb_returns_isTemporary,info->dwFileAttributes&FILE_ATTRIBUTE_TEMPORARY?True():False());
		return scope.Close(o);
	}
	static Handle<Array> basicToJs(resultData *data){
		HandleScope scope;
		Handle<Array> a=Array::New();
		while(data){
			a->Set(a->Length(),fileInfoToJs(&data->data));
			resultData *old=data;
			data=old->next;
			delete old;
		}
		return scope.Close(a);
	}
	static Handle<Function> functionRegister(bool isAsyncVersion){
		HandleScope scope;
		Handle<FunctionTemplate> t=FunctionTemplate::New(isAsyncVersion?jsAsync:jsSync);

		//set error messages
		Handle<Object> errors=Object::New();
		errors->Set(syb_err_wrong_arguments,syb_err_wrong_arguments,global_syb_attr_const);
		errors->Set(syb_err_not_a_constructor,syb_err_not_a_constructor,global_syb_attr_const);
		t->Set(String::NewSymbol("errors"),errors,global_syb_attr_const);

		//set events
		if(isAsyncVersion){
			Handle<Object> events=Object::New();
			events->Set(syb_evt_found,syb_evt_found,global_syb_attr_const);
			events->Set(syb_evt_succeeded,syb_evt_succeeded,global_syb_attr_const);
			events->Set(syb_evt_failed,syb_evt_failed,global_syb_attr_const);
			events->Set(syb_evt_interrupted,syb_evt_interrupted,global_syb_attr_const);
			t->Set(String::NewSymbol("events"),events,global_syb_attr_const);
		}

		//set properties of return value
		Handle<Object> returns=Object::New();
		returns->Set(syb_returns_longName,syb_returns_longName,global_syb_attr_const);
		returns->Set(syb_returns_shortName,syb_returns_shortName,global_syb_attr_const);
		returns->Set(syb_returns_creationTime,syb_returns_creationTime,global_syb_attr_const);
		returns->Set(syb_returns_lastAccessTime,syb_returns_lastAccessTime,global_syb_attr_const);
		returns->Set(syb_returns_lastWriteTime,syb_returns_lastWriteTime,global_syb_attr_const);
		returns->Set(syb_returns_size,syb_returns_size,global_syb_attr_const);
		returns->Set(syb_returns_isArchived,syb_returns_isArchived,global_syb_attr_const);
		returns->Set(syb_returns_isCompressed,syb_returns_isCompressed,global_syb_attr_const);
		returns->Set(syb_returns_isDirectory,syb_returns_isDirectory,global_syb_attr_const);
		returns->Set(syb_returns_isEncrypted,syb_returns_isEncrypted,global_syb_attr_const);
		returns->Set(syb_returns_isHidden,syb_returns_isHidden,global_syb_attr_const);
		returns->Set(syb_returns_isNotContentIndexed,syb_returns_isNotContentIndexed,global_syb_attr_const);
		returns->Set(syb_returns_isOffline,syb_returns_isOffline,global_syb_attr_const);
		returns->Set(syb_returns_isReadOnly,syb_returns_isReadOnly,global_syb_attr_const);
		returns->Set(syb_returns_isSparseFile,syb_returns_isSparseFile,global_syb_attr_const);
		returns->Set(syb_returns_isSystem,syb_returns_isSystem,global_syb_attr_const);
		returns->Set(syb_returns_isTemporary,syb_returns_isTemporary,global_syb_attr_const);
		returns->Set(syb_returns_reparsePointTag,syb_returns_reparsePointTag,global_syb_attr_const);
		t->Set(String::NewSymbol("returns"),returns,global_syb_attr_const);

		return scope.Close(t->GetFunction());
	}
private:
	static bool isValidInfo(const WIN32_FIND_DATAW *info){//determine whether it is the real content 
		return wcscmp(info->cFileName,L".")!=0&&wcscmp(info->cFileName,L"..")!=0;
	}
	static Handle<Value> jsSync(const Arguments& args){
		HandleScope scope;
		Handle<Value> result;
		if(args.IsConstructCall()){
			result=ThrowException(Exception::Error(syb_err_not_a_constructor));
		}else{
			if(args.Length()>0&&(args[0]->IsString()||args[0]->IsStringObject())){
				String::Value spath(args[0]);
				if(args.Length()>1&&args[1]->IsFunction()){
					jsCallbackData callbackdata={args.This(),Local<Function>::Cast(args[1])};
					result=Integer::New(basicWithCallback((wchar_t*)*spath,jsSyncCallback,&callbackdata));
				}else{
					result=basicToJs(basic((wchar_t*)*spath));
				}
			}else{
				result=ThrowException(Exception::Error(syb_err_wrong_arguments));
			}
		}
		return scope.Close(result);
	}
	static bool jsSyncCallback(const WIN32_FIND_DATAW *info,void *data){
		HandleScope scope;
		Handle<Value> o=fileInfoToJs(info);
		jsCallbackData *d=(jsCallbackData*)data;
		return d->func->Call(d->self,1,&o)->ToBoolean()->IsTrue();
	}
	static Handle<Value> jsAsync(const Arguments& args){
		HandleScope scope;
		Handle<Value> result;
		if(args.IsConstructCall()){
			result=ThrowException(Exception::Error(syb_err_not_a_constructor));
		}else{
			if(args.Length()>1&&(args[0]->IsString()||args[0]->IsStringObject())&&args[1]->IsFunction()){
				workdata *data=new workdata;
				data->req.data=data;
				data->self=Persistent<Object>::New(args.This());
				data->func=Persistent<Function>::New(Handle<Function>::Cast(args[1]));
				String::Value spath(args[0]);
				data->data=_wcsdup((wchar_t*)*spath);
				if(args.Length()>2&&args[2]->ToBoolean()->IsTrue()){
					data->hnd=INVALID_HANDLE_VALUE;
					data->count=0;
					data->stop=false;
				}else{
					data->hnd=NULL;
				}
				if(uv_queue_work(uv_default_loop(),&data->req,beginWork,afterWork)==0){
					result=True();
				}else{
					free(data->data);
					data->self.Dispose();
					data->func.Dispose();
					delete data;
					result=False();
				}
			}else{
				result=ThrowException(Exception::Error(syb_err_wrong_arguments));
			}
		}
		return scope.Close(result);
	}
	static void beginWork(uv_work_t *req){
		workdata *data=(workdata*)req->data;
		if(data->hnd){
			WIN32_FIND_DATAW *info=new WIN32_FIND_DATAW;
			if(data->hnd==INVALID_HANDLE_VALUE){
				data->hnd=FindFirstFileExW((wchar_t*)data->data,FindExInfoStandard,info,FindExSearchNameMatch,NULL,NULL);
				free(data->data);
				if(data->hnd!=INVALID_HANDLE_VALUE){
					while(!isValidInfo(info)){
						if(!FindNextFileW(data->hnd,info)){
							FindClose(data->hnd);
							data->hnd=INVALID_HANDLE_VALUE;
							break;
						}
					}
				}
			}else{
				if(!data->stop){
					if(!FindNextFileW(data->hnd,info)){
						FindClose(data->hnd);
						data->hnd=INVALID_HANDLE_VALUE;
					}else{
						while(!isValidInfo(info)){
							if(!FindNextFileW(data->hnd,info)){
								FindClose(data->hnd);
								data->hnd=INVALID_HANDLE_VALUE;
								break;
							}
						}
					}
				}else{
					FindClose(data->hnd);
					data->hnd=INVALID_HANDLE_VALUE;
				}
			}
			if(data->hnd==INVALID_HANDLE_VALUE){
				delete info;
			}else{
				data->data=info;
			}
		}else{
			resultData *rdata=basic((wchar_t*)data->data);
			free(data->data);
			data->data=rdata;
		}
	}
	static void afterWork(uv_work_t *req){
		HandleScope scope;
		workdata *data=(workdata*)req->data;
		int del;
		if(data->hnd){
			Handle<Value> result[2];
			if(data->hnd==INVALID_HANDLE_VALUE){
				result[0]=syb_evt_succeeded;
				result[1]=Number::New((double)data->count);
				del=1;
			}else{
				WIN32_FIND_DATAW *info=(WIN32_FIND_DATAW*)data->data;
				if(data->stop){
					result[0]=data->stop?syb_evt_interrupted:syb_evt_succeeded;
					result[1]=Number::New((double)data->count);
					del=1;
				}else{
					data->count++;
					result[0]=syb_evt_found;
					result[1]=fileInfoToJs(info);
					del=uv_queue_work(uv_default_loop(),&data->req,beginWork,afterWork);
				}
				delete info;
			}
			data->stop=data->func->Call(data->self,2,result)->ToBoolean()->IsTrue();
		}else{
			Handle<Value> result;
			result=basicToJs((resultData*)data->data);
			del=1;
			data->func->Call(data->self,1,&result);
		}
		if(del){
			if(del!=1){
				Handle<Value> result[2];
				result[0]=syb_evt_failed;
				result[1]=Number::New((double)data->count);
				data->func->Call(data->self,2,result);
			}
			data->func.Dispose();
			data->self.Dispose();
			delete data;
		}
	}
};
const Persistent<String> find::syb_err_wrong_arguments=global_syb_err_wrong_arguments;
const Persistent<String> find::syb_err_not_a_constructor=global_syb_err_not_a_constructor;
const Persistent<String> find::syb_evt_found=NODE_PSYMBOL("FOUND");
const Persistent<String> find::syb_evt_succeeded=NODE_PSYMBOL("SUCCEEDED");
const Persistent<String> find::syb_evt_failed=NODE_PSYMBOL("FAILED");
const Persistent<String> find::syb_evt_interrupted=NODE_PSYMBOL("INTERRUPTED");
const Persistent<String> find::syb_returns_longName=NODE_PSYMBOL("LONG_NAME");
const Persistent<String> find::syb_returns_shortName=NODE_PSYMBOL("SHORT_NAME");
const Persistent<String> find::syb_returns_creationTime=NODE_PSYMBOL("CREATION_TIME");
const Persistent<String> find::syb_returns_lastAccessTime=NODE_PSYMBOL("LAST_ACCESS_TIME");
const Persistent<String> find::syb_returns_lastWriteTime=NODE_PSYMBOL("LAST_WRITE_TIME");
const Persistent<String> find::syb_returns_size=NODE_PSYMBOL("SIZE");
const Persistent<String> find::syb_returns_reparsePointTag=NODE_PSYMBOL("REPARSE_POINT_TAG");
const Persistent<String> find::syb_returns_isDirectory=NODE_PSYMBOL("IS_DIRECTORY");
const Persistent<String> find::syb_returns_isCompressed=NODE_PSYMBOL("IS_COMPRESSED");
const Persistent<String> find::syb_returns_isEncrypted=NODE_PSYMBOL("IS_ENCRYPTED");
const Persistent<String> find::syb_returns_isSparseFile=NODE_PSYMBOL("IS_SPARSE_FILE");
const Persistent<String> find::syb_returns_isArchived=global_syb_fileAttr_isArchived;
const Persistent<String> find::syb_returns_isHidden=global_syb_fileAttr_isHidden;
const Persistent<String> find::syb_returns_isNotContentIndexed=global_syb_fileAttr_isNotContentIndexed;
const Persistent<String> find::syb_returns_isOffline=global_syb_fileAttr_isOffline;
const Persistent<String> find::syb_returns_isReadOnly=global_syb_fileAttr_isReadOnly;
const Persistent<String> find::syb_returns_isSystem=global_syb_fileAttr_isSystem;
const Persistent<String> find::syb_returns_isTemporary=global_syb_fileAttr_isTemporary;
const Persistent<String> find::syb_reparsePoint_unknown=NODE_PSYMBOL("UNKNOWN");
const Persistent<String> find::syb_reparsePoint_dfs=NODE_PSYMBOL("DFS");
const Persistent<String> find::syb_reparsePoint_dfsr=NODE_PSYMBOL("DFSR");
const Persistent<String> find::syb_reparsePoint_hsm=NODE_PSYMBOL("HSM");
const Persistent<String> find::syb_reparsePoint_hsm2=NODE_PSYMBOL("HSM2");
const Persistent<String> find::syb_reparsePoint_mountPoint=NODE_PSYMBOL("MOUNT_POINT");
const Persistent<String> find::syb_reparsePoint_sis=NODE_PSYMBOL("SIS");
const Persistent<String> find::syb_reparsePoint_symlink=NODE_PSYMBOL("SYMLINK");

class splitPath{
public:
	static const Persistent<String> syb_return_parent;
	static const Persistent<String> syb_return_name;
	static const struct splitedPath{
		size_t parentLen;//the length of the parent
		const wchar_t *name;//this could be also considered as the start position of the name
	};
private:
	static const Persistent<String> syb_err_wrong_arguments;
	static const Persistent<String> syb_err_not_a_constructor;
public:
	static splitedPath *basic(const wchar_t *path){//you need to delete the return value your self if it is not NULL;
		wchar_t *s=L"\\\\",s1=L'\\';
		size_t i,j=0,k=0,l=wcslen(path),m=wcslen(s);
		if(wcsncmp(s,path,m)==0){//is network path
			for(i=m+1;i<l-1;i++){
				if(path[i]==s1){
					if(++k==2){
						j=i+1;
						break;
					}
				}
			}
			if(k==2){
				k=0;
				for(i=l-2;i>j+1;i--){
					if(path[i]==s1){
						j=i;
						k=1;
						break;
					}
				}
			}else{
				k=0;
			}
		}else{//is local path
			for(i=l-2;i>1;i--){
				if(path[i]==s1){
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
		splitedPath *r=new splitedPath;
		r->parentLen=j;
		r->name=&path[j>0?j+k:j];
		return r;
	}
	//this function returns an Object with two properties: PARENT and NAME
	//the parent property could be empty if path is a rootdir
	static Handle<Object> js(Handle<String> path){
		HandleScope scope;
		String::Value p1(path);
		splitedPath *s=basic((wchar_t*)*p1);
		Handle<Object> r=Object::New();
		r->Set(syb_return_parent,String::New(*p1,s->parentLen));
		r->Set(syb_return_name,String::New((uint16_t*)s->name));
		delete s;
		return scope.Close(r);
	}
	static Handle<Function> functionRegister(){
		HandleScope scope;
		Handle<FunctionTemplate> t=FunctionTemplate::New(jsSync);

		//set errmessages
		Handle<Object> errors=Object::New();
		errors->Set(syb_err_wrong_arguments,syb_err_wrong_arguments,global_syb_attr_const);
		errors->Set(syb_err_not_a_constructor,syb_err_not_a_constructor,global_syb_attr_const);
		t->Set(String::NewSymbol("errors"),errors,global_syb_attr_const);

		//set properties of the return value
		Handle<Object> returns=Object::New();
		returns->Set(syb_return_parent,syb_return_parent,global_syb_attr_const);
		returns->Set(syb_return_name,syb_return_name,global_syb_attr_const);
		t->Set(String::NewSymbol("returns"),returns,global_syb_attr_const);

		return scope.Close(t->GetFunction());
	}
private:
	static Handle<Value> jsSync(const Arguments& args){
		HandleScope scope;
		Handle<Value> result;
		if(args.IsConstructCall()){
			result=ThrowException(Exception::Error(syb_err_not_a_constructor));
		}else{
			if(args.Length()>0&&(args[0]->IsString()||args[0]->IsStringObject())){
				result=js(Handle<String>::Cast(args[0]));
			}else{
				result=ThrowException(Exception::Error(syb_err_wrong_arguments));
			}
		}
		return scope.Close(result);
	}
};
const Persistent<String> splitPath::syb_return_parent=NODE_PSYMBOL("PARENT");
const Persistent<String> splitPath::syb_return_name=NODE_PSYMBOL("NAME");
const Persistent<String> splitPath::syb_err_wrong_arguments=global_syb_err_wrong_arguments;
const Persistent<String> splitPath::syb_err_not_a_constructor=global_syb_err_not_a_constructor;

class setShortName{
private:
	static const Persistent<String> syb_err_wrong_arguments;
	static const Persistent<String> syb_err_not_a_constructor;
	static const struct workdata{
		uv_work_t req;
		Persistent<Object> self;
		Persistent<Function> func;
		wchar_t *path;
		wchar_t *newname;
		bool result;
	};
public:
	static bool basic(const wchar_t *path,const wchar_t *newname){
		bool result=false;
		if(ensurePrivilege(SE_RESTORE_NAME)){//make sure the process has SE_RESTORE_NAME privilege
			HANDLE hnd=CreateFileW(path,GENERIC_WRITE|DELETE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,NULL);
			if(hnd==INVALID_HANDLE_VALUE){
				result=false;
			}else{
				result=SetFileShortNameW(hnd,newname?newname:L"")?true:false;
				CloseHandle(hnd);
			}
		}else{
			result=false;
		}
		return result;
	}
	static Handle<Boolean> js(const Handle<String> path,const Handle<String> newname){
		HandleScope scope;
		String::Value p(path);
		String::Value n(newname);
		Handle<Boolean> result=basic((wchar_t*)*p,(wchar_t*)*n)?True():False();
		return scope.Close(result);
	}
	static Handle<Function> functionRegister(bool isAsyncVersion){
		HandleScope scope;
		Handle<FunctionTemplate> t=FunctionTemplate::New(isAsyncVersion?jsAsync:jsSync);

		//set errmessages
		Handle<Object> errors=Object::New();
		errors->Set(syb_err_wrong_arguments,syb_err_wrong_arguments,global_syb_attr_const);
		errors->Set(syb_err_not_a_constructor,syb_err_not_a_constructor,global_syb_attr_const);
		t->Set(String::NewSymbol("errors"),errors,global_syb_attr_const);

		return scope.Close(t->GetFunction());
	}
private:
	static Handle<Value> jsSync(const Arguments& args){
		HandleScope scope;
		Handle<Value> result;
		if(args.IsConstructCall()){
			result=ThrowException(Exception::Error(syb_err_not_a_constructor));
		}else{
			if(args.Length()>0&&(args[0]->IsString()||args[0]->IsStringObject())){
				Handle<String> newname;
				if(args[1]->IsString()||args[1]->IsStringObject()){
					newname=Handle<String>::Cast(args[1]);
				}else{
					newname=String::NewSymbol("");
				}
				result=js(Handle<String>::Cast(args[0]),newname);
			}else{
				result=ThrowException(Exception::Error(syb_err_wrong_arguments));
			}
		}
		return scope.Close(result);
	}
	static Handle<Value> jsAsync(const Arguments& args){
		HandleScope scope;
		Handle<Value> result;
		if(args.IsConstructCall()){
			result=ThrowException(Exception::Error(syb_err_not_a_constructor));
		}else{
			if(args.Length()>0&&(args[0]->IsString()||args[0]->IsStringObject())){
				workdata *data=new workdata;
				data->req.data=data;
				data->self=Persistent<Object>::New(args.This());
				if(args.Length()>1){
					if(args[1]->IsString()||args[1]->IsStringObject()){
						String::Value n(Local<String>::Cast(args[1]));
						data->newname=_wcsdup((wchar_t*)*n);
						if(args.Length()>2&&args[2]->IsFunction()){
							data->func=Persistent<Function>::New(Handle<Function>::Cast(args[2]));
						}
					}else if(args[1]->IsFunction()){
						data->newname=NULL;
						data->func=Persistent<Function>::New(Handle<Function>::Cast(args[1]));
					}else{
						data->newname=NULL;
					}
				}
				String::Value p(Local<String>::Cast(args[0]));
				data->path=_wcsdup((wchar_t*)*p);
				if(uv_queue_work(uv_default_loop(),&data->req,beginWork,afterWork)==0){
					result=True();
				}else{
					free(data->path);
					if(data->newname){
						free(data->newname);
					}
					if(!data->func.IsEmpty()){
						data->func.Dispose();
					}
					data->self.Dispose();
					delete data;
					result=False();
				}
			}
		}
		return scope.Close(result);
	}
	static void beginWork(uv_work_t *req){
		workdata *data=(workdata*)req->data;
		data->result=basic(data->path,data->newname);
		free(data->path);
		if(data->newname){
			free(data->newname);
		}
	}
	static void afterWork(uv_work_t *req){
		HandleScope scope;
		workdata *data=(workdata*)req->data;
		Handle<Value> p=data->result?True():False();
		if(!data->func.IsEmpty()){
			data->func->Call(data->self,1,&p);
			data->func.Dispose();
		}
		data->self.Dispose();
		delete data;
	}
};
const Persistent<String> setShortName::syb_err_wrong_arguments=global_syb_err_wrong_arguments;
const Persistent<String> setShortName::syb_err_not_a_constructor=global_syb_err_not_a_constructor;

class setCompression{
public:
	typedef void (*callbackFunc)(const bool result,void *data);
private:
	static const Persistent<String> syb_err_wrong_arguments;
	static const Persistent<String> syb_err_not_a_constructor;
	static const struct workdata{
		Persistent<Object> self;
		Persistent<Function> func;
	};
	static const struct workdata2{
		callbackFunc callback;
		void *data;
		HANDLE hnd;
	};
public:
	static bool basic(const wchar_t *path,const bool compress){
		bool result=false;
		HANDLE hnd=CreateFileW(path,FILE_GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,NULL);
		if(hnd!=INVALID_HANDLE_VALUE){
			USHORT c=compress?COMPRESSION_FORMAT_DEFAULT:COMPRESSION_FORMAT_NONE;
			DWORD d;
			if(DeviceIoControl(hnd,FSCTL_SET_COMPRESSION,&c,sizeof(USHORT),NULL,0,&d,NULL)){
				result=true;
			}
			CloseHandle(hnd);
		}
		return result;
	}
	static bool basicWithCallback(const wchar_t *path,const bool compress,callbackFunc callback,void *data){
		bool result=false;
		workdata2 *work=new workdata2;
		work->callback=callback;
		work->data=data;
		work->hnd=CreateFileW(path,FILE_GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OVERLAPPED,NULL);
		if(work->hnd==INVALID_HANDLE_VALUE){
			delete work;
		}else{
			uv_work_t *req=new uv_work_t;
			memset(req,0,sizeof(uv_work_t));
			req->loop=uv_default_loop();
			req->type=UV_WORK;
			req->after_work_cb=afterWork;
			req->data=work;
			if(CreateIoCompletionPort(work->hnd,req->loop->iocp,(ULONG_PTR)req,0)){
				USHORT c=compress?COMPRESSION_FORMAT_DEFAULT:COMPRESSION_FORMAT_NONE;
				DeviceIoControl(work->hnd,FSCTL_SET_COMPRESSION,&c,sizeof(USHORT),NULL,0,NULL,&req->overlapped);
				if(GetLastError()==ERROR_IO_PENDING){
					uv_ref(req->loop);
					result=true;
				}else{
					CloseHandle(work->hnd);
					delete req;
					delete work;
				}
			}
		}
		return result;
	}
	static Handle<Function> functionRegister(bool isAsyncVersion){
		HandleScope scope;
		Handle<FunctionTemplate> t=FunctionTemplate::New(isAsyncVersion?jsAsync:jsSync);

		//set errmessages
		Handle<Object> errors=Object::New();
		errors->Set(syb_err_wrong_arguments,syb_err_wrong_arguments,global_syb_attr_const);
		errors->Set(syb_err_not_a_constructor,syb_err_not_a_constructor,global_syb_attr_const);
		t->Set(String::NewSymbol("errors"),errors,global_syb_attr_const);

		return scope.Close(t->GetFunction());
	}
private:
	static void afterWork(uv_work_t *req){
		workdata2 *work=(workdata2*)req->data;
		CloseHandle(work->hnd);
		delete req;
		work->callback(req->overlapped.Internal==ERROR_SUCCESS,work->data);
		delete work;
	}
	static Handle<Value> jsSync(const Arguments& args){
		HandleScope scope;
		Handle<Value> result;
		if(args.IsConstructCall()){
			result=ThrowException(Exception::Error(syb_err_not_a_constructor));
		}else{
			if(args.Length()>0&&(args[0]->IsString()||args[0]->IsStringObject())){
				String::Value spath(args[0]);
				result=basic((wchar_t*)*spath,args[1]->ToBoolean()->IsTrue())?True():False();
			}else{
				result=ThrowException(Exception::Error(syb_err_wrong_arguments));
			}
		}
		return scope.Close(result);
	}
	static Handle<Value> jsAsync(const Arguments& args){
		HandleScope scope;
		Handle<Value> result;
		if(args.IsConstructCall()){
			result=ThrowException(Exception::Error(syb_err_not_a_constructor));
		}else{
			if(args.Length()>0&&(args[0]->IsString()||args[0]->IsStringObject())){
				workdata *data=NULL;
				bool b;
				if(args.Length()>1){
					Handle<Function> f;
					if(args[1]->IsFunction()){
						f=Handle<Function>::Cast(args[1]);
						b=args[2]->ToBoolean()->IsTrue();
					}else{
						b=args[1]->ToBoolean()->IsTrue();
					}
					if(!f.IsEmpty()){
						data=new workdata;
						data->self=Persistent<Object>::New(args.This());
						data->func=Persistent<Function>::New(f);
					}
				}else{
					b=false;
				}
				String::Value p(args[0]);
				if(basicWithCallback((wchar_t*)*p,b,asyncCallback,data)){
					result=True();
				}else{
					if(data){
						data->self.Dispose();
						data->func.Dispose();
						delete data;
					}
					result=False();
				}
			}else{
				result=ThrowException(Exception::Error(syb_err_wrong_arguments));
			}
		}
		return scope.Close(result);
	}
	static void asyncCallback(const bool succeeded,void *data){
		if(data){
			workdata *work=(workdata*)data;
			Handle<Value> r=succeeded?True():False();
			work->func->Call(work->self,1,&r);
			work->func.Dispose();
			work->self.Dispose();
			delete work;
		}
	}
};
const Persistent<String> setCompression::syb_err_wrong_arguments=global_syb_err_wrong_arguments;
const Persistent<String> setCompression::syb_err_not_a_constructor=global_syb_err_not_a_constructor;

class getCompressedSize{
private:
	static const Persistent<String> syb_err_wrong_arguments;
	static const Persistent<String> syb_err_not_a_constructor;
	static const struct workdata{
		uv_work_t req;
		Persistent<Object> self;
		Persistent<Function> func;
		wchar_t *path;
		ULONGLONG result;
	};
public:
	static ULONGLONG basic(const wchar_t *path){
		ULARGE_INTEGER u;
		u.LowPart=GetCompressedFileSizeW(path,&u.HighPart);
		if(u.LowPart==INVALID_FILE_SIZE&&GetLastError()!=NO_ERROR){
			u.QuadPart=0;
		}
		return u.QuadPart;
	}
	static Handle<Function> functionRegister(bool isAsyncVersion){
		HandleScope scope;
		Handle<FunctionTemplate> t=FunctionTemplate::New(isAsyncVersion?jsAsync:jsSync);

		//set errmessages
		Handle<Object> errors=Object::New();
		errors->Set(syb_err_wrong_arguments,syb_err_wrong_arguments,global_syb_attr_const);
		errors->Set(syb_err_not_a_constructor,syb_err_not_a_constructor,global_syb_attr_const);
		t->Set(String::NewSymbol("errors"),errors,global_syb_attr_const);

		return scope.Close(t->GetFunction());
	}
private:
	static Handle<Value> jsSync(const Arguments& args){
		HandleScope scope;
		Handle<Value> result;
		if(args.IsConstructCall()){
			result=ThrowException(Exception::Error(syb_err_not_a_constructor));
		}else{
			if(args.Length()>0&&(args[0]->IsString()||args[0]->IsStringObject())){
				String::Value spath(args[0]);
				result=Number::New((double)basic((wchar_t*)*spath));
			}else{
				result=ThrowException(Exception::Error(syb_err_wrong_arguments));
			}
		}
		return scope.Close(result);
	}
	static Handle<Value> jsAsync(const Arguments& args){
		HandleScope scope;
		Handle<Value> result;
		if(args.IsConstructCall()){
			result=ThrowException(Exception::Error(syb_err_not_a_constructor));
		}else{
			if(args.Length()>1&&(args[0]->IsString()||args[0]->IsStringObject())&&args[1]->IsFunction()){
				workdata *data=new workdata;
				data->req.data=data;
				data->self=Persistent<Object>::New(args.This());
				data->func=Persistent<Function>::New(Handle<Function>::Cast(args[1]));
				String::Value p(args[0]);
				data->path=_wcsdup((wchar_t*)*p);
				if(uv_queue_work(uv_default_loop(),&data->req,beginWork,afterWork)==0){
					result=True();
				}else{
					free(data->path);
					data->self.Dispose();
					data->func.Dispose();
					delete data;
					result=False();
				}
			}else{
				result=ThrowException(Exception::Error(syb_err_wrong_arguments));
			}
		}
		return scope.Close(result);
	}
	static void beginWork(uv_work_t *req){
		workdata *data=(workdata*)req->data;
		data->result=basic(data->path);
		free(data->path);
	}
	static void afterWork(uv_work_t *req){
		HandleScope scope;
		workdata *data=(workdata*)req->data;
		Handle<Value> p=Number::New((double)data->result);
		data->func->Call(data->self,1,&p);
		data->func.Dispose();
		data->self.Dispose();
		delete data;
	}
};
const Persistent<String> getCompressedSize::syb_err_wrong_arguments=global_syb_err_wrong_arguments;
const Persistent<String> getCompressedSize::syb_err_not_a_constructor=global_syb_err_not_a_constructor;
	
class getVolumeSpace{
public:
	static const Persistent<String> syb_returns_totalSpace;
	static const Persistent<String> syb_returns_freeSpace;
	static const struct spaces{
		ULONGLONG totalSpace;
		ULONGLONG freeSpace;
	};
private:
	static const Persistent<String> syb_err_wrong_arguments;
	static const Persistent<String> syb_err_not_a_constructor;
	static const struct workdata{
		uv_work_t req;
		Persistent<Object> self;
		Persistent<Function> func;
		void *path;
	};
public:
	static spaces *basic(const wchar_t *path){//you need to delete the result yourself if it is not NULL
		ULARGE_INTEGER u1;
		ULARGE_INTEGER u2;
		spaces *result;
		if(GetDiskFreeSpaceExW(path,&u1,&u2,NULL)){
			result=new spaces;
			result->freeSpace=u1.QuadPart;
			result->totalSpace=u2.QuadPart;
		}else{
			result=NULL;
		}
		return result;
	}
	static Handle<Object> spacesToJs(const spaces *spc){//this function will delete the param if it is not NULL
		HandleScope scope;
		Handle<Object> result;
		if(spc){
			result=Object::New();
			result->Set(syb_returns_totalSpace,Number::New((double)spc->totalSpace));
			result->Set(syb_returns_freeSpace,Number::New((double)spc->freeSpace));
			delete spc;
		}
		return scope.Close(result);
	}
	static Handle<Function> functionRegister(bool isAsyncVersion){
		HandleScope scope;
		Handle<FunctionTemplate> t=FunctionTemplate::New(isAsyncVersion?jsAsync:jsSync);

		//set error messages
		Handle<Object> errors=Object::New();
		errors->Set(syb_err_wrong_arguments,syb_err_wrong_arguments,global_syb_attr_const);
		errors->Set(syb_err_not_a_constructor,syb_err_not_a_constructor,global_syb_attr_const);
		t->Set(String::NewSymbol("errors"),errors,global_syb_attr_const);
			
		//set return values
		Handle<Object> returns=Object::New();
		returns->Set(syb_returns_totalSpace,syb_returns_totalSpace,global_syb_attr_const);
		returns->Set(syb_returns_freeSpace,syb_returns_freeSpace,global_syb_attr_const);
		t->Set(String::NewSymbol("returns"),errors,global_syb_attr_const);

		return scope.Close(t->GetFunction());
	}
private:
	static Handle<Value> jsSync(const Arguments& args){
		HandleScope scope;
		Handle<Value> result;
		if(args.IsConstructCall()){
			result=ThrowException(Exception::Error(syb_err_not_a_constructor));
		}else{
			if(args.Length()>0&&(args[0]->IsString()||args[0]->IsStringObject())){
				String::Value spath(args[0]);
				result=spacesToJs(basic((wchar_t*)*spath));
			}else{
				result=ThrowException(Exception::Error(syb_err_wrong_arguments));
			}
		}
		return scope.Close(result);
	}
	static Handle<Value> jsAsync(const Arguments& args){
		HandleScope scope;
		Handle<Value> result;
		if(args.IsConstructCall()){
			result=ThrowException(Exception::Error(syb_err_not_a_constructor));
		}else{
			if(args.Length()>1&&(args[0]->IsString()||args[0]->IsStringObject())&&args[1]->IsFunction()){
				workdata *data=new workdata;
				data->req.data=data;
				data->self=Persistent<Object>::New(args.This());
				data->func=Persistent<Function>::New(Handle<Function>::Cast(args[1]));
				String::Value p(args[0]);
				data->path=_wcsdup((wchar_t*)*p);
				if(uv_queue_work(uv_default_loop(),&data->req,beginWork,afterWork)==0){
					result=True();
				}else{
					free(data->path);
					data->self.Dispose();
					data->func.Dispose();
					delete data;
					result=False();
				}
			}else{
				result=ThrowException(Exception::Error(syb_err_wrong_arguments));
			}
		}
		return scope.Close(result);
	}
	static void beginWork(uv_work_t *req){
		workdata *data=(workdata*)req->data;
		spaces *p=basic((wchar_t*)data->path);
		free(data->path);
		data->path=p;
	}
	static void afterWork(uv_work_t *req){
		HandleScope scope;
		workdata *data=(workdata*)req->data;
		Handle<Value> p=spacesToJs((spaces*)data->path);
		data->func->Call(data->self,1,&p);
		data->func.Dispose();
		data->self.Dispose();
		delete data;
	}
};
const Persistent<String> getVolumeSpace::syb_err_wrong_arguments=global_syb_err_wrong_arguments;
const Persistent<String> getVolumeSpace::syb_err_not_a_constructor=global_syb_err_not_a_constructor;
const Persistent<String> getVolumeSpace::syb_returns_totalSpace=NODE_PSYMBOL("TOTAL");
const Persistent<String> getVolumeSpace::syb_returns_freeSpace=NODE_PSYMBOL("FREE");

class setAttributes{
public:
	static const struct attrVal{//0=keep,1=yes,-1=no
		char archive;
		char hidden;
		char notContentIndexed;
		char offline;
		char readonly;
		char system;
		char temporary;
	};
private:
	static const Persistent<String> syb_param_attr_archive;
	static const Persistent<String> syb_param_attr_hidden;
	static const Persistent<String> syb_param_attr_notContentIndexed;
	static const Persistent<String> syb_param_attr_offline;
	static const Persistent<String> syb_param_attr_readonly;
	static const Persistent<String> syb_param_attr_system;
	static const Persistent<String> syb_param_attr_temporary;
	static const Persistent<String> syb_err_wrong_arguments;
	static const Persistent<String> syb_err_not_a_constructor;
	static const struct workdata{
		uv_work_t req;
		Persistent<Object> self;
		Persistent<Function> func;
		wchar_t *path;
		attrVal *attr;
		bool result;
	};
public:
	static bool basic(const wchar_t *file,const attrVal *attr){
		bool result;
		DWORD oldattr=GetFileAttributesW(file);
		if(oldattr==INVALID_FILE_ATTRIBUTES){
			result=false;
		}else{
			DWORD newattr=oldattr;
			if(attr->archive<0){
				if(oldattr&FILE_ATTRIBUTE_ARCHIVE){
					newattr^=FILE_ATTRIBUTE_ARCHIVE;
				}
			}else if(attr->archive>0){
				if(!(oldattr&FILE_ATTRIBUTE_ARCHIVE)){
					newattr|=FILE_ATTRIBUTE_ARCHIVE;
				}
			}
			if(attr->hidden<0){
				if(oldattr&FILE_ATTRIBUTE_HIDDEN){
					newattr^=FILE_ATTRIBUTE_HIDDEN;
				}
			}else if(attr->hidden>0){
				if(!(oldattr&FILE_ATTRIBUTE_HIDDEN)){
					newattr|=FILE_ATTRIBUTE_HIDDEN;
				}
			}
			if(attr->notContentIndexed<0){
				if(oldattr&FILE_ATTRIBUTE_NOT_CONTENT_INDEXED){
					newattr^=FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
				}
			}else if(attr->notContentIndexed>0){
				if(!(oldattr&FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)){
					newattr|=FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
				}
			}
			if(attr->readonly<0){
				if(oldattr&FILE_ATTRIBUTE_OFFLINE){
					newattr^=FILE_ATTRIBUTE_OFFLINE;
				}
			}else if(attr->readonly>0){
				if(!(oldattr&FILE_ATTRIBUTE_OFFLINE)){
					newattr|=FILE_ATTRIBUTE_OFFLINE;
				}
			}
			if(attr->archive<0){
				if(oldattr&FILE_ATTRIBUTE_READONLY){
					newattr^=FILE_ATTRIBUTE_READONLY;
				}
			}else if(attr->archive>0){
				if(!(oldattr&FILE_ATTRIBUTE_READONLY)){
					newattr|=FILE_ATTRIBUTE_READONLY;
				}
			}
			if(attr->system<0){
				if(oldattr&FILE_ATTRIBUTE_SYSTEM){
					newattr^=FILE_ATTRIBUTE_SYSTEM;
				}
			}else if(attr->system>0){
				if(!(oldattr&FILE_ATTRIBUTE_SYSTEM)){
					newattr|=FILE_ATTRIBUTE_SYSTEM;
				}
			}
			if(attr->temporary<0){
				if(oldattr&FILE_ATTRIBUTE_TEMPORARY){
					newattr^=FILE_ATTRIBUTE_TEMPORARY;
				}
			}else if(attr->temporary>0){
				if(!(oldattr&FILE_ATTRIBUTE_TEMPORARY)){
					newattr|=FILE_ATTRIBUTE_TEMPORARY;
				}
			}
			if(newattr==oldattr){
				result=true;
			}else{
				result=SetFileAttributesW(file,newattr)?true:false;
			}
		}
		return result;
	}
	static attrVal *jsToAttrval(Handle<Object> attr){//delete the result if it is not NULL
		HandleScope scope;
		attrVal *a=new attrVal;
		if(attr->HasOwnProperty(syb_param_attr_archive)){
			a->archive=attr->Get(syb_param_attr_archive)->ToBoolean()->IsTrue()?1:-1;
		}else{
			a->archive=0;
		}
		if(attr->HasOwnProperty(syb_param_attr_hidden)){
			a->hidden=attr->Get(syb_param_attr_hidden)->ToBoolean()->IsTrue()?1:-1;
		}else{
			a->hidden=0;
		}
		if(attr->HasOwnProperty(syb_param_attr_notContentIndexed)){
			a->notContentIndexed=attr->Get(syb_param_attr_notContentIndexed)->ToBoolean()->IsTrue()?1:-1;
		}else{
			a->notContentIndexed=0;
		}
		if(attr->HasOwnProperty(syb_param_attr_offline)){
			a->offline=attr->Get(syb_param_attr_offline)->ToBoolean()->IsTrue()?1:-1;
		}else{
			a->offline=0;
		}
		if(attr->HasOwnProperty(syb_param_attr_readonly)){
			a->readonly=attr->Get(syb_param_attr_readonly)->ToBoolean()->IsTrue()?1:-1;
		}else{
			a->readonly=0;
		}
		if(attr->HasOwnProperty(syb_param_attr_system)){
			a->system=attr->Get(syb_param_attr_system)->ToBoolean()->IsTrue()?1:-1;
		}else{
			a->system=0;
		}
		if(attr->HasOwnProperty(syb_param_attr_temporary)){
			a->temporary=attr->Get(syb_param_attr_temporary)->ToBoolean()->IsTrue()?1:-1;
		}else{
			a->temporary=0;
		}
		return a;
	}
	static Handle<Function> functionRegister(bool isAsyncVersion){
		HandleScope scope;
		Handle<FunctionTemplate> t=FunctionTemplate::New(isAsyncVersion?jsAsync:jsSync);

		//set errmessages
		Handle<Object> errors=Object::New();
		errors->Set(syb_err_wrong_arguments,syb_err_wrong_arguments,global_syb_attr_const);
		errors->Set(syb_err_not_a_constructor,syb_err_not_a_constructor,global_syb_attr_const);
		t->Set(String::NewSymbol("errors"),errors,global_syb_attr_const);
			
		//set params
		Handle<Object> params=Object::New();
		params->Set(syb_param_attr_archive,syb_param_attr_archive,global_syb_attr_const);
		params->Set(syb_param_attr_hidden,syb_param_attr_hidden,global_syb_attr_const);
		params->Set(syb_param_attr_notContentIndexed,syb_param_attr_notContentIndexed,global_syb_attr_const);
		params->Set(syb_param_attr_offline,syb_param_attr_offline,global_syb_attr_const);
		params->Set(syb_param_attr_readonly,syb_param_attr_readonly,global_syb_attr_const);
		params->Set(syb_param_attr_system,syb_param_attr_system,global_syb_attr_const);
		params->Set(syb_param_attr_temporary,syb_param_attr_temporary,global_syb_attr_const);
		t->Set(String::NewSymbol("params"),params,global_syb_attr_const);

		return scope.Close(t->GetFunction());
	}
private:
	static Handle<Value> jsSync(const Arguments& args){
		HandleScope scope;
		Handle<Value> result;
		if(args.IsConstructCall()){
			result=ThrowException(Exception::Error(syb_err_not_a_constructor));
		}else{
			if(args.Length()>1&&(args[0]->IsString()||args[0]->IsStringObject())&&args[1]->IsObject()){
				attrVal *a=jsToAttrval(Handle<Object>::Cast(args[1]));
				String::Value p(args[0]);
				result=basic((wchar_t*)*p,a)?True():False();
				delete a;
			}else{
				result=ThrowException(Exception::Error(syb_err_wrong_arguments));
			}
		}
		return scope.Close(result);
	}
	static Handle<Value> jsAsync(const Arguments& args){
		HandleScope scope;
		Handle<Value> result;
		if(args.IsConstructCall()){
			result=ThrowException(Exception::Error(syb_err_not_a_constructor));
		}else{
			if(args.Length()>1&&(args[0]->IsString()||args[0]->IsStringObject())&&args[1]->IsObject()){
				workdata *data=new workdata;
				data->req.data=data;
				data->self=Persistent<Object>::New(args.This());
				if(args.Length()>2&&args[2]->IsFunction()){
					data->func=Persistent<Function>::New(Handle<Function>::Cast(args[2]));
				}
				String::Value p(args[0]);
				data->path=_wcsdup((wchar_t*)*p);
				data->attr=jsToAttrval(Handle<Object>::Cast(args[1]));
				if(uv_queue_work(uv_default_loop(),&data->req,beginWork,afterWork)==0){
					result=True();
				}else{
					free(data->path);
					data->self.Dispose();
					if(!data->func.IsEmpty()){
						data->func.Dispose();
					}
					delete data->attr;
					delete data;
					result=False();
				}
			}else{
				result=ThrowException(Exception::Error(syb_err_wrong_arguments));
			}
		}
		return scope.Close(result);
	}
	static void beginWork(uv_work_t *req){
		workdata *data=(workdata*)req->data;
		data->result=basic(data->path,data->attr);
		free(data->path);
		delete data->attr;
	}
	static void afterWork(uv_work_t *req){
		HandleScope scope;
		workdata *data=(workdata*)req->data;
		Handle<Value> p=data->result?True():False();
		if(!data->func.IsEmpty()){
			data->func->Call(data->self,1,&p);
			data->func.Dispose();
		}
		data->self.Dispose();
		delete data;
	}
};
const Persistent<String> setAttributes::syb_err_wrong_arguments=global_syb_err_wrong_arguments;
const Persistent<String> setAttributes::syb_err_not_a_constructor=global_syb_err_not_a_constructor;
const Persistent<String> setAttributes::syb_param_attr_archive=global_syb_fileAttr_isArchived;
const Persistent<String> setAttributes::syb_param_attr_hidden=global_syb_fileAttr_isHidden;
const Persistent<String> setAttributes::syb_param_attr_notContentIndexed=global_syb_fileAttr_isNotContentIndexed;
const Persistent<String> setAttributes::syb_param_attr_offline=global_syb_fileAttr_isOffline;
const Persistent<String> setAttributes::syb_param_attr_readonly=global_syb_fileAttr_isReadOnly;
const Persistent<String> setAttributes::syb_param_attr_system=global_syb_fileAttr_isSystem;
const Persistent<String> setAttributes::syb_param_attr_temporary=global_syb_fileAttr_isTemporary;
	
class convertPath{
private:
	static const Persistent<String> syb_err_wrong_arguments;
	static const Persistent<String> syb_err_not_a_constructor;
	static const struct workdata{
		uv_work_t req;
		Persistent<Object> self;
		Persistent<Function> func;
		wchar_t *path;
		bool islong;
	};
public:
	static wchar_t *basic(const wchar_t *path,bool islong){//you need to free the result yourself if it is not NULL
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
		wchar_t *tpath=basic((wchar_t*)*spath,islong);
		if(tpath){
			r=String::New((uint16_t*)tpath);
			free(tpath);
		}else{
			r=String::NewSymbol("");
		}
		return scope.Close(r);
	}
	static Handle<Function> functionRegister(bool isAsyncVersion){
		HandleScope scope;
		Handle<FunctionTemplate> t=FunctionTemplate::New(isAsyncVersion?jsAsync:jsSync);

		//set errmessages
		Handle<Object> errors=Object::New();
		errors->Set(syb_err_wrong_arguments,syb_err_wrong_arguments,global_syb_attr_const);
		errors->Set(syb_err_not_a_constructor,syb_err_not_a_constructor,global_syb_attr_const);
		t->Set(String::NewSymbol("errors"),errors,global_syb_attr_const);

		return scope.Close(t->GetFunction());
	}
private:
	static Handle<Value> jsSync(const Arguments& args){
		HandleScope scope;
		Handle<Value> result;
		if(args.IsConstructCall()){
			result=ThrowException(Exception::Error(syb_err_not_a_constructor));
		}else{
			if(args.Length()>0&&(args[0]->IsString()||args[0]->IsStringObject())){
				result=js(Handle<String>::Cast(args[0]),args[1]->ToBoolean()->IsTrue());
			}else{
				result=ThrowException(Exception::Error(syb_err_wrong_arguments));
			}
		}
		return scope.Close(result);
	}
	static Handle<Value> jsAsync(const Arguments& args){
		HandleScope scope;
		Handle<Value> result;
		if(args.IsConstructCall()){
			result=ThrowException(Exception::Error(syb_err_not_a_constructor));
		}else{
			if(args.Length()>1&&(args[0]->IsString()||args[0]->IsStringObject())&&args[1]->IsFunction()){
				workdata *data=new workdata;
				data->req.data=data;
				data->self=Persistent<Object>::New(args.This());
				data->func=Persistent<Function>::New(Handle<Function>::Cast(args[1]));
				data->islong=args[2]->ToBoolean()->IsTrue();
				String::Value p(args[0]);
				data->path=_wcsdup((wchar_t*)*p);
				if(uv_queue_work(uv_default_loop(),&data->req,beginWork,afterWork)==0){
					result=True();
				}else{
					free(data->path);
					data->self.Dispose();
					data->func.Dispose();
					delete data;
					result=False();
				}
			}else{
				result=ThrowException(Exception::Error(syb_err_wrong_arguments));
			}
		}
		return scope.Close(result);
	}
	static void beginWork(uv_work_t *req){
		workdata *data=(workdata*)req->data;
		wchar_t *p=basic(data->path,data->islong);
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
			p=String::NewSymbol("");
		}
		data->func->Call(data->self,1,&p);
		data->func.Dispose();
		data->self.Dispose();
		delete data;
	}
};
const Persistent<String> convertPath::syb_err_wrong_arguments=global_syb_err_wrong_arguments;
const Persistent<String> convertPath::syb_err_not_a_constructor=global_syb_err_not_a_constructor;

class dirWatcher:ObjectWrap{
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
public:
	dirWatcher(Handle<Object> handle,Handle<Value> *args,uint32_t argc):ObjectWrap(){
		HandleScope scope;
		if(argc>1&&(args[0]->IsString()||args[0]->IsStringObject())&&args[1]->IsFunction()){
			bool e=false;
			Wrap(handle);
			Ref();
			definitions=Persistent<Object>::New(Object::New());
			definitions->Set(syb_callback,args[1]);
			String::Value spath(args[0]);
			pathhnd=CreateFileW((wchar_t*)*spath,FILE_LIST_DIRECTORY,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OVERLAPPED,NULL);
			if(pathhnd==INVALID_HANDLE_VALUE){
				e=true;
			}else{
				memset(&pathreq,0,sizeof(uv_work_t));
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
						Handle<String> path=getCurrentPathByHandle(pathhnd);//get the real path, it could be defferent from args[0]
						if(path->Length()>0){
							definitions->Set(syb_path,path);
							if(Handle<String>::Cast(splitPath::js(path)->Get(splitPath::syb_return_parent))->Length()>0){//path is not a rootdir, so we need to watch its parent to know if the path we are watching has been changed
								definitions->Set(syb_shortName,splitPath::js(convertPath::js(path,false))->Get(splitPath::syb_return_name));//we also need the shortname to makesure the event will be captured
								if(!watchParent(this)){
									e=true;
								}
							}else{
								parenthnd=INVALID_HANDLE_VALUE;
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
	static Handle<Function> functionRegister(){
		HandleScope scope;
		Handle<FunctionTemplate> t=FunctionTemplate::New(New);
		t->InstanceTemplate()->SetInternalFieldCount(1);
		//set methods
		NODE_SET_PROTOTYPE_METHOD(t,"close",close);

		//set error messages
		Handle<Object> errmsgs=Object::New();
		errmsgs->Set(syb_err_unable_to_watch_parent,syb_err_unable_to_watch_parent,global_syb_attr_const);
		errmsgs->Set(syb_err_unable_to_continue_watching,syb_err_unable_to_continue_watching,global_syb_attr_const);
		errmsgs->Set(syb_err_initialization_failed,syb_err_initialization_failed,global_syb_attr_const);
		errmsgs->Set(syb_err_wrong_arguments,syb_err_wrong_arguments,global_syb_attr_const);
		t->Set(String::NewSymbol("errors"),errmsgs,global_syb_attr_const);

		//set events
		Handle<Object> evts=Object::New();
		evts->Set(syb_evt_sta,syb_evt_sta,global_syb_attr_const);
		evts->Set(syb_evt_end,syb_evt_end,global_syb_attr_const);
		evts->Set(syb_evt_new,syb_evt_new,global_syb_attr_const);
		evts->Set(syb_evt_del,syb_evt_del,global_syb_attr_const);
		evts->Set(syb_evt_ren,syb_evt_ren,global_syb_attr_const);
		evts->Set(syb_evt_chg,syb_evt_chg,global_syb_attr_const);
		evts->Set(syb_evt_mov,syb_evt_mov,global_syb_attr_const);
		evts->Set(syb_evt_err,syb_evt_err,global_syb_attr_const);
		t->Set(String::NewSymbol("events"),evts,global_syb_attr_const);

		//set options
		Handle<Object> opts=Object::New();
		opts->Set(syb_opt_subDirs,syb_opt_subDirs,global_syb_attr_const);
		opts->Set(syb_opt_fileSize,syb_opt_fileSize,global_syb_attr_const);
		opts->Set(syb_opt_lastWrite,syb_opt_lastWrite,global_syb_attr_const);
		opts->Set(syb_opt_lastAccess,syb_opt_lastAccess,global_syb_attr_const);
		opts->Set(syb_opt_creation,syb_opt_creation,global_syb_attr_const);
		opts->Set(syb_opt_attributes,syb_opt_attributes,global_syb_attr_const);
		opts->Set(syb_opt_security,syb_opt_security,global_syb_attr_const);
		t->Set(String::NewSymbol("options"),opts,global_syb_attr_const);

		return scope.Close(t->GetFunction());
	}
private:
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
	static Handle<Value> close(const Arguments& args){
		HandleScope scope;
		Handle<Value> result;
		dirWatcher *self=Unwrap<dirWatcher>(args.This());
		if(self->pathhnd==INVALID_HANDLE_VALUE){
			result=False();//this method returns false if dirWatcher is failed to create or already closed
		}else{
			stopWatching(self);
			result=True();
		}
		return scope.Close(result);
	}
	static bool watchParent(dirWatcher *self){
		bool result;
		self->parentreq=(uv_work_t*)malloc(sizeof(uv_work_t));
		if(self->parentreq){
			memset(self->parentreq,0,sizeof(uv_work_t));
			self->parentreq->loop=self->pathreq.loop;
			self->parentreq->data=self;
			self->parentreq->after_work_cb=finishWatchingParent;
			self->parentreq->type=UV_WORK;
			String::Value parent(splitPath::js(Handle<String>::Cast(self->definitions->Get(syb_path)))->Get(splitPath::syb_return_parent));
			self->parenthnd=CreateFileW((wchar_t*)*parent,FILE_LIST_DIRECTORY,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OVERLAPPED,NULL);
			if(self->parenthnd==INVALID_HANDLE_VALUE){
				result=false;
			}else{
				if(CreateIoCompletionPort(self->parenthnd,self->parentreq->loop->iocp,(ULONG_PTR)self->parenthnd,0)){
					result=beginWatchingParent(self);
				}else{
					CloseHandle(self->parenthnd);
					result=false;
				}
			}
		}else{
			self->parenthnd=INVALID_HANDLE_VALUE;
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
		dirWatcher *self=(dirWatcher*)req->data;
		if(req!=self->parentreq||self->parenthnd==INVALID_HANDLE_VALUE){//this is the request we need to realase and it is ready to be released now
			free(req);
			if(self->pathhnd!=INVALID_HANDLE_VALUE){
				callJs(self,syb_evt_end,Null());
				self->Unref();
			}
		}else{
			void *buffer=self->parentbuffer;
			if(req->overlapped.Internal==ERROR_SUCCESS){
				FILE_NOTIFY_INFORMATION *pInfo;
				DWORD d=0;
				bool e=false;
				if(!beginWatchingParent(self)){
					e=true;
				}
				Handle<Value> oldname=splitPath::js(Handle<String>::Cast(self->definitions->Get(syb_path)))->Get(splitPath::syb_return_name);
				do{
					pInfo=(FILE_NOTIFY_INFORMATION*)((ULONG_PTR)buffer+d);
					Handle<String> filename=String::New((uint16_t*)pInfo->FileName,pInfo->FileNameLength/sizeof(wchar_t));
					if((pInfo->Action==FILE_ACTION_REMOVED||pInfo->Action==FILE_ACTION_RENAMED_OLD_NAME)&&(filename->StrictEquals(oldname)||filename->StrictEquals(self->definitions->Get(syb_shortName)))){
						Handle<String> newpath=getCurrentPathByHandle(self->pathhnd);
						self->definitions->Set(syb_path,newpath);
						self->definitions->Set(syb_shortName,splitPath::js(convertPath::js(newpath,false))->Get(splitPath::syb_return_name));
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
				self->parenthnd=INVALID_HANDLE_VALUE;
			}
			free(buffer);
		}
	}
	static void finishWatchingPath(uv_work_t *req){
		HandleScope scope;
		dirWatcher *self=(dirWatcher*)req->data;
		void *buffer=self->pathbuffer;
		self->pathref--;//uv_unref will be called when this function ends if there's no crash
		if(req->overlapped.Internal==ERROR_SUCCESS){
			FILE_NOTIFY_INFORMATION *pInfo;
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
		if(self->pathhnd!=INVALID_HANDLE_VALUE){
			CloseHandle(self->pathhnd);
			self->pathhnd=INVALID_HANDLE_VALUE;
		}
		if(self->pathref>0){
			self->pathreq.type=UV_UNKNOWN_REQ;//mute this request
			uv_unref(self->pathreq.loop);
			self->pathref--;
		}
		if(self->parenthnd!=INVALID_HANDLE_VALUE){
			CloseHandle(self->parenthnd);
			self->parenthnd=INVALID_HANDLE_VALUE;
		}else{
			callJs(self,syb_evt_end,Null());
			self->Unref();
		}
	}
	static void callJs(dirWatcher *self,Persistent<String> evt_type,Handle<Value> src){
		HandleScope scope;
		Handle<Value> arg[2]={evt_type->ToString(),src};
		Handle<Function> callback=Handle<Function>::Cast(self->definitions->Get(syb_callback));
		callback->Call(self->handle_,2,arg);
	}
};
const Persistent<String> dirWatcher::syb_path=NODE_PSYMBOL("PATH");
const Persistent<String> dirWatcher::syb_shortName=NODE_PSYMBOL("SHORT_NAME");
const Persistent<String> dirWatcher::syb_callback=NODE_PSYMBOL("CALLBACK");
const Persistent<String> dirWatcher::syb_opt_subDirs=NODE_PSYMBOL("WATCH_SUB_DIRECTORIES");
const Persistent<String> dirWatcher::syb_opt_fileSize=NODE_PSYMBOL("CHANGE_FILE_SIZE");
const Persistent<String> dirWatcher::syb_opt_lastWrite=NODE_PSYMBOL("CHANGE_LAST_WRITE");
const Persistent<String> dirWatcher::syb_opt_lastAccess=NODE_PSYMBOL("CHANGE_LAST_ACCESS");
const Persistent<String> dirWatcher::syb_opt_creation=NODE_PSYMBOL("CHANGE_CREATION");
const Persistent<String> dirWatcher::syb_opt_attributes=NODE_PSYMBOL("CHANGE_ATTRIBUTES");
const Persistent<String> dirWatcher::syb_opt_security=NODE_PSYMBOL("CHANGE_SECUTITY");
const Persistent<String> dirWatcher::syb_evt_sta=NODE_PSYMBOL("STARTED");
const Persistent<String> dirWatcher::syb_evt_new=NODE_PSYMBOL("ADDED");
const Persistent<String> dirWatcher::syb_evt_del=NODE_PSYMBOL("REMOVED");
const Persistent<String> dirWatcher::syb_evt_chg=NODE_PSYMBOL("MODIFIED");
const Persistent<String> dirWatcher::syb_evt_ren=NODE_PSYMBOL("RENAMED");
const Persistent<String> dirWatcher::syb_evt_mov=NODE_PSYMBOL("MOVED");
const Persistent<String> dirWatcher::syb_evt_end=global_syb_evt_end;
const Persistent<String> dirWatcher::syb_evt_err=global_syb_evt_err;
const Persistent<String> dirWatcher::syb_evt_ren_oldName=NODE_PSYMBOL("OLD_NAME");
const Persistent<String> dirWatcher::syb_evt_ren_newName=NODE_PSYMBOL("NEW_NAME");
const Persistent<String> dirWatcher::syb_err_unable_to_watch_parent=NODE_PSYMBOL("UNABLE_TO_WATCH_PARENT");
const Persistent<String> dirWatcher::syb_err_unable_to_continue_watching=NODE_PSYMBOL("UNABLE_TO_CONTINUE_WATCHING");
const Persistent<String> dirWatcher::syb_err_initialization_failed=global_syb_err_initialization_failed;
const Persistent<String> dirWatcher::syb_err_wrong_arguments=global_syb_err_wrong_arguments;

static void moduleRegister(Handle<Object> target){
	HandleScope scope;
	target->Set(String::NewSymbol("version"),String::NewSymbol(FSWIN_VERSION),global_syb_attr_const);

	target->Set(String::NewSymbol("convertPath"),convertPath::functionRegister(true),global_syb_attr_const);
	target->Set(String::NewSymbol("convertPathSync"),convertPath::functionRegister(false),global_syb_attr_const);
	target->Set(String::NewSymbol("dirWatcher"),dirWatcher::functionRegister(),global_syb_attr_const);
	target->Set(String::NewSymbol("find"),find::functionRegister(true),global_syb_attr_const);
	target->Set(String::NewSymbol("findSync"),find::functionRegister(false),global_syb_attr_const);
	target->Set(String::NewSymbol("getVolumeSpace"),getVolumeSpace::functionRegister(true),global_syb_attr_const);
	target->Set(String::NewSymbol("getVolumeSpaceSync"),getVolumeSpace::functionRegister(false),global_syb_attr_const);
	target->Set(String::NewSymbol("setAttributes"),setAttributes::functionRegister(true),global_syb_attr_const);
	target->Set(String::NewSymbol("setAttributesSync"),setAttributes::functionRegister(false),global_syb_attr_const);
	target->Set(String::NewSymbol("splitPath"),splitPath::functionRegister(),global_syb_attr_const);

	Handle<Object> ntfsgroup=Object::New();
	ntfsgroup->Set(String::NewSymbol("getCompressedSize"),getCompressedSize::functionRegister(true),global_syb_attr_const);
	ntfsgroup->Set(String::NewSymbol("getCompressedSizeSync"),getCompressedSize::functionRegister(false),global_syb_attr_const);
	ntfsgroup->Set(String::NewSymbol("setCompression"),setCompression::functionRegister(true),global_syb_attr_const);
	ntfsgroup->Set(String::NewSymbol("setCompressionSync"),setCompression::functionRegister(false),global_syb_attr_const);
	ntfsgroup->Set(String::NewSymbol("setShortName"),setShortName::functionRegister(true),global_syb_attr_const);
	ntfsgroup->Set(String::NewSymbol("setShortNameSync"),setShortName::functionRegister(false),global_syb_attr_const);
	target->Set(String::NewSymbol("ntfs"),ntfsgroup,global_syb_attr_const);
}
NODE_MODULE(fsWin,moduleRegister);