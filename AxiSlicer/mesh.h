#include <vector>

#ifndef _MESH_H_
#define _MESH_H_

#define	ENABLE_GCODE	1
#define	GCODE_LOAD_CALLBACK	0
#define	ENABLE_GUI	0

#define	MATH_PI	3.1415926535

#if _DEBUG
void _memnew(void* _ptr, char* _name, char* _file, int _line);
void _memdel(void* _ptr);
#else
#define	_memnew(a, b, c, d)
#define	_memdel(a)
#endif
void _memdbg();

class printableObject;

typedef struct {
	float x, y, z;
} vertex;

class fpxy
{
public:
	float x;
	float y;

	fpxy()
	{
		x = 0;
		y = 0;
	}

	fpxy(float *fp) { x = fp[0]; y = fp[1]; }
	fpxy(float fx, float fy) { this->x = fx; this->y = fy; }

	fpxy operator -(fpxy& b)
	{
		fpxy r;
		r.x = x - b.x;
		r.y = y - b.y;

		return r;
	}
//	void operator =(const fpxy& b);
	void reverse()
	{
		float t = x;
		x = y;
		y = t;
	}

	int operator ==(const fpxy& b)
	{
		return x == b.x && y == b.y;
	}

	void operator /=(const float f)
	{
		x /= f;
		y /= f;
	}

	fpxy operator /(const float f)
	{
		fpxy nf;
		nf.x = x / f;
		nf.y = y / f;

		return nf;
	}

	void operator *=(const float f)
	{
		x *= f;
		y *= f;
	}

	fpxy operator +(const fpxy& b)
	{
		fpxy nf;

		nf.x = x + b.x;
		nf.y = y + b.y;

		return nf;
	}

	fpxy operator *(const float f)
	{
		fpxy nf;

		nf.x = x * f;
		nf.y = y * f;

		return nf;
	}
};


/*
void fpxy::operator =(const fpxy& b)
{
	x = b.x;
	y = b.y;
}
*/


int _fpxyCompare(const void* p1, const void* p2);

class fpxyvect
{
public:
	fpxy* data;
	int   size;
	int   capacity;

	fpxyvect(int n, char* _file, int _line)
	{
		_memnew(this, "fpxyvect", _file, _line);
		size = n;
		capacity = n;
		if(capacity == 0)
			capacity = 4;
		data = new fpxy[capacity];
	}

	~fpxyvect()
	{
		if(data != NULL)
			delete data;
		_memdel(this);
	}

	void append(fpxy& xy)
	{
		if(size >= capacity) {
			resize(capacity+100);
		}
		data[size] = xy;
		size += 1;
	}

	void append(fpxyvect* fpv)
	{
		for(int n = 0; n < fpv->size; n++) {
			append(fpv->data[n]);
		}
	}

	void resize(int newcap)
	{
		capacity = newcap;
		fpxy* ndata = new fpxy[capacity];
		if(data != NULL) {
			for(int n = 0; n < size; n++)
				ndata[n] = data[n];
			free(data);
		}
		data = ndata;
	}

	fpxyvect* copy(char* _file, int _line)
	{
		fpxyvect* newvect = new fpxyvect(0, _file, _line);
		newvect->resize(size);
		for(int n = 0; n < size; n++) {
			newvect->data[n] = data[n];
		}
		newvect->size = size;
		return newvect;
	}

	void sort()
	{
		qsort(data, size, sizeof(fpxy), _fpxyCompare);
	}

	void del(int idx)
	{
		if(idx < 0)
			idx = size - idx;
		while(idx < size-1) {
			data[idx] = data[idx+1];
			idx++;
		}
		size--;
	}

	fpxy getmin(int dim)
	{
		fpxy mn;

		for(int i = 0; i < this->size; i++) {
			if(i == 0)
				mn = this->data[i];
			else {
				if(mn.x > this->data[i].x)
					mn.x = this->data[i].x;
				if(mn.y > this->data[i].y)
					mn.y = this->data[i].y;
			}
		}

		return mn;
	}

