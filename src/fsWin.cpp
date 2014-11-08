#define FSWIN_VERSION "2.14.1108"
#include "convertPath.h"
#include "dirWatcher.h"
#include "find.h"
#include "getCompressedSize.h"
#include "getVolumeSize.h"
#include "setAttributes.h"
#include "getAttributes.h"
#include "setCompression.h"
#include "setShortName.h"
#include "splitPath.h"

static void moduleRegister(Handle<Object> target) {
	ISOLATE_NEW;
	SCOPE;
	target->Set(NEWSTRING("version"), NEWSTRING(FSWIN_VERSION), SYB_ATTR_CONST);

	target->Set(NEWSTRING("convertPath"), convertPath::functionRegister(true), SYB_ATTR_CONST);
	target->Set(NEWSTRING("convertPathSync"), convertPath::functionRegister(false), SYB_ATTR_CONST);
	target->Set(NEWSTRING("dirWatcher"), dirWatcher::functionRegister(), SYB_ATTR_CONST);
	target->Set(NEWSTRING("find"), find::functionRegister(true), SYB_ATTR_CONST);
	target->Set(NEWSTRING("findSync"), find::functionRegister(false), SYB_ATTR_CONST);
	target->Set(NEWSTRING("getVolumeSpace"), getVolumeSpace::functionRegister(true), SYB_ATTR_CONST);
	target->Set(NEWSTRING("getVolumeSpaceSync"), getVolumeSpace::functionRegister(false), SYB_ATTR_CONST);
	target->Set(NEWSTRING("setAttributes"), setAttributes::functionRegister(true), SYB_ATTR_CONST);
	target->Set(NEWSTRING("setAttributesSync"), setAttributes::functionRegister(false), SYB_ATTR_CONST);
	target->Set(NEWSTRING("getAttributes"), getAttributes::functionRegister(true), SYB_ATTR_CONST);
	target->Set(NEWSTRING("getAttributesSync"), getAttributes::functionRegister(false), SYB_ATTR_CONST);
	target->Set(NEWSTRING("splitPath"), splitPath::functionRegister(), SYB_ATTR_CONST);

	RETURNTYPE<Object> ntfsgroup = Object::New(ISOLATE);
	ntfsgroup->Set(NEWSTRING("getCompressedSize"), getCompressedSize::functionRegister(true), SYB_ATTR_CONST);
	ntfsgroup->Set(NEWSTRING("getCompressedSizeSync"), getCompressedSize::functionRegister(false), SYB_ATTR_CONST);
	ntfsgroup->Set(NEWSTRING("setCompression"), setCompression::functionRegister(true), SYB_ATTR_CONST);
	ntfsgroup->Set(NEWSTRING("setCompressionSync"), setCompression::functionRegister(false), SYB_ATTR_CONST);
	ntfsgroup->Set(NEWSTRING("setShortName"), setShortName::functionRegister(true), SYB_ATTR_CONST);
	ntfsgroup->Set(NEWSTRING("setShortNameSync"), setShortName::functionRegister(false), SYB_ATTR_CONST);
	target->Set(NEWSTRING("ntfs"), ntfsgroup, SYB_ATTR_CONST);
}
NODE_MODULE(fswin, moduleRegister);