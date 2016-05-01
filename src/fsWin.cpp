#define FSWIN_VERSION "2.16.501"
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
	SETWITHATTR(target, NEWSTRING("version"), NEWSTRING(FSWIN_VERSION), SYB_ATTR_CONST);
	SETWITHATTR(target, NEWSTRING("convertPath"), convertPath::functionRegister(true), SYB_ATTR_CONST);
	SETWITHATTR(target, NEWSTRING("convertPathSync"), convertPath::functionRegister(false), SYB_ATTR_CONST);
	SETWITHATTR(target, NEWSTRING("dirWatcher"), dirWatcher::functionRegister(), SYB_ATTR_CONST);
	SETWITHATTR(target, NEWSTRING("find"), find::functionRegister(true), SYB_ATTR_CONST);
	SETWITHATTR(target, NEWSTRING("findSync"), find::functionRegister(false), SYB_ATTR_CONST);
	SETWITHATTR(target, NEWSTRING("getVolumeSpace"), getVolumeSpace::functionRegister(true), SYB_ATTR_CONST);
	SETWITHATTR(target, NEWSTRING("getVolumeSpaceSync"), getVolumeSpace::functionRegister(false), SYB_ATTR_CONST);
	SETWITHATTR(target, NEWSTRING("setAttributes"), setAttributes::functionRegister(true), SYB_ATTR_CONST);
	SETWITHATTR(target, NEWSTRING("setAttributesSync"), setAttributes::functionRegister(false), SYB_ATTR_CONST);
	SETWITHATTR(target, NEWSTRING("getAttributes"), getAttributes::functionRegister(true), SYB_ATTR_CONST);
	SETWITHATTR(target, NEWSTRING("getAttributesSync"), getAttributes::functionRegister(false), SYB_ATTR_CONST);
	SETWITHATTR(target, NEWSTRING("splitPath"), splitPath::functionRegister(), SYB_ATTR_CONST);

	RETURNTYPE<Object> ntfsgroup = Object::New(ISOLATE);
	SETWITHATTR(ntfsgroup, NEWSTRING("getCompressedSize"), getCompressedSize::functionRegister(true), SYB_ATTR_CONST);
	SETWITHATTR(ntfsgroup, NEWSTRING("getCompressedSizeSync"), getCompressedSize::functionRegister(false), SYB_ATTR_CONST);
	SETWITHATTR(ntfsgroup, NEWSTRING("setCompression"), setCompression::functionRegister(true), SYB_ATTR_CONST);
	SETWITHATTR(ntfsgroup, NEWSTRING("setCompressionSync"), setCompression::functionRegister(false), SYB_ATTR_CONST);
	SETWITHATTR(ntfsgroup, NEWSTRING("setShortName"), setShortName::functionRegister(true), SYB_ATTR_CONST);
	SETWITHATTR(ntfsgroup, NEWSTRING("setShortNameSync"), setShortName::functionRegister(false), SYB_ATTR_CONST);
	SETWITHATTR(target, NEWSTRING("ntfs"), ntfsgroup, SYB_ATTR_CONST);
}
NODE_MODULE(fswin, moduleRegister);