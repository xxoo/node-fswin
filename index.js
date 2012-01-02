var path = require('path');
var fs = require('fs');
var util = require('util');
var EventEmitter = require('events').EventEmitter;

var fsWin = require('./fsWin.node');


module.exports = {
  watchFolder: watchFolder,
  explodePath: explodePath,
  convertName: convertName,
  longName: longName,
  shortName: shortName
};


/**
 * Convert a path to a long name
 * @param  {String}   path       Input pathname
 * @param  {Function} [callback] Optional callback for asynchronous use.
 * @return {[String]}            Returns converted path if used synchronously
 */
function longName(path, callback){
  return convertName(path, true, callback);
}

/**
 * Convert a path to a short name
 * @param  {String}   path       Input pathname
 * @param  {Function} [callback] Optional callback for asynchronous use.
 * @return {[String]}            Returns converted path if used synchronously
 */
function shortName(path, callback){
  return convertName(path, false, callback);
}

/**
 * Convert a path to a long or short name
 * @param  {String}   path       Input pathname
 * @param  {Boolean}  long       Whether to return long or short name
 * @param  {Function} [callback] Optional callback for asynchronous use.
 * @return {[String]}            Returns converted path if used synchronously
 */
function convertName(path, long, callback){
  if (!callback) {
    return fsWin.convertPathSync(path, long) || new Error('convertPath failed');
  }

  var error = !fsWin.convertPath(path, function(path){
    callback(null, path);
  }, long);
  if (error) {
    callback(new Error('convertPath failed'));
  }
}

/**
 * Convert a path into an array of path components
 * @param  {String}   path  Input pathname
 * @return {String[]}       Array of components
 */
function explodePath(path){
  var result = { PARENT: path };
  var output = [];
  while (result.PARENT.length) {
    result = fsWin.splitPath(result.PARENT);
    output.unshift(result.NAME);
  }
  return output;
}

/**
 * Add a directory watcher to a folder
 * @param  {String} path       Input pathname
 * @return {DirectoryWatcher}  An event emitter that emits events when changes happen
 */
function watchFolder(path){
  return new DirectoryWatcher(path);
}

function DirectoryWatcher(path){
  this.path = resolve(path);
  if (!this.path) throw new Error('Path not found');

  EventEmitter.call(this);

  Object.defineProperty(this, 'dirwatcher', {
    value: new fsWin.dirWatcher(this.path, handler.bind(this), defaults)
  });
}

util.inherits(DirectoryWatcher, EventEmitter);

DirectoryWatcher.prototype.close = function close(){ this.dirwatcher.close() };


function handler(name, message){
  var event = { path: this.path };
  switch (name) {
    case 'STARTED':
    case 'ENDED':
      break;
    case 'ADDED':
    case 'REMOVED':
    case 'MODIFIED':
      event.target = message;
      break;
    case 'RENAMED':
      event.target = message.OLD_NAME;
      event.changed = message.NEW_NAME;
      break;
    case 'MOVED':
      event.target = this.path;
      event.changed = message;
      break;
    case 'ERROR':
      if (message) {
        if (message === 'UNABLE_TO_WATCH_PARENT') {
          return Object.defineProperty(this, 'noMoveNotifications', {
            value: true,
            enumerable: true
          });
        }
        event.error = message;
        event.message = errors[message];
      } else {
        event.error = 'UNKNOWN_ERROR';
        event.message = 'An unknown error has occured. You should never see this.';
      }
  }
  this.emit(name.toLowerCase(), event);
}


function resolve(name){
  var resolved = name;
  if (!path.existsSync(resolved)) {
    resolved = path.resolve(name);
    if (!path.existsSync(resolved)) {
      resolved = path.resolve(process.cwd(), name);
      if (!path.existsSync(resolved)) {
        resolved = false;
      }
    }
  }
  return resolved;
}

var defaults = {
  WATCH_SUB_DIRECTORIES: true,
  CHANGE_FILE_SIZE:      true,
  CHANGE_LAST_WRITE:     true,
  CHANGE_LAST_ACCESS:    true,
  CHANGE_CREATION:       true,
  CHANGE_ATTRIBUTES:     true,
  CHANGE_SECUTITY:       true,
};

var errors = {
  'INITIALIZATION_FAILED': 'Failed to initialize. Any failure during initialization may case this error, '+
                           'including inaccessible or non-existant paths.',
  'UNABLE_TO_CONTINUE_WATCHING': 'An error is has caused the watcher to stop working.'
};