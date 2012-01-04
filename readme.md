# Introduction

node-fsWin is a native windows add-on for node.js. It contains some platform specified functions.

#### dirWatcher
a directory watcher object that is more suitable for windows then the internal fs.watch().
it supplies some freture the fs.watch() doesn't contain.

1. directory tree watching(with higher performance then recursion functions).
2. more events(including added, removed, modified, renamed).
3. with options.
4. also watches the directory itself, not only its children(this feature requires vista or latter).

#### splitPath
a function that split a path to its parent and name.
this function can recognize rootdirs(including local and network paths).
if a path that passed in is a rootdir the parent part will be empty.
and the name is just the path that passed in.
local rootdirs always contain a back slash in the end, such as **c:\\**.
network rootdir is something like **\\\\mycomputer\\sharedfolder**.
note: this function is only suitable for windows full paths.
passing a relative path or any other kind of path will case an unexpected return value.

#### convertPath and convertPathSync
converts paths between 8.3 name and long name.
these functions require a filesystem or network I/O, so there are both a block and non-block versions.

#### find and findSync
find file or directory by path.
these functions are like the dir command. using wild cards are allowed.
both the block and non-block versions contain a basic mode and progressive mode.
basic mode will wait till the search finish and return all results in an array.
the progressive mode will return a single result for each callback.
this is useful when you are listing many files or the callback has much works to do.
during the process the file information might be outdated.
and you also want to save an I/O for each result doing this job.


# Examples

#### dirWatcher

```javascript
var fsWin=require('fsWin.node');
var options={},e;
options[fsWin.dirWatcher.options.WATCH_SUB_DIRECTORIES]=true;//watch the dir tree
options[fsWin.dirWatcher.options.CHANGE_FILE_SIZE]=true;//watch file size changes, will fire in 'MODIFIED' event
options[fsWin.dirWatcher.options.CHANGE_LAST_WRITE]=true;//watch last write time changes, will fire in 'MODIFIED' event
options[fsWin.dirWatcher.options.CHANGE_LAST_ACCESS]=false;//watch last access time changes, will fire in 'MODIFIED' event
options[fsWin.dirWatcher.options.CHANGE_CREATION]=false;//watch creation time changes, will fire in 'MODIFIED' event
options[fsWin.dirWatcher.options.CHANGE_ATTRIBUTES]=false;//watch attributes changes, will fire in 'MODIFIED' event
options[fsWin.dirWatcher.options.CHANGE_SECUTITY]=false;//watch security changes, will fire in 'MODIFIED' event;
try{
	var watcher=new fsWin.dirWatcher(
		'd:\\test',//the directory you are about to watch
		function(event,message){
			if(event in this.constructor.events){
			}else{
			}
			if(event===this.constructor.events.STARTED){
				console.log('watcher started in: "'+message+'"');
			}else if(event===this.constructor.events.ADDED){
				console.log('"'+message+'" is added');
			}else if(event===this.constructor.events.REMOVED){
				console.log('"'+message+'" is removed');
			}else if(event===this.constructor.events.MODIFIED){
				console.log('"'+message+'" is modified');
			}else if(event===this.constructor.events.RENAMED){
				console.log('"'+message.OLD_NAME+'" is renamed to "'+message.NEW_NAME+'"');
			}else if(event===this.constructor.events.MOVED){
				console.log('the directory you are watching is moved to "'+message+'"');
			}else if(event===this.constructor.events.ENDED){
				console.log('the watcher is about to quit');
			}else if(event===this.constructor.events.ERROR){
				if(message===this.constructor.errors.INITIALIZATION_FAILED){
					console.log('failed to initialze the watcher. any failure during the initialization may case this error. such as you want to watch an unaccessable or unexist directory.');
				}else if(message===this.constructor.errors.UNABLE_TO_WATCH_PARENT){
					console.log('failed to watch parent diectory. it means the "MOVED" event will nolonger fire. this error always occurs at the start up under winxp. since the GetFinalPathNameByHandleW API is not available.');
				}else if(message===this.constructor.errors.UNABLE_TO_CONTINUE_WATCHING){
					console.log('some error makes the watcher can not continue working. it also means the watcher will exit soon.');
				}else{
					console.log('you should never see this message: "'+message+'"');
				}
			}
			//if you want to stop watching, call the close method
			//note: this method returns false if the watcher is already or being closed. otherwise true
			//if(this.close()){
			//	console.log('closing the watcher.');
			//}else{
			//	console.log('no need to close the watcher again.');
			//}
		},
		options//not required, and this is the default value
	);
}catch(e){
	if(e.message===fsWin.dirWatcher.errors.WRONG_ARGUMENTS){
		console.log('check the argumens you\'ve passed in. make sure there are at least two arguments. the first is a string, and the second is a function.');
	}else{
		console.log('an unexcepted error occurs: '+ e.message);
	}
}
```

