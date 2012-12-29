#pragma once
#include "main.h"

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