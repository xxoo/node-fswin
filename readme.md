Introduction

node-fsWin is a native windows add-on for node.js.
It contains some platform specified functions

dirWatcher
a directory watcher object that is more suitable for windows then the internal fs.watch()
it supplies some freture that the fs.watch() doesn't contain
a) directory tree watching(with higher performance then recursion functions)
b) more events(including added,removed,modified,renamed)
c) more options
d) also watchs the directory itself, not only its contents(this feature requires vista or latter)

splitPath
a function that split a path to its parent and name
this function can recognize rootdirs(including local and network paths).
and if a path that passed in is a rootdir the parent part will be empty
and the name is just the path that passed in
note: this function is only suitable for windows full paths
passing a relative path or any other kind of path will case a unexpected return value

convertPath and convertPathSync
it converts paths between 8.3 name and long name.
this function requires a filesystem I/O, so it contains both a block and non-block version

Examples

dirWatcher

```javascript
var fsWin=require('fsWin.node');
var watcher=new fsWin.dirWatcher('d:\\test',
function(event,detail){
	if(event==='started'){
		console.log('watcher started in: '+detail);
	}else if(event==='added'){
		console.log(detail+' is added');
	}else if(event==='removed'){
		console.log(detail+' is removed');
	}else if(event==='changed'){
		console.log(detail+' is changed');
	}else if(event==='renamed'){
		console.log(detail.from+' is renamed to '+detail.to);
	}else if(event==='moved'){
		console.log('the directory you are watching is moved to '+detail);
	}else if(event==='error'){
		console.log('an error occured: '+detail);
	}else if(event==='ended'){
		console.log('the watcher is about to quit');
	}
	//if you want to stop watching, call the close method
	//this.close();
},
{//options is not required, and this is the default value
	subDirs:true,//watch the dir tree
	fileSize:true,//watch file size changes, will fire in change event
	lastWrite:true,//watch last write time changes, will fire in change event
	lastAccess:false,//watch last access time changes, will fire in change event
	creation:false,//watch creation time changes, will fire in change event
	attributes:false,//watch attributes changes, will fire in change event
	security:false//watch security changes, will fire in change event;
});
```

splitPath

```javascript
var fsWin=require('fsWin.node');
var paths=['C:\\PROGRA~1','C:\\program files\\Common Files','c:\\windows\\system32','c:\\','\\\\mycomputer\\sharefolder\\somedir','\\\\mycomputer\\sharedfolder'];
var i,splittedpath;
for(i=0;i<paths.length;i++){
	splittedpath=fsWin.splitPath(paths[i]);
	console.log('path: "'+paths[i]+'" is splitted to "'+splittedpath.parent+'" and "'+splittedpath.name+'"');
}
```

convertPath and convertPathSync

```javascript
var fsWin=require('fsWin.node');
var paths=['C:\\PROGRA~1','C:\\program files\\Common Files','c:\\windows\\system32','c:\\','\\\\mycomputer\\sharefolder\\somedir','\\\\mycomputer\\sharedfolder'];
var i;
//test the sync version
//note: this function may return an empty string if the path you passed in is not found.
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
		console.log('this function will get your answer');
	}else{
		console.log('there is an unexpected error, do not trust the filename it returns');
	}
}
```