	fpxy getmax(int dim)
	{
		fpxy mn;

		for(int i = 0; i < this->size; i++) {
			if(i == 0)
				mn = this->data[i];
			else {
				if(mn.x < this->data[i].x)
					mn.x = this->data[i].x;
				if(mn.y < this->data[i].y)
					mn.y = this->data[i].y;
			}
		}

		return mn;
	}
};

fpxyvect* operator +(fpxyvect* vect, fpxy& xy);
fpxyvect* operator -(fpxyvect* vect, fpxy& xy);

class fpxyz {
public:
	float data[3];

	fpxyz()
	{
		data[0] = data[1] = data[2] = 0;
	}
	fpxyz(float _x, float _y, float _z)
	{
		data[0] = _x;
		data[1] = _y;
		data[2] = _z;
	}
	fpxyz(float *f) {
		data[0] = f[0];
		data[1] = f[1];
		data[2] = f[2];
	}

	void operator -=(fpxyz& fp)
	{
		data[0] -= fp.data[0];
		data[1] -= fp.data[1];
		data[2] -= fp.data[2];
	}

	void operator +=(fpxyz& fp)
	{
		data[0] += fp.data[0];
		data[1] += fp.data[1];
		data[2] += fp.data[2];
	}

	void operator /=(float f)
	{
		data[0] /= f;
		data[1] /= f;
		data[2] /= f;
	}

	void operator *=(float f)
	{
		data[0] *= f;
		data[1] *= f;
		data[2] *= f;
	}

	fpxyz operator -(fpxyz &b)
	{
		fpxyz nfp;

		nfp.data[0] = data[0] - b.data[0];
		nfp.data[1] = data[1] - b.data[1];
		nfp.data[2] = data[2] - b.data[2];

		return nfp;
	}

	fpxyz operator +(fpxyz &b)
	{
		fpxyz nfp;

		nfp.data[0] = data[0] + b.data[0];
		nfp.data[1] = data[1] + b.data[1];
		nfp.data[2] = data[2] + b.data[2];

		return nfp;
	}

	fpxyz operator /(const float f)
	{
		fpxyz nf;

		nf.data[0] = data[0] / f;
		nf.data[1] = data[1] / f;
		nf.data[2] = data[2] / f;

		return nf;
	}

	fpxyz operator *(const float f)
	{
		fpxyz nf;

		nf.data[0] = data[0] * f;
		nf.data[1] = data[1] * f;
		nf.data[2] = data[2] * f;

		return nf;
	}

	void set(float* f)
	{
		data[0] = f[0];
		data[1] = f[1];
		data[2] = f[2];
	}
} ;

int operator !=(fpxyz &a, fpxyz &b);

/*
class fprect
{
public:
	fpxy data[4];

	fprect(fpxy& p0, fpxy& p1, fpxy& p2, fpxy& p3)
	{
		data[0] = p0;
		data[1] = p1;
		data[2] = p2;
		data[3] = p3;
	}
};

class fprects
{
public:
	int size;
	int capacity;

	fprect *data;

	fprects()
	{
		size = 0;
		capacity = 100;
		data = (fprect*)malloc(sizeof(fprect)*capacity);
	}

	void append(fprect& f)
	{
		if(size >= capacity) {
			capacity += 100;
			fprect* ndata = (fprect*)malloc(sizeof(fprect)*capacity);
			memcpy(ndata, data, sizeof(fprect)*size);
			free(data);
			data = ndata;
		}
		data[size] = f;
		size += 1;
	}
};
*/

class fpxyvectvect
{
public:
	fpxyvect** data;
	int cap;
	int len;

	fpxyvectvect(char* _file, int _line)
	{
		_memnew(this, "fpxyvectvect", _file, _line);
		data = NULL;
		cap = 0;
		len = 0;
	}

