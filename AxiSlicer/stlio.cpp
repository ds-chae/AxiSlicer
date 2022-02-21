#include "stdafx.h"
#include "ctype.h"
#include "stdio.h"
#include "stlio.h"

stlfile::stlfile()
{
	_memnew(this, "stlfile", __FILE__, __LINE__);
	h = INVALID_HANDLE_VALUE;
	length = 0;
	next = 0;
	last = 0;
	iseof = 1;
}

stlfile::~stlfile()
{
	_memdel(this);
	if(h != INVALID_HANDLE_VALUE) {
		CloseHandle(h);
		h = INVALID_HANDLE_VALUE;
	}
}


HANDLE stlfile::open(LPCTSTR fn, char* mode)
{
	close();

	if(*mode == 'r')
		h = CreateFile(fn, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	else
		h = CreateFile(fn, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(h != INVALID_HANDLE_VALUE)
		iseof = 0;
	return h;
}

int stlfile::close()
{
	if(h != INVALID_HANDLE_VALUE) {
		int ret = CloseHandle(h);
		h = INVALID_HANDLE_VALUE;
		return ret;
	}
	return 0;
}

char stlfile::_getc()
{
	if(h == INVALID_HANDLE_VALUE)
		return -1;
	if(iseof)
		return -1;
	if(next >= last) {
		DWORD dwio = 0;
		ReadFile(h, buf, sizeof(buf), &dwio, NULL);
		next = 0;
		last = dwio;
		if(next >= dwio) {
			iseof = 1;
			return -1;
		}
	}
	return buf[next++] & 0x00ff;
}

char* stlfile::gets(char* buf, int size)
{
	int dataexist = 0;
	int len = 0;
	size -= 1;

	while(len < size) {
		int c = _getc();
		if(c == -1)
			break;
		dataexist = 1;
		if(c == '\n')
			break;
		if(c != '\r')
			buf[len++] = c;
	}
	if(!dataexist)
		return NULL;

	buf[len] = 0;
	return buf;
}


char* strstrnocase(char* buf, char* s)
{
	while(*buf) {
		char* bp = buf;
		char* cp = s;
		while(*cp) {
			if(toupper(*bp) != toupper(*cp))
				break;
			bp++;
			cp++;
		}
		if(*cp == 0)
			return buf;
		buf++;
	}
	
	return NULL;
}

int scanfloat(char* buf, float* data, int size)
{
	char fs[64];
	float fv = 0;
	int  cnt = 0;
	int  len = 0;
	while(1) {
		if(*buf >= '0' && *buf <= '9' || *buf == '.' || *buf == 'e' || *buf == '-' || *buf == '+') {
			if(len < 16) {
				fs[len++] = *buf;
			}
		} else {
			if(len > 0) {
				fs[len] = 0;
				sscanf_s(fs, "%f", &fv);
				data[cnt++] = fv;
				if(cnt >= size)
					return cnt;
				len = 0;
			}
		}
		if(*buf == 0)
			break;
		buf++;
	}

	return cnt;
}

void stlfile::setpos(int pos)
{
	if(h == INVALID_HANDLE_VALUE)
		return;

	SetFilePointer(h, pos, NULL, FILE_BEGIN);
	this->next = 0;
	this->last = 0;
	this->iseof = 0;
}

int stlfile::read(void* ptr, int size)
{
	DWORD dwio = 0;
	ReadFile(h, ptr, size, &dwio, NULL);
	return dwio;
}

int  stlfile::write(void* buf, int size)
{
	DWORD dwio = 0;
	WriteFile(h, buf, size, &dwio, NULL);
	return dwio;
}

int strcmpnocase(char* s1, char* s2)
{
	while(*s1 && *s2) {
		if(toupper(*s1) != toupper(*s2))
			break;
		s1++;
		s2++;
	}

	return toupper(*s1) - toupper(*s2);
}

int  stlfile::write(vertexmat* mat)
{
	return 0;
}

int tstrcmpnocase(TCHAR* s1, TCHAR* s2)
{
	while(*s1 && *s2) {
		if(toupper(*s1) != toupper(*s2))
			break;
		s1++;
		s2++;
	}

	return toupper(*s1) - toupper(*s2);
}

