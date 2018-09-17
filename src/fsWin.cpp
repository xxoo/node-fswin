#include "dirWatcher.h"
#include "convertPath.h"
#include "splitPath.h"
#include "find.h"
#include "getVolumeSize.h"
#include "getAttributes.h"
#include "setAttributes.h"
#include "setShortName.h"
#include "getCompressedSize.h"
#include "setCompression.h"

napi_value init_all(napi_env env, napi_value exports) {
	napi_value o;
	napi_create_string_latin1(env, "3.18.917", NAPI_AUTO_LENGTH, &o);
	napi_set_named_property(env, exports, "version", o);
	napi_set_named_property(env, exports, "dirWatcher", dirWatcher::init(env));
	napi_set_named_property(env, exports, "convertPath", convertPath::init(env));
	napi_set_named_property(env, exports, "convertPathSync", convertPath::init(env, true));
	napi_set_named_property(env, exports, "splitPath", splitPath::init(env));
	napi_set_named_property(env, exports, "find", find::init(env));
	napi_set_named_property(env, exports, "findSync", find::init(env, true));
	napi_set_named_property(env, exports, "getVolumeSize", getVolumeSize::init(env));
	napi_set_named_property(env, exports, "getVolumeSizeSync", getVolumeSize::init(env, true));
	napi_set_named_property(env, exports, "getAttributes", getAttributes::init(env));
	napi_set_named_property(env, exports, "getAttributesSync", getAttributes::init(env, true));
	napi_set_named_property(env, exports, "setAttributes", setAttributes::init(env));
	napi_set_named_property(env, exports, "setAttributesSync", setAttributes::init(env, true));
	napi_create_object(env, &o);
	napi_set_named_property(env, exports, "ntfs", o);
	napi_set_named_property(env, o, "setShortName", setShortName::init(env));
	napi_set_named_property(env, o, "setShortNameSync", setShortName::init(env, true));
	napi_set_named_property(env, o, "getCompressedSize", getCompressedSize::init(env));
	napi_set_named_property(env, o, "getCompressedSizeSync", getCompressedSize::init(env, true));
	napi_set_named_property(env, o, "setCompression", setCompression::init(env));
	napi_set_named_property(env, o, "setCompressionSync", setCompression::init(env, true));
	
	return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, init_all)