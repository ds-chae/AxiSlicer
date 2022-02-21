#include "stdafx.h"
#include "gcodefilelist.h"

CGCODEFileList gcodeFileList;

// CGCODEFileList
CGCODEFileList::CGCODEFileList()
{
	filecount = 0;
}

#define	OFFSET_UNDEFINED	-9999

void CGCODEFileList::addFilePath(LPCTSTR strPathName)
{
	if(filecount >= MAX_GCODE_FILES)
		return;
	
	lstrcpy(filenames[filecount], strPathName);
	if(filecount == 0) {
		x_offset[filecount] = 0;
		y_offset[filecount] = 0;
	} else {
		x_offset[filecount] = OFFSET_UNDEFINED;
		y_offset[filecount] = OFFSET_UNDEFINED;
	}

	filecount++;
}