	~fpxyvectvect()
	{
		_memdel(this);
		for(int i = 0; i < len; i++)
			delete data[i];
		if(data != NULL)
			free(data);
	}

	void add(fpxyvect* ptr)
	{
		if(len >= cap) {
			cap += 10;
			fpxyvect** ndata = (fpxyvect**)malloc(cap * sizeof(fpxyvect*));
			for(int i = 0; i < len; i++)
				ndata[i] = data[i];
			if(data != NULL)
				free(data);
			data = ndata;
		}
		data[len++] = ptr;
	}
};

//typedef std::vector<float> Vec;
//typedef std::vector<Vec> Mat;

//typedef float (*f3)[3];

class vertexmat
{
public:
	fpxyz *data; // float (*data)[3];
	int size;
	int capacity;
	char* strbuf;

	~vertexmat()
	{
		if(this->data != NULL)
			free(data);
		_memdel(this);
	}

	vertexmat(char* _file, int _line)
	{
		_memnew(this, "vertexmat", _file, _line);
		data = NULL;
		size = 0;
		capacity = 0;
		strbuf = NULL;
	}

	vertexmat(int n, char* _file, int _line)
	{
		_memnew(this, "vertexmat", _file, _line);
		strbuf = NULL;
		size = n;
		capacity = size;
//		data = (f3)malloc(size * sizeof(f3*));
		data = (fpxyz*)malloc(size * sizeof(fpxyz));
	}

	vertexmat* copy(char* _file, int _line)
	{
		vertexmat* vmat = new vertexmat(this->size, _file, _line);
		for(int i = 0; i < this->size; i++)
			vmat->data[i] = this->data[i];
		return vmat;
	}

	void append(float *f)
	{
		append(f[0], f[1], f[2]);
	}

	void append(float x, float y, float z)
	{
		if(size >= capacity) {
			capacity += 1000;
/*
			f3 ndata = (f3)malloc(capacity * sizeof(f3*));
			for(int i = 0; i < size; i++) {
				ndata[i][0] = data[i][0];
			}
*/
			fpxyz* ndata = (fpxyz*)malloc(capacity * sizeof(fpxyz));
			for(int i = 0; i < size; i++) {
				ndata[i] = data[i];
			}
			if(data != NULL)
				free(data);
			data = ndata;
		}
		data[size].data[0] = x;
		data[size].data[1] = y;
		data[size].data[2] = z;

		size += 1;
	}

	char* flatstr(char* delimeter);

	void operator -=(fpxyz& fp)
	{
		for(int i = 0; i < size; i++) {
			data[i] -= fp;
		}
	}

	void operator +=(fpxyz& fp)
	{
		for(int i = 0; i < size; i++) {
			data[i] += fp;
		}
	}

	void operator /=(float f)
	{
		for(int i = 0; i < size; i++) {
			data[i] /= f;
		}
	}

	void operator *=(float f)
	{
		for(int i = 0; i < size; i++) {
			data[i] *= f;
		}
	}

	void operator*=(const vertexmat& b)
	{
		int i, j, k;
		int m = this->size;
		int n = 3;
		int p = 3;

		vertexmat *c = new vertexmat(m, __FILE__, __LINE__);

		for(i = 0; i < m; i++) {
			float row[3];
			for(j = 0; j < p; j++) {
				row[j] = 0;
				for(k = 0; k < n; k++) {
					row[j] += this->data[i].data[k] * b.data[k].data[j];
				}
			}
			for(int ci = 0; ci < 3; ci++) c->data[i].data[ci] = row[ci];
		}

		for(i = 0; i < m; i++) {
			this->data[i] = c->data[i];
		}
		delete c;
	}

	float getnorm(int rs, int re, int cs, int ce)
	{
		float n = 0;
		for(int r = rs; r < re; r++) {
			for(int c = cs; c < ce; c++) {
				n = n + this->data[r].data[c] * this->data[r].data[c];
			}
		}

		return sqrt(n);
	}

