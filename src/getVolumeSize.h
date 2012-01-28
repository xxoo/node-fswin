#pragma once
#include "main.h"

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