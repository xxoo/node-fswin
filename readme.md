Introduction
============

node-fsWin is a native windows add-on for node.js. It contains some platform specified functions.


## dirWatcher

a directory watcher object that is more suitable for windows then the internal `fs.watch()`.
it supplies some freture the `fs.watch()` doesn't contain.

- directory tree watching(with higher performance then recursion functions).
- more events(including added, removed, modified, renamed).
- with options.
- also watches the directory itself, not only its children(this feature requires vista or latter).


## splitPath

a function to split a path to its parent and name that recognizes rootdirs(including local and network paths).
if a path passed in is a rootdir the parent part will be empty, and the name is just the path itself.
rootdirs always contain a trailing backslash, such as `C:\` or `\\mycomputer\sharedfolder\`.

note: this function is only suitable for windows full paths.
passing a relative path or any other kind of path will case an unexpected return value.


## convertPath and convertPathSync

converts paths between 8.3 name and long name.

these functions require a filesystem or network I/O, so there are both a block and non-block versions.


## setShortName and setShortNameSync

set or delete the 8.3 name of a file or directory. these functions have some requirements:

- NTFS file system
- you may need to run as administrator
- require win7 or latter to delete an existing short name
- the registry key `HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\FileSystem\NtfsDisable8dot3NameCreation` must be 0(the default value).

for details, see http://msdn.microsoft.com/en-us/library/aa365543


## find and findSync

find files or directories by path.

these functions are like the dir command. using wild cards are allowed.
both the block and non-block versions contain a basic mode and progressive mode.

basic mode will wait till the search finish and return all results in an array.
the progressive mode will reutrn every single result as soon as it is available.
this mode is useful when you are listing many files or the callback has much works to do.
during the process the file information might be outdated.
if you don't want to waste another I/O on each file for doing this job, try this mode.


Examples
========

you might need to set your own path to `fsWin.node`

## dirWatcher

```javascript
var fsWin=require('fsWin.node');
var options={};
options[fsWin.dirWatcher.options.WATCH_SUB_DIRECTORIES]=true;//watch the directory tree
options[fsWin.dirWatcher.options.CHANGE_FILE_SIZE]=true;//watch file size changes, will fire in 'MODIFIED' event
options[fsWin.dirWatcher.options.CHANGE_LAST_WRITE]=true;//watch last write time changes, will fire in 'MODIFIED' event
options[fsWin.dirWatcher.options.CHANGE_LAST_ACCESS]=false;//watch last access time changes, will fire in 'MODIFIED' event
options[fsWin.dirWatcher.options.CHANGE_CREATION]=false;//watch creation time changes, will fire in 'MODIFIED' event
options[fsWin.dirWatcher.options.CHANGE_ATTRIBUTES]=false;//watch attributes changes, will fire in 'MODIFIED' event
options[fsWin.dirWatcher.options.CHANGE_SECUTITY]=false;//watch security changes, will fire in 'MODIFIED' event;
var watcher=new fsWin.dirWatcher(//the 'new' operator can be ignored
	'd:\\test',//the directory you are about to watch
	function(event,message){
		if(event===fsWin.dirWatcher.events.STARTED){
			console.log('watcher started in: "'+message+'". on vista and latter, this message is a full path. and it could be different from the path that you passed in, as symlink will resolve to its target.');
		}else if(event===fsWin.dirWatcher.events.MOVED){
			console.log('the directory you are watching is moved to "'+message+'". this event will not fire on xp, and the message is also a full path. just like the "STARTED" event');
		}else if(event===fsWin.dirWatcher.events.ADDED){
			console.log('"'+message+'" is added. it could be created or moved to here.');
		}else if(event===fsWin.dirWatcher.events.REMOVED){
			console.log('"'+message+'" is removed. it could be deleted or moved from here.');
		}else if(event===fsWin.dirWatcher.events.MODIFIED){
			console.log('"'+message+'" is modified. this event does not contain any detail of what change is made.');
		}else if(event===fsWin.dirWatcher.events.RENAMED){
			console.log('"'+message.OLD_NAME+'" is renamed to "'+message.NEW_NAME+'"');
		}else if(event===fsWin.dirWatcher.events.ENDED){
			console.log('the watcher is about to quit. it is save to set the watcher to null or any other value now.');
		}else if(event===fsWin.dirWatcher.events.ERROR){
			if(message===fsWin.dirWatcher.errors.INITIALIZATION_FAILED){
				console.log('failed to initialze the watcher. any failure during the initialization may case this error. such as you want to watch an unaccessable or unexist directory.');
			}else if(message===fsWin.dirWatcher.errors.UNABLE_TO_WATCH_PARENT){
				console.log('failed to watch parent diectory. it means the "MOVED" event will nolonger fire. this error always occurs at the start up under winxp. since the GetFinalPathNameByHandleW API is not available.');
			}else if(message===fsWin.dirWatcher.errors.UNABLE_TO_CONTINUE_WATCHING){
				console.log('some error makes the watcher stop working. perhaps the directory you are watching is deleted or become unaccessable. the "ENDED" event will fire after this error.');
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
	options//not required, and this is the default value since filesize+lastwrite is always enough to determine a content change in most case.
);
```


## splitPath

```javascript
var fsWin=require('fsWin.node');
var paths=['C:\\PROGRA~1','C:\\program files\\Common Files','\\\\mycomputer\\sharefolder\\somedir\\anotherdir','c:\\','\\\\mycomputer\\sharefolder\\somedir','\\\\mycomputer\\sharedfolder\\'];
var i,splittedpath;
for(i=0;i<paths.length;i++){
	splittedpath=fsWin.splitPath(paths[i]);
	console.log(paths[i]+'" is splitted to "'+splittedpath[fsWin.splitPath.returns.PARENT]+'" and "'+splittedpath[fsWin.splitPath.returns.NAME]+'"');
}
```


## convertPath and convertPathSync

```javascript
var fsWin=require('fsWin.node');
var paths=['C:\\PROGRA~1','C:\\program files\\Common Files','\\\\mycomputer\\sharefolder\\somedir\\anotherdir','c:\\','\\\\mycomputer\\sharefolder\\somedir','\\\\mycomputer\\sharedfolder\\'];
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

## setShortName and setShortNameSync

```javascript
var fsWin=require('fsWin.node');
var pathtoset='d:\\downloads';

//sync version
fsWin.findSync(pathtoset,function(file){
	console.log('short name is: "'+file.SHORT_NAME+'"');
});
if(fsWin.ntfs.setShortNameSync(pathtoset,'12345')){
	fsWin.findSync(pathtoset,function(file){
		console.log('new short name is: "'+file.SHORT_NAME+'"');
	});
}else{
	console.log('failed to set short name');
}

//async version
fsWin.findSync(pathtoset,function(file){
	console.log('short name is: "'+file.SHORT_NAME+'"');
});
fsWin.ntfs.setShortName(pathtoset,'abcde',function(done){
	if(done){
		fsWin.findSync(pathtoset,function(file){
			console.log('new short name is: "'+file.SHORT_NAME+'"');
		});
	}else{
		console.log('failed to set short name');
	}
})
```


## find and findSync

```javascript
var fsWin=require('fsWin.node');
var path='c:\\windows\\system32\\*';

//list property names of the file object that the following functions will return.
//for more infomation see http://msdn.microsoft.com/en-us/library/windows/desktop/aa365740
//the value of property 'REPARSE_POINT_TAG' could be one of 'DFS', 'DFSR', 'HSM', 'HSM2', 'MOUNT_POINT', 'SIS', 'SYMLINK', 'UNKNOWN' and ''(an empty string means no tag)
var n;
for(n in fsWin.find.returns){
	console.log(fsWin.find.returns[n]);
}

//sync basic mode
var i;
var files=fsWin.findSync(path);
for(i=0;i<result.length;i++){
	console.log(files[i].LONG_NAME+'	'+(files[i].IS_DIRECTORY?'<DIR>':files[i].SIZE));
}

//sync progressive mode
console.log('found '+fsWin.findSync(path,function(file){
	console.log(file.LONG_NAME+'	'+(file.IS_DIRECTORY?'<DIR>':file.SIZE));
	if(file.LONG_NAME.toLowerCase()==='drivers'){
		return true;//stop the search process by returning this value
	}
})+' file(s) or dir(s)');

//async basic mode
console.log(fsWin.find(path,function(files){
	var i;
	for(i=0;i<files.length;i++){
		console.log(files[i].LONG_NAME+'	'+(files[i].IS_DIRECTORY?'<DIR>':files[i].SIZE));
	}
})?'succeeded':'failed');

//async progressive mode
console.log(fsWin.find(path,function(event,message){
	if(event==='FOUND'){
		console.log(message.LONG_NAME+'	'+(message.IS_DIRECTORY?'<DIR>':message.SIZE));
		if(message.LONG_NAME.toLowerCase()==='shell32.dll'){
			return true;//stop the search process by returning this value, and 'INTERRUPTED' event will fire
		}
	}else if(event==='SUCCEEDED'){
		console.log('this operation is completed successfully. found '+message+' file(s) or dir(s)');
	}else if(event==='FAILED'){
		console.log('unable to search next file. the result might be incomplete. found '+message+' file(s) or dir(s)');
	}else if(event==='INTERRUPTED'){
		console.log('this operation is interrupted by user. found '+message+' file(s) or dir(s)');
	}
},true)?'succeeded':'failed');
```