	fpxyvect* getxyvect(char* _file, int _line)
	{
		fpxyvect* vect = new fpxyvect(this->size, _file, _line);
		for(int i = 0; i < this->size; i++) {
			fpxy f(this->data[i].data[0], this->data[i].data[1]);
			vect->data[i] = f;
		}

		return vect;
	}

	fpxyz getmin(int dim)
	{
		fpxyz f = this->data[0];
		fpxyz nf;
		nf = f;

		if(this->size < 1)
			return nf;
		
		for(int i = 1; i < this->size; i++ ) {
			fpxyz f = this->data[i];
			if(f.data[0] < nf.data[0])
				nf.data[0] = f.data[0];
			if(f.data[1] < nf.data[1])
				nf.data[1] = f.data[1];
			if(f.data[2] < nf.data[2])
				nf.data[2] = f.data[2];
		}
		return nf;
	}

	fpxyz getmax(int dim)
	{
		fpxyz f = this->data[0];
		fpxyz nf;
		nf = f;

		if(this->size < 1)
			return nf;
		
		for(int i = 1; i < this->size; i++ ) {
			fpxyz f = this->data[i];
			if(f.data[0] > nf.data[0])
				nf.data[0] = f.data[0];
			if(f.data[1] > nf.data[1])
				nf.data[1] = f.data[1];
			if(f.data[2] > nf.data[2])
				nf.data[2] = f.data[2];
		}
		return nf;
	}

	fpxyz getminzvertex()
	{
		fpxyz nf(0,0,0);

		for(int zindex = 0; zindex < this->size; zindex++) {
			if(zindex == 0) {
				nf.set(this->data[zindex].data);
			} else {
				if(this->data[zindex].data[2] < nf.data[2])
					nf.set(this->data[zindex].data);
			}
		}
		return nf;
	}

	void append(vertexmat* v)
	{
		for(int i = 0; i < v->size; i++)
			append(v->data[i].data);
	}
};

/*
class cMat : public Mat
{
public:
	char* strbuf;

	cMat();
	~cMat();
};
*/

class GLVBO;

class mesh
{
public:
	vertexmat *vertexes; // float (*vertexes)[3];
//	cMat* vertexes;
	int vertexCount;
	vertexmat *normal;
	vertexmat *invNormal;

	// VBO is OpenGL Vertex Buffer Object
	GLVBO* vbo;
	//printableObject* _obj;
	printableObject *_obj;

	mesh(printableObject* obj, char* _file, int _line)
	{
		_memnew(this, "mesh", _file, _line);
		vertexes = NULL;
		vertexCount = 0;
		normal = NULL;
		invNormal = NULL;
		vbo = NULL;
		_obj = obj;
	}

	~mesh();

	void _addFace(float x0, float y0, float z0, float x1, float y1, float z1, float x2, float y2, float z2);
	void _prepareFaceCount(int faceNumber);
	void _calculateNormals();
	int _vertexHash(int idx);
	void _idxFromHash(/*map, idx*/);
	vertexmat* getTransformedVertexes(int applyOffsets = 0);
	void split(/*callback*/);
	mesh* copy();
};

class animation;

class MeshList
{
public:
	mesh** data;
	int    len;
	int    cap;

	MeshList(char* _file, int _line)
	{
		_memnew(this, "MeshList", _file, _line);
		cap = 10;
		len = 0;
		data = (mesh**)malloc(cap * sizeof(mesh*));
	}
	~MeshList()
	{
		_memdel(this);
		for(int i = 0; i < len; i++)
			delete data[i];
		free(data);
	}

	void add(mesh* m)
	{
		if(len >= cap) {
			cap += 10;
			mesh** ndata = (mesh**)malloc(cap * sizeof(mesh*));
			for(int i = 0; i < len; i++)
				ndata[i] = data[i];
			free(data);
			data = ndata;
		}
		data[len++] = m;
	}
};


