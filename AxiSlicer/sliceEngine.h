#include <windows.h>

//from Cura.util import profile
//from Cura.util import version

extern TCHAR __file__[MAX_PATH];

int FileExist(TCHAR* path);
TCHAR* GetPathFromFile(TCHAR* src, TCHAR* dst);
TCHAR* getEngineFilename();
TCHAR* getTempFilename(TCHAR *path);

#define	PTYPE_INT	0
#define	PTYPE_FLOAT	1
#define	PTYPE_STR	2

class cPair
{
public:
	char* key;
	int   vt;
	char  sv[128];
	int   iv;
	float fv;

	cPair()
	{
		_memnew(this, "cPair", __FILE__, __LINE__);
		vt = -1;
		key = NULL;
	}
	cPair(char* k, int v)
	{
		_memnew(this, "cPair", __FILE__, __LINE__);
		key = _strdup(k);
		iv = v;
		vt = PTYPE_INT;
	}
	cPair(char* k, char* str)
	{
		_memnew(this, "cPair", __FILE__, __LINE__);
		key = _strdup(k);
		strcpy_s(sv, 128, str);
		vt = PTYPE_STR;
	}

	~cPair()
	{
		_memdel(this);
		if(key != NULL)
			free(key);
	}
	
	void set(int v)
	{
		vt = 0;
		iv = v;
	}
	void set(float v)
	{
		vt = 1;
		fv = v;
	}
	void set(char* s)
	{
		vt = 2;
		strcpy_s(sv, 128, s);
	}

	char* tostr()
	{
		switch(vt){
		case 0 :
			sprintf_s(sv, 128, "%d", iv);
			break;
		case 1 :
			sprintf_s(sv, 128, "%f", fv);
		}

		return sv;
	}
};


class cSettings
{
public:
	std::vector<cPair*> pairs;

	cSettings()
	{
		_memnew(this, "cSettings", __FILE__, __LINE__);
	}

	~cSettings()
	{
		_memdel(this);
		std::vector<cPair*>::iterator ite;
		for(ite = pairs.begin(); ite != pairs.end(); ite++) {
			cPair* p = *ite;
			delete p;
		}
	}

	void add(char* key, int iv);
	void add(char* key, char* str);
	int  getInt(char* key);
	float getFloat(char* key);
	cPair* cSettings::find(char* key);

	void set(char* key, int iv);
	void set(char* key, char* str);
};

class Slicer
{
public:
	Slicer(CWnd* parent);
	~Slicer();

	CWnd* _parent;
	TCHAR _binaryStorageFilename[MAX_PATH];
	TCHAR _exportFilename[MAX_PATH];
	CString _sliceLog;
	int   _objCount;
	int   _id;

	float _filamentMM[2];
	int   _printTimeSeconds;

/*
	self._process = None
		self._thread = None
		self._callback = progressCallback
		self._progressSteps = ['inset', 'skin', 'export']
		self._modelHash = None
*/
	void cleanup();
	void abortSlicer();
	void wait();
	TCHAR* getGCodeFilename();
	LPCTSTR getSliceLog();
	int   getID();
	float getFilamentWeight(int e=0);
	char* getFilamentCost(int e=0);
	char* getPrintTime();
	char* getFilamentAmount(int e=0);
	void runSlicer(Scene& scene);
	void _watchProcess(/*commandList, oldThread*/);
	cSettings* _engineSettings(int extruderCount);
	void _runSliceProcess(/*cmdList*/);
	void submitSliceInfoOnline();
};

