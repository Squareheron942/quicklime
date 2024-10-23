#include "config.h"
#include <3ds.h>

CFG_Language ql::config::lang	   = CFG_LANGUAGE_JP;
CFG_SystemModel ql::config::model  = CFG_MODEL_3DS;
CFG_Region ql::config::region	   = CFG_REGION_JPN;
bool ql::config::wideIsUnsupported = false;
bool ql::config::isOnCitra		   = false;
bool ql::config::newmodel		   = false;

Result ql::config::getInfo() {
	Result r = cfguInit();

	CFGU_GetSystemModel((u8 *)&model);
	CFGU_GetSystemLanguage((u8 *)&lang);
	CFGU_SecureInfoGetRegion((u8 *)&region);

	cfguExit(); // since it doesn't change at runtime we can just init, read,
				// close

	s64 version = 0;
	svcGetSystemInfo(&version, CITRA_TYPE, CITRA_VERSION); // magic idk
	isOnCitra		  = (version != 0);

	wideIsUnsupported = isOnCitra || model == CFG_MODEL_2DS;
	newmodel = model == CFG_MODEL_N2DSXL || model == CFG_MODEL_N3DSXL ||
			   model == CFG_MODEL_N3DS;

	return r;
}