# Introduction [![Build with node-gyp](https://github.com/xxoo/node-fswin/actions/workflows/main.yml/badge.svg)](https://github.com/xxoo/node-fswin/actions/workflows/main.yml)

[`fswin`](https://www.npmjs.com/package/fswin) is a native [`node.js`](http://nodejs.org) add-on that works on windows.
It has ported some platform specified filesystem APIs. And made them easy to use in javascript.

For api documents and examples please see [wiki](https://github.com/xxoo/node-fswin/wiki)

NOTE: fswin depends on n-api since v3. which requires node.js v8 or later.

## How to Install

You may install fswin by using the following command line:

```
npm install fswin
```

The package contains prebuilt binaries for `node.js`, `electron` and `nw.js` on all architectures.

## How to Build

first you need to install [Visual Studio](http://www.visualstudio.com) 2019 or later.

then you need to install [Python](https://www.python.org/downloads/windows)

then you need to install node-gyp

```
npm -g install node-gyp
```

then you need to clone the repo

```
git clone https://github.com/xxoo/node-fswin.git
cd node-fswin
```

finally you can build the source

for `node.js`:

```
node-gyp rebuild
```

for `electron`:

```
node-gyp rebuild --target=x.x.x --dist-url=https://atom.io/download/electron
```

for `nw.js` please follow the [official document](https://nwjs.readthedocs.io/en/latest/For%20Users/Advanced/Use%20Native%20Node%20Modules/)
