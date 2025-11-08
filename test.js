'use strict';

const fs = require('node:fs');
const path = require('node:path');
const assert = require('node:assert/strict');
const { test } = require('node:test');

(async () => {
	if (process.platform !== 'win32') {
		await test('fswin is only available on Windows', () => {
			const fswin = require('./index.js');
			assert.strictEqual(fswin, null);
		});
		return;
	}

	await test('prepare built binary', () => {
		const binaryPath = './build/Release/fswin.node';
		assert.ok(fs.existsSync(binaryPath), 'binary exists after node-gyp build');
		const archDir = `./${process.arch}`;
		const destPath = `${archDir}/fswin.node`;
		fs.mkdirSync(archDir, { recursive: true });
		fs.rmSync(destPath, { force: true });
		fs.linkSync(binaryPath, destPath);
	});

	const fswin = require('./index.js');

	const driveLetter = process.env.SystemDrive[0];
	const driveLetterLower = driveLetter.toLowerCase();
	const systemRoot = process.env.SystemRoot || `${driveLetter}:\\Windows`;
	const volumeRoot = `${driveLetter}:\\`;

	const scratch = {
		root: path.resolve('tmp-fswin-tests'),
		nested: null,
		file: null,
		extraFile: null,
		watchDir: null,
		watchTarget: null,
		sparseFile: null,
		shortNameDir: null,
		wildcard: null,
	};
	scratch.nested = path.join(scratch.root, 'Sub Dir');
	scratch.file = path.join(scratch.nested, 'sample.txt');
	scratch.extraFile = path.join(scratch.nested, 'extra.log');
	scratch.watchDir = path.join(scratch.root, 'watch');
	scratch.watchTarget = path.join(scratch.watchDir, 'watch.txt');
	scratch.sparseFile = path.join(scratch.root, 'sparse.bin');
	scratch.shortNameDir = path.join(scratch.root, 'ShortName Target');
	scratch.wildcard = path.join(scratch.nested, '*.txt');

	const fileContents = 'fswin wiki test fixture';

	await test('fswin exposes documented surface area', () => {
		assert.ok(fswin, 'fswin exports a value on Windows');
		assert.strictEqual(typeof fswin.version, 'string');
		assert.strictEqual(typeof fswin.convertPathSync, 'function');
		assert.strictEqual(typeof fswin.dirWatcher, 'function');
		assert.strictEqual(typeof fswin.findSync, 'function');
		assert.strictEqual(typeof fswin.getLogicalDriveListSync, 'function');
		assert.strictEqual(typeof fswin.getDriveDeviceSync, 'function');
		assert.strictEqual(typeof fswin.getDeviceCapabilitiesSync, 'function');
		assert.strictEqual(typeof fswin.getStoragePropertiesSync, 'function');
		assert.strictEqual(typeof fswin.getVolumeInformationSync, 'function');
		assert.strictEqual(typeof fswin.getVolumeSpaceSync, 'function');
		assert.strictEqual(typeof fswin.setVolumeLabelSync, 'function');
		assert.strictEqual(typeof fswin.ejectDriveSync, 'function');
		assert.strictEqual(typeof fswin.getAttributesSync, 'function');
		assert.strictEqual(typeof fswin.setAttributesSync, 'function');
		assert.strictEqual(typeof fswin.splitPath, 'function');
		assert.ok(fswin.ntfs, 'ntfs namespace available');
		assert.strictEqual(typeof fswin.ntfs.getCompressedSizeSync, 'function');
		assert.strictEqual(typeof fswin.ntfs.setCompressionSync, 'function');
		assert.strictEqual(typeof fswin.ntfs.setShortNameSync, 'function');
		assert.strictEqual(typeof fswin.ntfs.setSparseSync, 'function');
	});

	await test('set up scratch fixtures', () => {
		fs.rmSync(scratch.root, { recursive: true, force: true });
		fs.mkdirSync(scratch.nested, { recursive: true });
		fs.mkdirSync(scratch.watchDir, { recursive: true });
		fs.mkdirSync(scratch.shortNameDir, { recursive: true });
		fs.writeFileSync(scratch.file, fileContents);
		fs.writeFileSync(scratch.extraFile, 'additional entry');
		fs.writeFileSync(scratch.sparseFile, Buffer.alloc(4096, 0));
		fs.writeFileSync(path.join(scratch.shortNameDir, 'child.txt'), 'short name child');
		fs.rmSync(scratch.watchTarget, { force: true });
	});

	await test('convertPath converts long and short names like in the wiki', () => {
		const longSystemRoot = fswin.convertPathSync(systemRoot, true);
		assert.ok(longSystemRoot, 'long path should resolve');
		assert.ok(longSystemRoot.toLowerCase().endsWith('\\windows'), 'long path resolves to Windows directory');
		const shortSystemRoot = fswin.convertPathSync(longSystemRoot);
		assert.strictEqual(typeof shortSystemRoot, 'string');
		assert.ok(shortSystemRoot.length > 0, 'short path is not empty');
		assert.match(shortSystemRoot, /^[A-Za-z]:\\/, 'short path keeps drive information');
	});

	await test('convertPathAsync mirrors sync results', async () => {
		const expected = fswin.convertPathSync(systemRoot, true);
		const actual = await fswin.convertPathAsync(systemRoot, true);
		assert.strictEqual(actual, expected);
	});

	await test('splitPath handles local and UNC forms', () => {
		const local = fswin.splitPath(`${volumeRoot}Folder\\Child`);
		assert.strictEqual(local.PARENT.toLowerCase(), `${volumeRoot}folder`.toLowerCase());
		assert.strictEqual(local.NAME.toLowerCase(), 'child');
		const unc = fswin.splitPath('\\\\server\\share\\folder');
		assert.strictEqual(unc.PARENT.toLowerCase(), '\\\\server\\share\\');
		assert.strictEqual(unc.NAME.toLowerCase(), 'folder');
	});

	await test('dirWatcher reports file additions and modifications', async () => {
		fs.rmSync(scratch.watchTarget, { force: true });
		await new Promise((resolve, reject) => {
			const events = new Set();
			let watcher;
			const timer = setTimeout(() => {
				watcher?.close();
				reject(Error('dirWatcher timed out without events'));
			}, 10000);
			watcher = new fswin.dirWatcher(
				scratch.watchDir,
				(event, message) => {
					if (event === 'STARTED') {
						setTimeout(() => {
							fs.writeFileSync(scratch.watchTarget, 'watch-start');
							setTimeout(() => {
								fs.renameSync(scratch.watchDir, scratch.watchDir + '_moved');
							}, 200);
						}, 100);
					} else if (event === 'ADDED' && message === 'watch.txt') {
						events.add('ADDED');
					} else if (event === 'MODIFIED' && message === 'watch.txt') {
						events.add('MODIFIED');
					} else if (event === 'MOVED' && message === scratch.watchDir + '_moved') {
						events.add('MOVED');
					} else if (event === 'ERROR') {
						clearTimeout(timer);
						watcher.close();
						reject(Error(`dirWatcher error: ${message}`));
					}
					if (events.has('ADDED') && events.has('MODIFIED') && events.has('MOVED')) {
						clearTimeout(timer);
						watcher.close();
						resolve();
					}
				},
				{
					CHANGE_FILE_SIZE: true,
					CHANGE_LAST_WRITE: true,
				},
			);
		});
	});

	await test('getAttributes reports metadata for directories', async () => {
		const attrs = fswin.getAttributesSync(scratch.nested);
		assert.ok(attrs, 'sync attributes should exist');
		assert.strictEqual(attrs.IS_DIRECTORY, true);
		assert.ok(attrs.CREATION_TIME instanceof Date);
		const asyncAttrs = await fswin.getAttributesAsync(scratch.nested);
		assert.strictEqual(asyncAttrs.IS_DIRECTORY, true);
	});

	await test('getAttributes reports metadata for files', async () => {
		const attrs = fswin.getAttributesSync(scratch.file);
		assert.ok(attrs);
		assert.strictEqual(attrs.IS_DIRECTORY, false);
		assert.ok(attrs.SIZE >= Buffer.byteLength(fileContents));
		const asyncAttrs = await fswin.getAttributesAsync(scratch.file);
		assert.strictEqual(asyncAttrs.IS_DIRECTORY, false);
	});

	await test('setAttributes toggles read-only flag on files', async () => {
		try {
			const setReadOnly = fswin.setAttributesSync(scratch.file, { IS_READ_ONLY: true });
			assert.strictEqual(setReadOnly, true);
			let attrs = fswin.getAttributesSync(scratch.file);
			assert.strictEqual(attrs.IS_READ_ONLY, true);
			const cleared = await fswin.setAttributesAsync(scratch.file, { IS_READ_ONLY: false });
			assert.strictEqual(cleared, true);
			attrs = fswin.getAttributesSync(scratch.file);
			assert.strictEqual(attrs.IS_READ_ONLY, false);
		} finally {
			fswin.setAttributesSync(scratch.file, { IS_READ_ONLY: false });
		}
	});

	await test('findSync basic mode returns expected files', () => {
		const matches = fswin.findSync(path.join(scratch.nested, 'sample.txt'));
		assert.strictEqual(matches.length, 1);
		assert.strictEqual(matches[0].LONG_NAME.toLowerCase(), 'sample.txt');
		assert.strictEqual(matches[0].IS_DIRECTORY, false);
	});

	await test('findSync progressive mode stops when requested', () => {
		const pattern = path.join(scratch.nested, '*');
		let callbackCount = 0;
		const discovered = fswin.findSync(pattern, file => {
			callbackCount++;
			if (!file.IS_DIRECTORY && file.LONG_NAME.toLowerCase() === 'sample.txt') {
				return true;
			}
		});
		assert.ok(callbackCount >= 1, 'progressive callback should be invoked');
		assert.ok(discovered >= 1, 'return value reports files processed');
	});

	await test('find async basic mode matches sync result set', async () => {
		const syncMatches = fswin.findSync(scratch.wildcard).map(file => file.LONG_NAME).sort();
		const asyncMatches = await new Promise((resolve, reject) => {
			const queued = fswin.find(scratch.wildcard, files => resolve(files.map(file => file.LONG_NAME).sort()));
			if (!queued) {
				return reject(Error('failed to queue async find job'));
			}
		});
		assert.deepStrictEqual(asyncMatches, syncMatches);
	});

	await test('find async progressive mode emits FOUND and SUCCEEDED', async () => {
		await new Promise((resolve, reject) => {
			const events = new Set();
			const timer = setTimeout(() => reject(Error('progressive find timed out')), 15000);
			const queued = fswin.find(
				scratch.wildcard,
				(event, message) => {
					events.add(event);
					if (event === 'FOUND') {
						assert.strictEqual(message.IS_DIRECTORY, false);
					} else if (event === 'SUCCEEDED') {
						clearTimeout(timer);
						assert.ok(events.has('FOUND'));
						resolve();
					} else if (event === 'FAILED') {
						clearTimeout(timer);
						reject(Error('find operation failed unexpectedly'));
					}
				},
				true,
			);
			if (!queued) {
				clearTimeout(timer);
				reject(Error('failed to queue progressive find job'));
			}
		});
	});

	await test('getLogicalDriveList enumerates drives per wiki description', async () => {
		const driveList = fswin.getLogicalDriveListSync();
		assert.ok(driveList && typeof driveList === 'object');
		assert.ok(driveLetter in driveList, 'system drive should be present');
		const asyncDriveList = await fswin.getLogicalDriveListAsync();
		assert.strictEqual(asyncDriveList[driveLetter], driveList[driveLetter]);
	});

	let driveDeviceInfo;
	await test('getDriveDevice returns device metadata', async () => {
		driveDeviceInfo = fswin.getDriveDeviceSync(driveLetterLower);
		assert.ok(driveDeviceInfo);
		assert.ok(driveDeviceInfo.devicePath.startsWith('\\\\'));
		const asyncInfo = await fswin.getDriveDeviceAsync(driveLetterLower);
		assert.strictEqual(asyncInfo.deviceId, driveDeviceInfo.deviceId);
	});

	await test('getDeviceCapabilities surfaces capability flags', async t => {
		if (!driveDeviceInfo || !driveDeviceInfo.parentDeviceId) {
			t.skip('Drive device metadata unavailable');
			return;
		}
		const caps = fswin.getDeviceCapabilitiesSync(driveDeviceInfo.parentDeviceId);
		if (!caps) {
			t.skip('Device capabilities unavailable on this runner');
			return;
		}
		assert.strictEqual(typeof caps.REMOVABLE, 'boolean');
		const asyncCaps = await fswin.getDeviceCapabilitiesAsync(driveDeviceInfo.parentDeviceId);
		assert.strictEqual(asyncCaps.REMOVABLE, caps.REMOVABLE);
	});

	await test('getStorageProperties returns requested information', async t => {
		if (!driveDeviceInfo) {
			t.skip('Drive device metadata unavailable');
			return;
		}
		const selection = {
			deviceProperty: true,
			deviceSeekPenalty: true,
		};
		const props = fswin.getStoragePropertiesSync(driveDeviceInfo.devicePath, selection);
		if (!props) {
			t.skip('Storage properties require elevated permissions');
			return;
		}
		assert.ok(props.deviceProperty);
		const asyncProps = await fswin.getStoragePropertiesAsync(driveDeviceInfo.devicePath, selection);
		assert.deepStrictEqual(asyncProps.deviceSeekPenalty, props.deviceSeekPenalty);
	});

	await test('getVolumeInformation reports filesystem metadata', async () => {
		const info = fswin.getVolumeInformationSync(volumeRoot);
		assert.ok(info);
		assert.strictEqual(typeof info.LABEL, 'string');
		assert.strictEqual(typeof info.FILESYSTEM, 'string');
		const asyncInfo = await fswin.getVolumeInformationAsync(volumeRoot);
		assert.strictEqual(asyncInfo.FILESYSTEM, info.FILESYSTEM);
	});

	await test('getVolumeSpace returns positive totals', async () => {
		const space = fswin.getVolumeSpaceSync(volumeRoot);
		assert.ok(space.TOTAL > 0);
		assert.ok(space.FREE >= 0);
		const asyncSpace = await fswin.getVolumeSpaceAsync(volumeRoot);
		assert.strictEqual(asyncSpace.TOTAL, space.TOTAL);
	});

	await test('setVolumeLabel rejects non-root paths', async () => {
		const resultSync = fswin.setVolumeLabelSync(scratch.nested, 'NOOP');
		assert.strictEqual(resultSync, false);
		const resultAsync = await fswin.setVolumeLabelAsync(scratch.nested, 'NOOP');
		assert.strictEqual(resultAsync, false);
	});

	await test('ejectDrive gracefully fails for unavailable drive', async () => {
		const resultSync = fswin.ejectDriveSync('z', 0);
		assert.strictEqual(resultSync, false);
		const resultAsync = await fswin.ejectDriveAsync('z', 1);
		assert.strictEqual(resultAsync, false);
	});

	await test('ntfs.getCompressedSize reflects file size', async () => {
		const syncSize = fswin.ntfs.getCompressedSizeSync(scratch.file);
		assert.ok(syncSize >= Buffer.byteLength(fileContents));
		const asyncSize = await fswin.ntfs.getCompressedSizeAsync(scratch.file);
		assert.strictEqual(asyncSize, syncSize);
	});

	await test('ntfs.setCompression toggles compression flag', async () => {
		const compressed = fswin.ntfs.setCompressionSync(scratch.file, true);
		assert.strictEqual(compressed, true, 'compression should succeed on scratch file');
		let attrs = fswin.getAttributesSync(scratch.file);
		assert.strictEqual(attrs.IS_COMPRESSED, true);
		const decompressed = await fswin.ntfs.setCompressionAsync(scratch.file, false);
		assert.strictEqual(decompressed, true, 'decompression should succeed on scratch file');
		attrs = fswin.getAttributesSync(scratch.file);
		assert.strictEqual(attrs.IS_COMPRESSED, false);
	});

	await test('ntfs.setSparse marks files with sparse attribute', async () => {
		const resultSync = fswin.ntfs.setSparseSync(scratch.sparseFile);
		assert.strictEqual(resultSync, true);
		let attrs = fswin.getAttributesSync(scratch.sparseFile);
		assert.strictEqual(attrs.IS_SPARSE_FILE, true);
		const resultAsync = await fswin.ntfs.setSparseAsync(scratch.sparseFile);
		assert.strictEqual(resultAsync, true);
		attrs = fswin.getAttributesSync(scratch.sparseFile);
		assert.strictEqual(attrs.IS_SPARSE_FILE, true);
	});

	await test('ntfs.setShortName updates entry when supported', async t => {
		const originalShortPath = fswin.convertPathSync(scratch.shortNameDir);
		if (!originalShortPath) {
			t.skip('Short names disabled on this volume');
			return;
		}
		const originalShortName = path.basename(originalShortPath);
		const desiredShort = 'FSWIN01';
		const succeeded = fswin.ntfs.setShortNameSync(scratch.shortNameDir, desiredShort);
		if (!succeeded) {
			t.skip('Unable to set short name (likely unsupported FS or short names disabled)');
			return;
		}
		try {
			const updatedShortPath = fswin.convertPathSync(scratch.shortNameDir);
			assert.ok(updatedShortPath.toUpperCase().endsWith(`\\${desiredShort}`));
		} finally {
			await fswin.ntfs.setShortNameAsync(scratch.shortNameDir, originalShortName);
		}
	});

	await test('cleanup scratch fixtures', () => {
		fs.rmSync(scratch.root, { recursive: true, force: true });
	});
})();
