#define FSWIN_VERSION "0.2.2012.229"
#include "comvertPath.h"
#include "dirWatcher.h"
#include "find.h"
#include "getCompressedSize.h"
#include "getVolumeSize.h"
#include "setAttributes.h"
#include "setCompression.h"
#include "setShortName.h"
#include "splitPath.h"

static void moduleRegister(Handle<Object> target){
	HandleScope scope;
	target->Set(String::NewSymbol("version"),String::NewSymbol(FSWIN_VERSION),global_syb_attr_const);

	target->Set(String::NewSymbol("convertPath"),convertPath::functionRegister(true),global_syb_attr_const);
	target->Set(String::NewSymbol("convertPathSync"),convertPath::functionRegister(false),global_syb_attr_const);
	target->Set(String::NewSymbol("dirWatcher"),dirWatcher::functionRegister(),global_syb_attr_const);
	target->Set(String::NewSymbol("find"),find::functionRegister(true),global_syb_attr_const);
	target->Set(String::NewSymbol("findSync"),find::functionRegister(false),global_syb_attr_const);
	target->Set(String::NewSymbol("getVolumeSpace"),getVolumeSpace::functionRegister(true),global_syb_attr_const);
	target->Set(String::NewSymbol("getVolumeSpaceSync"),getVolumeSpace::functionRegister(false),global_syb_attr_const);
	target->Set(String::NewSymbol("setAttributes"),setAttributes::functionRegister(true),global_syb_attr_const);
	target->Set(String::NewSymbol("setAttributesSync"),setAttributes::functionRegister(false),global_syb_attr_const);
	target->Set(String::NewSymbol("splitPath"),splitPath::functionRegister(),global_syb_attr_const);

	Handle<Object> ntfsgroup=Object::New();
	ntfsgroup->Set(String::NewSymbol("getCompressedSize"),getCompressedSize::functionRegister(true),global_syb_attr_const);
	ntfsgroup->Set(String::NewSymbol("getCompressedSizeSync"),getCompressedSize::functionRegister(false),global_syb_attr_const);
	ntfsgroup->Set(String::NewSymbol("setCompression"),setCompression::functionRegister(true),global_syb_attr_const);
	ntfsgroup->Set(String::NewSymbol("setCompressionSync"),setCompression::functionRegister(false),global_syb_attr_const);
	ntfsgroup->Set(String::NewSymbol("setShortName"),setShortName::functionRegister(true),global_syb_attr_const);
	ntfsgroup->Set(String::NewSymbol("setShortNameSync"),setShortName::functionRegister(false),global_syb_attr_const);
	target->Set(String::NewSymbol("ntfs"),ntfsgroup,global_syb_attr_const);
}
NODE_MODULE(fsWin,moduleRegister);