#pragma once
#include "main.h"

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
					ngx_queue_insert_tail(&req->loop->active_reqs,&req->active_queue);
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