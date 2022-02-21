#define	MAX_GCODE_FILES	20
#define	OFFSET_UNDEFINED	-9999

class CGCODEFileList {
public:
	TCHAR filenames[MAX_GCODE_FILES][MAX_PATH];
	int filecount;
	float x_offset[MAX_GCODE_FILES];
	float y_offset[MAX_GCODE_FILES];

	CGCODEFileList();
	void addFilePath(LPCTSTR strPathName);
};

extern CGCODEFileList gcodeFileList;
