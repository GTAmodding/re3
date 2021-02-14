#include "common.h"

#include "Camera.h"
#include "ModelInfo.h"
#include "General.h"

CTimeModelInfo*
CTimeModelInfo::FindOtherTimeModel(void)
{
	char name[40];
	char *p;
	int i;

	strcpy(name, GetModelName());
	// change _nt to _dy
	if(p = strstr(name, "_nt"))
		strncpy(p, "_dy", 4);
	// change _dy to _nt
	else if(p = strstr(name, "_dy"))
		strncpy(p, "_nt", 4);
	else
		return nil;

	for(i = 0; i < MODELINFOSIZE; i++){
		CBaseModelInfo *mi = CModelInfo::GetModelInfo(i);
		if (mi && mi->GetModelType() == MITYPE_TIME &&
		   !CGeneral::faststrncmp(name, mi->GetModelName(), MAX_MODEL_NAME)){
			m_otherTimeModelID = i;
			return (CTimeModelInfo*)mi;
		}
	}
	return nil;
}
