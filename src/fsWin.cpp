#include "dirWatcher.h"
#include "splitPath.h"
#include "convertPath.h"
#include "find.h"
#include "getLogicalDriveList.h"
#include "getDriveDevice.h"
#include "getStorageProperties.h"
#include "getDeviceCapabilities.h"
#include "getVolumeInformation.h"
#include "getVolumeSpace.h"
#include "setVolumeLabel.h"
#include "getAttributes.h"
#include "setAttributes.h"
#include "ejectDrive.h"
#include "setShortName.h"
#include "getCompressedSize.h"
#include "setCompression.h"
#include "setSparse.h"

NAPI_MODULE_INIT() {
	napi_value o;
	napi_create_string_latin1(env, "3.23.119", NAPI_AUTO_LENGTH, &o);
	napi_set_named_property(env, exports, "version", o);
	napi_set_named_property(env, exports, "dirWatcher", dirWatcher::init(env));
	napi_set_named_property(env, exports, "splitPath", splitPath::init(env));
	napi_set_named_property(env, exports, "convertPath", convertPath::init(env));
	napi_set_named_property(env, exports, "convertPathSync", convertPath::init(env, true));
	napi_set_named_property(env, exports, "find", find::init(env));
	napi_set_named_property(env, exports, "findSync", find::init(env, true));
	napi_set_named_property(env, exports, "getLogicalDriveList", getLogicalDriveList::init(env));
	napi_set_named_property(env, exports, "getLogicalDriveListSync", getLogicalDriveList::init(env, true));
	napi_set_named_property(env, exports, "getDriveDevice", getDriveDevice::init(env));
	napi_set_named_property(env, exports, "getDriveDeviceSync", getDriveDevice::init(env, true));
	napi_set_named_property(env, exports, "getDeviceCapabilities", getDeviceCapabilities::init(env));
	napi_set_named_property(env, exports, "getDeviceCapabilitiesSync", getDeviceCapabilities::init(env, true));
	napi_set_named_property(env, exports, "getStorageProperties", getStorageProperties::init(env));
	napi_set_named_property(env, exports, "getStoragePropertiesSync", getStorageProperties::init(env, true));
	napi_set_named_property(env, exports, "getVolumeInformation", getVolumeInformation::init(env));
	napi_set_named_property(env, exports, "getVolumeInformationSync", getVolumeInformation::init(env, true));
	napi_set_named_property(env, exports, "getVolumeSpace", getVolumeSpace::init(env));
	napi_set_named_property(env, exports, "getVolumeSpaceSync", getVolumeSpace::init(env, true));
	napi_set_named_property(env, exports, "setVolumeLabel", setVolumeLabel::init(env));
	napi_set_named_property(env, exports, "setVolumeLabelSync", setVolumeLabel::init(env, true));
	napi_set_named_property(env, exports, "getAttributes", getAttributes::init(env));
	napi_set_named_property(env, exports, "getAttributesSync", getAttributes::init(env, true));
	napi_set_named_property(env, exports, "setAttributes", setAttributes::init(env));
	napi_set_named_property(env, exports, "setAttributesSync", setAttributes::init(env, true));
	napi_set_named_property(env, exports, "ejectDrive", ejectDrive::init(env));
	napi_set_named_property(env, exports, "ejectDriveSync", ejectDrive::init(env, true));
	napi_create_object(env, &o);
	napi_set_named_property(env, exports, "ntfs", o);
	napi_set_named_property(env, o, "setShortName", setShortName::init(env));
	napi_set_named_property(env, o, "setShortNameSync", setShortName::init(env, true));
	napi_set_named_property(env, o, "getCompressedSize", getCompressedSize::init(env));
	napi_set_named_property(env, o, "getCompressedSizeSync", getCompressedSize::init(env, true));
	napi_set_named_property(env, o, "setCompression", setCompression::init(env));
	napi_set_named_property(env, o, "setCompressionSync", setCompression::init(env, true));
	napi_set_named_property(env, o, "setSparse", setSparse::init(env));
	napi_set_named_property(env, o, "setSparseSync", setSparse::init(env, true));

	return exports;
}