class printableObject
{
public:
	fpxy _headMinSize;
	MeshList* _meshList;
	fpxyvect* _boundaryHull;
	fpxyvect* _headAreaHull;
	fpxyvect* _headAreaMinHull;
	fpxyvect* _printAreaHull;

	fpxyz _transformedMin;
	fpxyz _transformedMax;

	fpxy _position;
	animation* _loadAnim;

	float _boundaryCircleSize;

	fpxyvect* _printAreaExtend;
	fpxyvect* _headAreaExtend;

	printableObject(LPCTSTR originFilename, char* _file, int _line);
	~printableObject();

	printableObject* copy(char* _file, int _line);
	mesh* _addMesh();
	void _postProcessAfterLoad();
	void applyMatrix(vertexmat& m);
	void processMatrix();
	fpxyz getMinimum();
	fpxy getPosition();
	fpxyz getMaximum();
	vertexmat* _matrix;
	fpxyz _drawOffset;
	float* printableObject::getSize();

	fpxyz _transformedSize;

	TCHAR _originFilename[MAX_PATH];
	TCHAR _name[MAX_PATH];
/*
	def getName(self):
		return self._name
	def getOriginFilename(self):
		return self._originFilename
*/
	void setPosition(fpxy& newPos);
	vertexmat* getMatrix();
	fpxyz getDrawOffset();

	float getBoundaryCircle() { return _boundaryCircleSize; }

	void setPrintAreaExtends(fpxyvect *fv);
	void setHeadArea(fpxyvect *poly, fpxy& minSize);
	void mirror(int axis);
	fpxyz getScale();
	void setScale(float scale, int axis, int uniform);
	void setSize(float size, int axis, int uniform);
	void resetScale();
	void resetRotation();
	void layFlat();
	void scaleUpTo(fpxyz& size);
	void split(/*callback*/);
	int  canStoreAsSTL();
	void getVertexIndexList();
};

class printableObjectList {
public:
	printableObject** list;
	printableObjectList(char* _file, int _line)
	{
		_memnew(this, "printableObjectList", _file, _line);
		list = NULL;
		cap = 0;
		size = 0;
	}
	~printableObjectList()
	{
		_memdel(this);
		if(list != NULL) {
			for(int i = 0; i < size; i++) {
				if(list[i] != NULL)
					delete list[i];
			}
		}
	}

	int cap;
	int size;

	void add(printableObject* obj)
	{
		if(size >= cap) {
			cap += 10;
			printableObject** nlist = (printableObject**)malloc(cap * sizeof(printableObject*));
			if(list != NULL) {
				for(int i = 0; i < size; i++)
					nlist[i] = list[i];
				free(list);
			}
			list = nlist;
		}
		list[size++] = obj;
	}
};

class strList
{
public:
	TCHAR** ptrs;
	int size;
	int capacity;

	strList(char* _file, int _line)
	{
		_memnew(this, "strList", _file, _line);
		size = 0;
		capacity = 0;
		ptrs = NULL;
	}

	~strList()
	{
		_memdel(this);
		clear();
	}

	void clear()
	{
		for(int i = 0; i < size; i++) {
			if(ptrs[i] != NULL) free(ptrs[i]);
		}
		if(ptrs != NULL) {
			free(ptrs);
			ptrs = NULL;
		}
		size = 0;
		capacity = 0;
	}

	void add(LPCTSTR s)
	{
		if(size >= capacity) {
			capacity += 100;
			TCHAR** nptrs = (TCHAR**)malloc(capacity * sizeof(TCHAR*));
			int i;
			for( i = 0; i < size; i++)
				nptrs[i] = ptrs[i];
#if _DEBUG
			while(i < capacity) {
				nptrs[i] = (TCHAR*)0x12345678;
				i++;
			}
#endif
			if(ptrs != NULL)
				free(ptrs);
			ptrs = nptrs;
		}

		ptrs[size++] = _tcsdup(s);
	}
#if 0
	void add(char* s)
	{
		int len = strlen(s)+1;
		TCHAR* t = (TCHAR*)malloc(len*sizeof(TCHAR));
		MultiByteToWideChar(CP_ACP, 0, s, len, t, len);
		ptrs.push_back(t);
	}
#endif
	void add(int iv)
	{
		TCHAR s[16];
		wsprintf(s, L"%d", iv);
		add(s);
	}

