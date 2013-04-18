#pragma once
#include "main.h"

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
	static void afterWork(uv_work_t *req,int status){
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