#### splitPath

```javascript
var fsWin=require('fsWin.node');
var paths=['C:\\PROGRA~1','C:\\program files\\Common Files','c:\\windows\\system32','c:\\','\\\\mycomputer\\sharefolder\\somedir','\\\\mycomputer\\sharedfolder'];
var i,splittedpath;
for(i=0;i<paths.length;i++){
	splittedpath=fsWin.splitPath(paths[i]);
	console.log(paths[i]+'" is splitted to "'+splittedpath[fsWin.splitPath.returns.PARENT]+'" and "'+splittedpath[fsWin.splitPath.returns.NAME]+'"');
}
```

#### convertPath and convertPathSync

```javascript
var fsWin=require('fsWin.node');
var paths=['C:\\PROGRA~1','C:\\program files\\Common Files','c:\\windows\\system32','c:\\','\\\\mycomputer\\sharefolder\\somedir','\\\\mycomputer\\sharedfolder'];
var i;
//test the sync version
//note: if the path does not exist, this function will return an empty string.
for(i=0;i<paths.length;i++){
	console.log('the long name of "'+paths[i]+'" is: "'+fsWin.convertPathSync(paths[i],true)+'" and its short name is "'+fsWin.convertPathSync(paths[i])+'"');
}
//test the async version
//note: this function returns false before the async call if error occurs
for(i=0;i<paths.length;i++){
	if(fsWin.convertPath(paths[i],function(path){
		this.convertPath(path,function(lpath){
			console.log('short name: "'+path+'" long name: "'+lpath+'"');
		},true);
	})){
		console.log('the async call will get your answer');
	}else{
		console.log('there is an unexpected error, the async call should not be called. but if it is called, do not trust the filename');
	}
}
```

#### convertPath and convertPathSync

```javascript
var fsWin=require('fsWin.node');
var path='c:\\windows\\system32\\*';

//a list of properties of the file object
//for more infomation see http://msdn.microsoft.com/en-us/library/windows/desktop/aa365740
var n;
for(n in fsWin.find.returns){
	console.log(n);
}

//sync basic mode
var i;
var result=fsWin.findSync(path);
for(i=0;i<result.length;i++){
	console.log(result[i].LONG_NAME+'	'+(result[i].IS_DIRECTORY?'<DIR>':result[i].SIZE));
}

//sync progressive mode
console.log('found ',+fsWin.findSync(path,function(file){
	console.log(file.LONG_NAME+'	'+(file.IS_DIRECTORY?'<DIR>':file.SIZE));
})+' file(s) or dir(s)');

//async basic mode
console.log(fsWin.find(path,function(result){
	var i;
	for(i=0;i<result.length;i++){
		console.log(result[i].LONG_NAME+'	'+(result[i].IS_DIRECTORY?'<DIR>':result[i].SIZE));
	}
})?'succeeded':'failed');

//async progressive mode
console.log(fsWin.find(path,function(event,message){
	if(event==='FOUND'){
		console.log(message.LONG_NAME+'	'+(message.IS_DIRECTORY?'<DIR>':message.SIZE));
	}else if(event==='ENDED'){
		console.log('this operation is complated.');
	}else if(event==='ERROR'){
		console.log(message);//should be 'UNABLE_TO_CONTINUE_SEARCHING' if error occurs
	}
},true)?'succeeded':'failed');
```