	TCHAR* pop(int index)
	{
		if(index >= size || index < 0)
			return NULL;

		TCHAR* ret = ptrs[index];
		for(int i = index; i < size-1; i++)
			ptrs[i] = ptrs[i+1];
		size--;

		return ret;
	}

	int split(char* str, char delimeter)
	{
		char line[256];
		TCHAR tline[256];
		int   len = 0;

		clear();
		while(*str) {
			if(*str == delimeter) {
				line[len] = 0;
				MultiByteToWideChar(CP_ACP, 0, line, len+1, tline, sizeof(line));
				add(tline);
				len = 0;
			} else {
				if(len < sizeof(line)-1) {
					line[len++] = *str;
				}
			}
			str++;
		}
		if(len > 0) {
			line[len] = 0;
			MultiByteToWideChar(CP_ACP, 0, line, len+1, tline, sizeof(line));
			add(tline);
		}
		return size;
	}
};

/*
Mat* operator*(const cMat& a, const cMat& b)
{
	int i, j, k;
	int m = a.size();
	int n = 3;
	int p = 3;

	Mat *c = new Mat;

	for(i = 0; i < m; i++) {
		Vec row;
		for(j = 0; j < p; j++) {
			row[j] = 0;
			for(k = 0; k < n; k++) {
				row[j] += a[i][k] * b[k][j];
			}
		}
		c->push_back(row);
	}

	return c;
}
*/

vertexmat* operator*(const vertexmat& a, const vertexmat& b);
void operator-=(vertexmat& a, float *b);
//void operator+=(Mat& a, const Vec& b);
void operator+=(vertexmat& a, float* b);

//typedef std::vector<int> _intvector;
class intvector
{
public:
	int *data;
	int cap;
	int len;

	intvector(char* _file, int _line)
	{
		_memnew(this, "intvector", _file, _line);
		cap = 0;
		len = 0;
		data = NULL;
	}
	~intvector()
	{
		_memdel(this);
		if(data != NULL)
			delete data;
	}

	void add(int v)
	{
		if(len >= cap) {
			cap += 100;
			int* ndata = (int*)malloc(cap * sizeof(int));
			for(int i = 0; i < len; i++)
				ndata[i] = data[i];
			if(data != NULL)
				free(data);
			data = ndata;
		}
		data[len++] = v;
	}

	int getsum()
	{
		int vsum = 0;
		for(unsigned int ni = 0; ni < this->len; ni++) {
			vsum += this->data[ni];
		}

		return vsum;
	};

	int intmax()
	{
		int ret = 0;
		for(int itn = 0; itn < len; itn++) {
			if( itn == 0)
				ret = data[itn];
			else {
				if(data[itn] > ret)
					ret = data[itn];
			}
		}
	
		return ret;
	}

	void resize(int newlen, int v)
	{
		if(newlen <= len) {
			len = newlen;
			return;
		}

		if(newlen > cap) {
			cap = newlen;
			int* ndata = (int*)malloc(cap * sizeof(int));
			for(int i = 0; i < len; i++)
				ndata[i] = data[i];
			if(data != NULL) free(data);
			data = ndata;
		}
		while(len < newlen) {
			data[len++] = v;
		}
	}
};

class fpvector
{
public:
	float *data;
	int cap;
	int len;

	fpvector(char* _file, int _line)
	{
		_memnew(this, "fpvector", _file, _line);
		cap = 0;
		len = 0;
		data = NULL;
	}
	~fpvector()
	{
		_memdel(this);
		if(data != NULL)
			delete data;
	}

