#pragma once
#include "main.h"

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