#include "stlio.h"

class cstl
{
public:
	cstl();
	~cstl();

	void _loadAscii(mesh* m, stlfile* f);
	void _loadBinary(mesh* m, stlfile* f);
	printableObjectList* loadScene(TCHAR* filename);
	void saveScene(LPCTSTR filename, printableObjectList* objects);
	void saveSceneStream(FILE* stream, printableObjectList* objects);
};

extern cstl stl;