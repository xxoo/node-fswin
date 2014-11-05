#define FSWIN_VERSION (uint8_t*)"2.14.1105"
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
	Isolate *isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	target->Set(String::NewFromOneByte(isolate, (uint8_t*)"version"), String::NewFromOneByte(isolate, FSWIN_VERSION), SYB_ATTR_CONST);

	target->Set(String::NewFromOneByte(isolate, (uint8_t*)"convertPath"), convertPath::functionRegister(true), SYB_ATTR_CONST);
	target->Set(String::NewFromOneByte(isolate, (uint8_t*)"convertPathSync"), convertPath::functionRegister(false), SYB_ATTR_CONST);
	target->Set(String::NewFromOneByte(isolate, (uint8_t*)"dirWatcher"), dirWatcher::functionRegister(), SYB_ATTR_CONST);
	target->Set(String::NewFromOneByte(isolate, (uint8_t*)"find"), find::functionRegister(true), SYB_ATTR_CONST);
	target->Set(String::NewFromOneByte(isolate, (uint8_t*)"findSync"), find::functionRegister(false), SYB_ATTR_CONST);
	target->Set(String::NewFromOneByte(isolate, (uint8_t*)"getVolumeSpace"), getVolumeSpace::functionRegister(true), SYB_ATTR_CONST);
	target->Set(String::NewFromOneByte(isolate, (uint8_t*)"getVolumeSpaceSync"), getVolumeSpace::functionRegister(false), SYB_ATTR_CONST);
	target->Set(String::NewFromOneByte(isolate, (uint8_t*)"setAttributes"), setAttributes::functionRegister(true), SYB_ATTR_CONST);
	target->Set(String::NewFromOneByte(isolate, (uint8_t*)"setAttributesSync"), setAttributes::functionRegister(false), SYB_ATTR_CONST);
	target->Set(String::NewFromOneByte(isolate, (uint8_t*)"getAttributes"), getAttributes::functionRegister(true), SYB_ATTR_CONST);
	target->Set(String::NewFromOneByte(isolate, (uint8_t*)"getAttributesSync"), getAttributes::functionRegister(false), SYB_ATTR_CONST);
	target->Set(String::NewFromOneByte(isolate, (uint8_t*)"splitPath"), splitPath::functionRegister(), SYB_ATTR_CONST);

	Local<Object> ntfsgroup = Object::New(isolate);
	ntfsgroup->Set(String::NewFromOneByte(isolate, (uint8_t*)"getCompressedSize"), getCompressedSize::functionRegister(true), SYB_ATTR_CONST);
	ntfsgroup->Set(String::NewFromOneByte(isolate, (uint8_t*)"getCompressedSizeSync"), getCompressedSize::functionRegister(false), SYB_ATTR_CONST);
	ntfsgroup->Set(String::NewFromOneByte(isolate, (uint8_t*)"setCompression"), setCompression::functionRegister(true), SYB_ATTR_CONST);
	ntfsgroup->Set(String::NewFromOneByte(isolate, (uint8_t*)"setCompressionSync"), setCompression::functionRegister(false), SYB_ATTR_CONST);
	ntfsgroup->Set(String::NewFromOneByte(isolate, (uint8_t*)"setShortName"), setShortName::functionRegister(true), SYB_ATTR_CONST);
	ntfsgroup->Set(String::NewFromOneByte(isolate, (uint8_t*)"setShortNameSync"), setShortName::functionRegister(false), SYB_ATTR_CONST);
	target->Set(String::NewFromOneByte(isolate, (uint8_t*)"ntfs"), ntfsgroup, SYB_ATTR_CONST);
}
NODE_MODULE(fswin, moduleRegister);