	void append(float v)
	{
		if(len >= cap) {
			cap += 100;
			float* ndata = (float*)malloc(cap * sizeof(float));
			for(int i = 0; i < len; i++)
				ndata[i] = data[i];
			if(data != NULL)
				free(data);
			data = ndata;
		}
		data[len++] = v;
	}

	float getsum()
	{
		float vsum = 0;
		for(unsigned int ni = 0; ni < this->len; ni++) {
			vsum += this->data[ni];
		}

		return vsum;
	};

	float intmax()
	{
		float ret = 0;
		for(int itn = 0; itn < len; itn++) {
			if( itn == 0)
				ret = data[itn];
			else {
				if(data[itn] > ret)
					ret = data[itn];
			}
		}
	
		return ret;
	}

	void resize(int newlen, int v)
	{
		if(newlen <= len) {
			len = newlen;
			return;
		}

		if(newlen > cap) {
			cap = newlen;
			float* ndata = (float*)malloc(cap * sizeof(float));
			for(int i = 0; i < len; i++)
				ndata[i] = data[i];
			if(data != NULL) free(data);
			data = ndata;
		}
		while(len < newlen) {
			data[len++] = v;
		}
	}
};

typedef std::vector<intvector*> pintvector;

#define	True	1
#define	False	0

float norm(fpxy& p);

float float_max(float* fa, int start, int end);
/*
class ipxy
{
public:
	int x, y;

	ipxy();
	ipxy(int fx, int fy);
	ipxy(int *ip) { x = ip[0]; y = ip[1]; }

	ipxy operator -(ipxy& b)
	{
		ipxy r;
		r.x = x - b.x;
		r.y = y - b.y;

		return r;
	}
//	void operator =(const fpxy& b);
	void reverse()
	{
		int t = x;
		x = y;
		y = t;
	}
};
*/

/*
class ipxyvect
{
public:
	ipxy* data;
	int   size;
	int   capacity;

	ipxyvect()
	{
		size = 0;
		capacity = 100;
		data = new ipxy[capacity];
	}

	ipxyvect(int n, ipxy xy = ipxy(0,0))
	{
		size = n;
		capacity = n;
		data = new ipxy[capacity];
		for(int i = 0; i < n; i++)
			data[i] = xy;
	}

	void append(ipxy& xy)
	{
		if(size >= capacity) {
			resize(capacity+100);
		}
		data[size] = xy;
		size += 1;
	}

	void append(ipxyvect* fpv)
	{
		for(int n = 0; n < fpv->size; n++) {
			append(fpv->data[n]);
		}
	}

	void resize(int newcap)
	{
		capacity = newcap;
		ipxy* ndata = new ipxy[capacity];
		if(data != NULL) {
			for(int n = 0; n < size; n++)
				ndata[n] = data[n];
			free(data);
		}
		data = ndata;
	}

	ipxyvect* copy()
	{
		ipxyvect* newvect = new ipxyvect();
		newvect->resize(size);
		for(int n = 0; n < size; n++) {
			newvect->data[n] = data[n];
		}
		newvect->size = size;
		return newvect;
	}
//
//	void sort()
//	{
//		qsort(data, size, sizeof(fpxy), _fpxyCompare);
//	}
//
	void del(int idx)
	{
		if(idx < 0)
			idx = size - idx;
		while(idx < size-1) {
			data[idx] = data[idx+1];
			idx++;
		}
		size--;
	}
};
*/

int strcmpnocase(char* s1, char* s2);
TCHAR* getFileExt(TCHAR* filename);
int tstrcmpnocase(TCHAR* s1, TCHAR* s2);
int isMeshLoaderSupportedExtensions(TCHAR* ext);
int isImageToMeshSupportedExtensions(TCHAR* ext);
int isDirectory(TCHAR* filename);
int isFile(LPCTSTR filename);

#endif	// _MESH_H_
