#include "stdafx.h"
#include "windows.h"
#include "mesh.h"
#include "stlio.h"
#include "cstl.h"
#include <vector>

cstl stl;

cstl::cstl()
{
}

cstl::~cstl()
{
}

#define	HALF_FACES	0

void cstl::_loadAscii(mesh* m, stlfile* f)
{
	char buf[256];
	float data[3][3];
	int cnt = 0;
	int facecount = 0;

	while(f->gets(buf, sizeof(buf)) != NULL) {
		char* pstr = strstrnocase(buf, "vertex");
		if(pstr) {
			scanfloat(pstr+6, data[cnt], 3);
			cnt++;
			if(cnt == 3) {
				facecount++;
				cnt = 0;
			}
		}
	}

#if HALF_FACES
	int fc = 0;
	m->_prepareFaceCount(facecount);
	f->setpos(80);
	while(f->gets(buf, sizeof(buf)) != NULL) {
		char* pstr = strstrnocase(buf, "vertex");
		if(pstr) {
			scanfloat(pstr+6, data[cnt], 3);
			cnt++;
			if(cnt == 3) {
				if(((fc / 100) % 2) == 1) {
					m->_addFace(data[0][0], data[0][1], data[0][2], data[1][0], data[1][1], data[1][2], data[2][0], data[2][1], data[2][2]);
				}
				fc++;
				cnt = 0;
			}
		}
	}
#else
	m->_prepareFaceCount(facecount);
	f->setpos(80);
	while(f->gets(buf, sizeof(buf)) != NULL) {
		char* pstr = strstrnocase(buf, "vertex");
		if(pstr) {
			scanfloat(pstr+6, data[cnt], 3);
			cnt++;
			if(cnt == 3) {
				m->_addFace(data[0][0], data[0][1], data[0][2], data[1][0], data[1][1], data[1][2], data[2][0], data[2][1], data[2][2]);
				cnt = 0;
			}
		}
	}
#endif
}

void cstl::_loadBinary(mesh* m, stlfile* f)
{
	int faceCount;
	
	// Skip the header
	f->setpos(80);
	f->read(&faceCount, 4);
	m->_prepareFaceCount(faceCount);
	for(int idx = 0; idx < faceCount; idx++) {
		float data[16];
		f->read(data, 50);
		m->_addFace(data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11]);
	}
}

printableObjectList* cstl::loadScene(TCHAR* filename)
{
	printableObjectList *pol = new printableObjectList(__FILE__, __LINE__);
	
	printableObject* obj = new printableObject(filename, __FILE__, __LINE__);
	mesh* m = obj->_addMesh();

	stlfile f;
	if(f.open(filename, "r") == INVALID_HANDLE_VALUE) {
		delete obj;
		return pol;
	}

	char str6[6];
	f.read(str6, 5);
	str6[5] = 0;
	if(strcmpnocase(str6, "solid") == 0) {
		_loadAscii(m, &f);
		if( m->vertexCount < 3) {
			f.setpos(5);
			_loadBinary(m, &f);
		}
	} else {
		_loadBinary(m, &f);
	}
	f.close();

	obj->_postProcessAfterLoad();
	pol->add(obj);

	return pol;
}

typedef std::vector<printableObject*> tobjects;

void saveSceneStream(stlfile& stream, tobjects& objects);

void saveScene(LPCTSTR filename, tobjects& objects)
{
	stlfile f;
	if(f.open(filename, "wb") != INVALID_HANDLE_VALUE) {
		saveSceneStream(f, objects);
		f.close();
	}
}

void saveSceneStream(stlfile& stream, tobjects& objects)
{
	// #Write the STL binary header. This can contain any info, except for "SOLID" at the start.
	char buf[128];
	SYSTEMTIME st;
	GetLocalTime(&st);
	sprintf_s(buf, 128, "AXISLICER BINARY STL EXPORT. %04d%02d%02d %02d:%02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	stream.write(buf, 80);

	int vertexCount = 0;
	for(int ite = 0; ite < objects.size(); ite++) {
		printableObject* obj = objects.data()[ite];

		for(int item = 0; item < obj->_meshList->len; item++) {
			mesh* m = obj->_meshList->data[item];
			vertexCount += m->vertexCount;
		}
	}
	// #Next follow 4 binary bytes containing the amount of faces, and then the face information.
	vertexCount /= 3;
	stream.write( &vertexCount, 4);

	for(int ite = 0; ite < objects.size(); ite++) {
		printableObject* obj = objects.data()[ite];
		for(int item = 0; item < obj->_meshList->len; item++) {
			mesh* m = obj->_meshList->data[item];
			vertexmat* vertexes = m->getTransformedVertexes(1);
			for(int idx =0; idx < m->vertexCount; idx += 3) {
				vertex v0 = { 0, 0, 0 };
				stream.write(&v0.x, 4); stream.write(&v0.y, 4); stream.write(&v0.z, 4);
				fpxyz v1 = vertexes->data[idx];
				fpxyz v2 = vertexes->data[idx+1];
				fpxyz v3 = vertexes->data[idx+2];
				stream.write(&v1.data[0], 4); stream.write(&v1.data[1], 4); stream.write(&v1.data[2], 4);
				stream.write(&v2.data[0], 4); stream.write(&v2.data[1], 4); stream.write(&v2.data[2], 4);
				stream.write(&v3.data[0], 4); stream.write(&v3.data[1], 4); stream.write(&v3.data[2], 4);
				stream.write("\0\0", 2);
			}
			delete vertexes;
		}
	}
}
