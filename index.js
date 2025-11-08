'use strict';
if (process.platform === 'win32') {
	const fswin = require(`./${process.arch}/fswin.node`),
		promisify = (fn, maxArgs, cbPos) => (...args) => new Promise((resolve, reject) => {
			args.splice(maxArgs);
			if (cbPos > args.length) {
				args.push(...Array(cbPos - args.length), resolve);
			} else {
				args.splice(cbPos, 0, resolve);
			}
			fn(...args) || reject(Error('UNABLE_TO_QUEUE_JOB'));
		});
	fswin.convertPathAsync = promisify(fswin.convertPath, 2, 1);
	fswin.ejectDriveAsync = promisify(fswin.ejectDrive, 2, 2);
	fswin.findAsync = promisify(fswin.find, 1, 1);
	fswin.getAttributesAsync = promisify(fswin.getAttributes, 1, 1);
	fswin.getDeviceCapabilitiesAsync = promisify(fswin.getDeviceCapabilities, 1, 1);
	fswin.getDriveDeviceAsync = promisify(fswin.getDriveDevice, 1, 1);
	fswin.getLogicalDriveListAsync = promisify(fswin.getLogicalDriveList, 0, 0);
	fswin.getStoragePropertiesAsync = promisify(fswin.getStorageProperties, 2, 2);
	fswin.getVolumeInformationAsync = promisify(fswin.getVolumeInformation, 1, 1);
	fswin.getVolumeSpaceAsync = promisify(fswin.getVolumeSpace, 1, 1);
	fswin.setAttributesAsync = promisify(fswin.setAttributes, 2, 2);
	fswin.setVolumeLabelAsync = promisify(fswin.setVolumeLabel, 2, 2);
	fswin.ntfs.getCompressedSizeAsync = promisify(fswin.ntfs.getCompressedSize, 1, 1);
	fswin.ntfs.setCompressionAsync = promisify(fswin.ntfs.setCompression, 3, 1);
	fswin.ntfs.setShortNameAsync = promisify(fswin.ntfs.setShortName, 2, 2);
	fswin.ntfs.setSparseAsync = promisify(fswin.ntfs.setSparse, 2, 1);
	module.exports = fswin;
} else {
	module.exports = null;
}