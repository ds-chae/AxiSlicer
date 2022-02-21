#include "mesh.h"

#ifndef _STLIO_H_
#define	_STLIO_H_

class stlfile
{
public:
	HANDLE h;
	int length;
	int next;
	int last;
	int iseof;
	char buf[4096];

	stlfile();
	~stlfile();

	HANDLE open(LPCTSTR fn, char* mode);
	int close();
	char _getc();
	char *gets(char* buf, int size);
	void setpos(int pos);
	int  read(void*, int size);
	int  write(void* buf, int size);
	int  write(vertexmat* mat);
};

char* strstrnocase(char* buf, char* s);
int scanfloat(char* buf, float* data, int size);
int strcmpnocase(char* s1, char* s2);

#endif	// _STLIO_H_