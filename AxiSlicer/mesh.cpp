#include "stdafx.h"
//#include "numpy.h"
#include "mesh.h"
#include "polygon.h"
#include "opengl.h"

typedef struct {
	void  *ptr;
	char* name;
	char* file;
	int   line;
} memdebug;

#define	MEMDBG_SIZE	100000

memdebug debugtbl[MEMDBG_SIZE];
int memcount = 0;

#if _DEBUG
void _memnew(void* _ptr, char* _name, char* _file, int _line)
{
	if(memcount < MEMDBG_SIZE) {
		debugtbl[memcount].ptr = _ptr;
		debugtbl[memcount].name = _name;
		debugtbl[memcount].file = _file;
		debugtbl[memcount].line = _line;
		memcount++;
	}
}

void _memdel(void* _ptr)
{
	for(int i = 0; i < memcount; i++) {
		if(debugtbl[i].ptr == _ptr)
			debugtbl[i].ptr = NULL;
	}
}
#endif

void _memdbg()
{
#if _DEBUG
	for(int i = 0; i < memcount; i++) {
		if(debugtbl[i].ptr != NULL) {
			char sbuf[256];
			sprintf_s(sbuf, 256, "%s : %s,%d\r\n", debugtbl[i].name, debugtbl[i].file, debugtbl[i].line);
			TCHAR tbuf[256];
			MultiByteToWideChar(CP_ACP, 0, sbuf, strlen(sbuf)+1, tbuf, 256);
			OutputDebugString(tbuf);
		}
	}
#endif
}

int _fpxyCompare(const void* p1, const void* p2)
{
	fpxy* fp1 = (fpxy*)p1;
	fpxy* fp2 = (fpxy*)p2;

	if(fp1->x > fp2->x)
		return 1;

	if(fp1->x == fp2->x) {
		if(fp1->y > fp2->y)
			return 1;
		if(fp1->y == fp2->y)
			return 0;
		return -1;
	}

	return -1;
}

TCHAR* _basename(LPTSTR path)
{
	int i = lstrlen(path);
	while(path[i])
		i++;
	while(i > 0) {
		i--;
		if(path[i] == '\\')
			break;
	}

	if(path[i] == '\\')
		return (path + i + 1);

	return path;
}

printableObject* gObject;

printableObject::printableObject(LPCTSTR originFilename, char* _file, int _line)
{
	gObject = this;
	_memnew(this, "printableObject", _file, _line);
	if(originFilename == NULL)
		_originFilename[0] = 0;
	else
		lstrcpy(this->_originFilename, originFilename);

	if (_originFilename[0] == 0)
		lstrcpy(this->_name, _T("None"));
	else
		lstrcpy(this->_name, _basename(_originFilename));

	LPTSTR pstr = StrChr(_name, '.');
	if(pstr != NULL)
		*pstr = 0; // get first name part

	this->_meshList = new MeshList(__FILE__, __LINE__);

	this->_position = fpxy(0, 0);
	this->_matrix = new vertexmat(__FILE__, __LINE__);
	_matrix->append(1,0,0);
	_matrix->append(0,1,0);
	_matrix->append(0,0,1);

	this->_boundaryCircleSize = 0;
	this->_drawOffset = fpxyz(0,0, 0);
	this->_boundaryHull = NULL;
	this->_printAreaExtend = new fpxyvect(0, __FILE__, __LINE__);
	_printAreaExtend->append(fpxy(-1,-1));
	_printAreaExtend->append(fpxy(1,-1));
	_printAreaExtend->append(fpxy(1, 1));
	_printAreaExtend->append(fpxy(-1, 1));

	this->_headAreaExtend = new fpxyvect(0, __FILE__, __LINE__);
	_headAreaExtend->append(fpxy(-1,-1));
	_headAreaExtend->append(fpxy( 1,-1));
	_headAreaExtend->append(fpxy( 1, 1));
	_headAreaExtend->append(fpxy(-1, 1));

	_headMinSize = fpxy(1, 1);
	_printAreaHull = NULL;
	_headAreaHull = NULL;
	_headAreaMinHull = NULL;

	this->_loadAnim = NULL;
}

printableObject::~printableObject()
{
	_memdel(this);

	if(_meshList != NULL)
		delete _meshList;

	if(_boundaryHull != NULL)
		delete _boundaryHull;

	if(NULL != this->_matrix)
		delete this->_matrix;
	if(NULL != this->_printAreaExtend)
		delete this->_printAreaExtend;
	if(NULL != this->_headAreaExtend)
		delete this->_headAreaExtend;
	if(NULL != this->_headAreaMinHull)
		delete this->_headAreaMinHull;
	if(this->_printAreaHull != NULL)
		delete this->_printAreaHull;
	if(this->_headAreaHull != NULL)
		delete this->_headAreaHull;
}

printableObject* printableObject::copy(char* _file, int _line)
{
	printableObject* ret = new printableObject(this->_originFilename, _file, _line);
	ret->_matrix = this->_matrix->copy(__FILE__, __LINE__);
	ret->_transformedMin = this->_transformedMin;
	ret->_transformedMax = this->_transformedMax;
	ret->_transformedSize = this->_transformedSize;
	ret->_boundaryCircleSize = this->_boundaryCircleSize;
	ret->_boundaryHull = this->_boundaryHull->copy(__FILE__, __LINE__);
	ret->_printAreaExtend = this->_printAreaExtend->copy(__FILE__, __LINE__);
	ret->_printAreaHull = this->_printAreaHull->copy(__FILE__, __LINE__);
	ret->_drawOffset = this->_drawOffset;
	for( int midx = 0; midx < this->_meshList->len; midx++) {
		mesh *m = _meshList->data[midx];
		mesh* m2 = m->copy();
		m2->vertexCount = m->vertexCount;
		ret->_meshList->add(m2);
/*		ret->_addMesh();
		m2->vbo = m.vbo;
		m2->vbo.incRef(); */
	}
	
	return ret;
}

mesh* printableObject::_addMesh()
{
	mesh* m = new mesh(this, __FILE__, __LINE__);
	this->_meshList->add(m);

	return m;
}

void printableObject::_postProcessAfterLoad()
{
	for(int im = 0; im < this->_meshList->len; im++) {
		mesh* m = _meshList->data[im];
		m->_calculateNormals();
	}

	this->processMatrix();
	if (float_max(this->getSize(), 0, 3) > 10000.0) {
		for(int im = 0; im < this->_meshList->len; im++) {
			mesh* m = _meshList->data[im];
			*m->vertexes /= 1000.0;
		}
		this->processMatrix();
	}
	if (float_max(this->getSize(), 0, 3) < 1.0) {
		for(int im = 0; im < this->_meshList->len; im++) {
			mesh* m = _meshList->data[im];
			*m->vertexes *= 1000.0;
		}
		this->processMatrix();
	}
}

void printableObject::applyMatrix(vertexmat& m)
{
	*this->_matrix *= m;
	this->processMatrix();
}

float squaresum(fpxyz& f)
{
	float fv = f.data[0]*f.data[0] + f.data[1]*f.data[1] + f.data[2]*f.data[2];

	return fv;
}

void printableObject::processMatrix()
{
	this->_transformedMin = fpxyz(999999999999,999999999999,999999999999);
	this->_transformedMax = fpxyz(-999999999999,-999999999999,-999999999999);
	this->_boundaryCircleSize = 0;

	fpxyvect* hull = new fpxyvect(0, __FILE__, __LINE__); // numpy.zeros((0, 2), numpy.int);
	for(int im = 0; im < _meshList->len; im++){
		mesh *m = _meshList->data[im];
		vertexmat* transformedVertexes = m->getTransformedVertexes();
		fpxyvect* xyvect = transformedVertexes->getxyvect(__FILE__, __LINE__); // transformedVertexes[:,0:2]
		xyvect->append(hull);

		if(hull != NULL) { delete hull; hull = NULL; }
		hull = polygon.convexHull(xyvect, __FILE__, __LINE__);
		delete xyvect; xyvect = NULL;
		fpxyz transformedMin = transformedVertexes->getmin(0); // minimum of 0th dimension
		fpxyz transformedMax = transformedVertexes->getmax(0);
		for(int n = 0; n < 3; n++) {
			this->_transformedMin.data[n] = min(transformedMin.data[n], this->_transformedMin.data[n]);
			this->_transformedMax.data[n] = max(transformedMax.data[n], this->_transformedMax.data[n]);
		}
//			#Calculate the boundary circle
		fpxyz transformedSize = transformedMax - transformedMin;
		fpxyz center = transformedSize / 2.0;
		center += transformedMin;

		// fucking python method...
		//float boundaryCircleSize = round(sqrt(max(((transformedVertexes->get[::,0] - center[0]) * (transformedVertexes[::,0] - center[0])) + ((transformedVertexes[::,1] - center[1]) * (transformedVertexes[::,1] - center[1])) + ((transformedVertexes[::,2] - center[2]) * (transformedVertexes[::,2] - center[2])))), 3);
		// if cpp, it is
		float boundaryCircleSize = 0;
		for(int ri = 0; ri < transformedVertexes->size; ri++) {
			fpxyz fp = transformedVertexes->data[ri];
			fp -= center;
			float sqv = squaresum(fp);
			if(ri == 0)
				boundaryCircleSize = sqv;
			else {
				if(boundaryCircleSize < sqv)
					boundaryCircleSize = sqv;
			}
		}
		boundaryCircleSize = sqrt(boundaryCircleSize);

		this->_boundaryCircleSize = max(this->_boundaryCircleSize, boundaryCircleSize);

		delete transformedVertexes;
	}

	this->_transformedSize = this->_transformedMax - this->_transformedMin;
	this->_drawOffset = (this->_transformedMax + this->_transformedMin) / 2;
	this->_drawOffset.data[2] = this->_transformedMin.data[2]; // no Z movement.
	this->_transformedMax -= this->_drawOffset;
	this->_transformedMin -= this->_drawOffset;

	fpxyvect* bvect = new fpxyvect(0, __FILE__, __LINE__);
	bvect->append(fpxy(-1,-1));
	bvect->append(fpxy(-1,1));
	bvect->append(fpxy(1,1));
	bvect->append(fpxy(1,-1));
	fpxy offset = fpxy(this->_drawOffset.data[0], this->_drawOffset.data[1]);
	fpxyvect* ovect = hull - offset;
	if(this->_boundaryHull != NULL)
		delete this->_boundaryHull ;
	this->_boundaryHull = polygon.minkowskiHull(ovect, bvect, __FILE__, __LINE__);
	if(this->_printAreaHull != NULL)
		delete this->_printAreaHull ;
	this->_printAreaHull = polygon.minkowskiHull(this->_boundaryHull, this->_printAreaExtend, __FILE__, __LINE__);
	this->setHeadArea(this->_headAreaExtend, this->_headMinSize);
	delete ovect;
	delete bvect;

	if(hull != NULL) { delete hull; hull = NULL; }
}

/*
	def getName(self):
		return this->_name
	def getOriginFilename(self):
		return this->_originFilename
*/
fpxy printableObject::getPosition()
{
	return this->_position;
}

void printableObject::setPosition(fpxy& newPos)
{
	_position = newPos;
}

vertexmat* printableObject::getMatrix()
{
	return this->_matrix;
}

fpxyz printableObject::getMaximum()
{
	return this->_transformedMax;
}

fpxyz printableObject::getMinimum()
{
	return this->_transformedMin;
}

float* printableObject::getSize()
{
	return this->_transformedSize.data;
}

fpxyz printableObject::getDrawOffset()
{
	return this->_drawOffset;
}

/*
	def getBoundaryCircle(self):
		return this->_boundaryCircleSize
*/
void printableObject::setPrintAreaExtends(fpxyvect* poly)
{
	if(this->_printAreaExtend != NULL)
		delete this->_printAreaExtend;
	this->_printAreaExtend = poly->copy(__FILE__, __LINE__);
	if(this->_printAreaHull != NULL)
		delete this->_printAreaHull;
	this->_printAreaHull = polygon.minkowskiHull(this->_boundaryHull, this->_printAreaExtend, __FILE__, __LINE__);

	this->setHeadArea(this->_headAreaExtend, this->_headMinSize);
}

void printableObject::setHeadArea(fpxyvect* poly, fpxy& minSize)
{
	if(this->_headAreaExtend != poly) {
		if(this->_headAreaExtend != NULL)
			delete this->_headAreaExtend;
		this->_headAreaExtend = poly->copy(__FILE__, __LINE__);
	}
	this->_headMinSize = minSize;
	if(this->_headAreaHull != NULL)
		delete this->_headAreaHull;
	this->_headAreaHull = polygon.minkowskiHull(this->_printAreaHull, this->_headAreaExtend, __FILE__, __LINE__);
	//pMin = numpy.min(this->_printAreaHull, 0) - this->_headMinSize;
	//pMax = numpy.max(this->_printAreaHull, 0) + this->_headMinSize;
	fpxy pMin = this->_printAreaHull->getmin(0) - this->_headMinSize;
	fpxy pMax = this->_printAreaHull->getmax(0) + this->_headMinSize;
	fpxyvect* square = new fpxyvect(0, __FILE__, __LINE__);
	square->append(pMin);
	square->append(fpxy(pMin.x, pMax.y));
	square->append( pMax);
	square->append(fpxy(pMax.x, pMin.y));

	if(this->_headAreaMinHull != NULL)
		delete this->_headAreaMinHull ;
	this->_headAreaMinHull = polygon.clipConvex(this->_headAreaHull, square);
	delete square;
}

void printableObject::mirror(int axis)
{
	vertexmat* matrix = new vertexmat(__FILE__, __LINE__);
	matrix->append(1,0,0);
	matrix->append(0, 1, 0);
	matrix->append(0, 0, 1);
	matrix->data[axis].data[axis] = -1;
	
	this->applyMatrix(*matrix);
	delete matrix;
}

fpxyz printableObject::getScale()
{
/*		return numpy.array([
			numpy.linalg.norm(this->_matrix[::,0].getA().flatten()),
			numpy.linalg.norm(this->_matrix[::,1].getA().flatten()),
			numpy.linalg.norm(this->_matrix[::,2].getA().flatten())], numpy.float64);
			*/
	float xnorm = _matrix->getnorm(0, _matrix->size, 0, 1);
	float ynorm = _matrix->getnorm(0, _matrix->size, 1, 2);
	float znorm = _matrix->getnorm(0, _matrix->size, 2, 3);

	return fpxyz(xnorm, ynorm, znorm);
}

void printableObject::setScale(float scale, int axis, int uniform)
{
	float currentScale = _matrix->getnorm(0, _matrix->size, axis, axis+1); // get normal by axis - x,y,z
	scale /= currentScale;
	if (scale == 0)
		return;
	vertexmat* matrix = new vertexmat(__FILE__, __LINE__);
	if (uniform) {
		matrix->append(scale,0,0);
		matrix->append(0, scale, 0);
		matrix->append(0, 0, scale);
	} else {
		matrix->append(1.0,0,0);
		matrix->append(0, 1.0, 0);
		matrix->append(0, 0, 1.0);
		matrix->data[axis].data[axis] = scale;
	}

	this->applyMatrix(*matrix);

	delete matrix;
}

void printableObject::setSize(float size, int axis, int uniform)
{
	float scale = this->getSize()[axis];
	scale = size / scale;
	if (scale == 0)
		return;
	vertexmat *matrix = new vertexmat(__FILE__, __LINE__);
	if (uniform) {
		matrix->append(scale,0,0);
		matrix->append(0, scale, 0);
		matrix->append(0, 0, scale);
	} else {
		matrix->append(1,0,0);
		matrix->append(0, 1, 0);
		matrix->append(0, 0, 1);
		matrix->data[axis].data[axis] = scale;
	}

	this->applyMatrix(*matrix);

	delete matrix;
}

void printableObject::resetScale()
{
	float x = 1 / _matrix->getnorm(0,3, 0,1);
	float y = 1 / _matrix->getnorm(0,3, 1,2);
	float z = 1 / _matrix->getnorm(0,3, 2,3);

	vertexmat *matrix = new vertexmat(__FILE__, __LINE__);
	matrix->append(x,0,0);
	matrix->append(0,y,0);
	matrix->append(0,0,z);

	this->applyMatrix(*matrix);

	delete matrix;
}

void printableObject::resetRotation()
{
	float x = this->_matrix->getnorm(0,_matrix->size, 0, 1);
	float y = this->_matrix->getnorm(0,_matrix->size, 1, 2);
	float z = this->_matrix->getnorm(0,_matrix->size, 2, 3);

	this->_matrix->data[0] = fpxyz(x,0,0);
	this->_matrix->data[0] = fpxyz(0,y,0);
	this->_matrix->data[0] = fpxyz(0,0,z);

	this->processMatrix();
}

void printableObject::layFlat()
{
	vertexmat* transformedVertexes = this->_meshList->data[0]->getTransformedVertexes();
	fpxyz minZvertex = transformedVertexes->getminzvertex();

	float dotMin = 1.0;
	fpxyz dotV;
	int   dotVfound = 0;
	for(int vn = 0; vn < transformedVertexes->size; vn++){
		fpxyz v =  transformedVertexes->data[vn];
		fpxyz diff = v - minZvertex;
		float len = sqrt(diff.data[0] * diff.data[0] + diff.data[1] * diff.data[1] + diff.data[2] * diff.data[2]);
		if (len < 5)
			continue;
		float dot = (diff.data[2] / len);
		if (dotMin > dot) {
			dotMin = dot;
			dotV = diff;
			dotVfound = 1;
		}
	}
	delete transformedVertexes;
	if (!dotVfound )
		return;

	float rad = -atan2(dotV.data[1], dotV.data[0]);
	vertexmat *mat = new vertexmat(__FILE__, __LINE__);
	mat->append(cos(rad), sin(rad), 0);
	mat->append(-sin(rad), cos(rad), 0);
	mat->append(0,0,1);
	*this->_matrix *= *mat;
	delete mat;

	rad = -asin(dotMin);
	vertexmat *mat2 = new vertexmat(__FILE__, __LINE__);
	mat2->append(cos(rad), 0, sin(rad));
	mat2->append(0,1,0);
	mat2->append(-sin(rad), 0, cos(rad));
	*this->_matrix *= *mat2;
	delete mat2;
	
	transformedVertexes = this->_meshList->data[0]->getTransformedVertexes();

	minZvertex = transformedVertexes->getminzvertex();
	dotMin = 1.0;
	dotVfound = 0;

	for(int vn = 0; vn < transformedVertexes->size; vn++) {
		fpxyz v = transformedVertexes->data[vn];
		fpxyz diff = v - minZvertex;
		float len = sqrt(diff.data[1] * diff.data[1] + diff.data[2] * diff.data[2]);
		if (len < 5)
			continue;
		float dot = (diff.data[2] / len);
		if (dotMin > dot) {
			dotMin = dot;
			dotV = diff;
			dotVfound = 1;
		}
	}
	delete transformedVertexes;
	if (!dotVfound)
		return;
	if (dotV.data[1] < 0) {
		rad = asin(dotMin);
	} else {
		rad = -asin(dotMin);
	}

	mat2->data[0] = fpxyz(1,0,0);
	mat2->data[1] = fpxyz(0, cos(rad), sin(rad));
	mat2->data[2] = fpxyz(0, -sin(rad), cos(rad));
	this->applyMatrix(*mat2);

	delete mat2;
}

void printableObject::scaleUpTo(fpxyz& size)
{
	fpxyz vMin = this->_transformedMin;
	fpxyz vMax = this->_transformedMax;

	float scaleX1 = (size.data[0] / 2 - this->_position.x) / ((vMax.data[0] - vMin.data[0]) / 2);
	float scaleY1 = (size.data[1] / 2 - this->_position.y) / ((vMax.data[1] - vMin.data[1]) / 2);
	float scaleX2 = (this->_position.x + size.data[0] / 2) / ((vMax.data[0] - vMin.data[0]) / 2);
	float scaleY2 = (this->_position.y + size.data[1] / 2) / ((vMax.data[1] - vMin.data[1]) / 2);
	float scaleZ = size.data[2] / (vMax.data[2] - vMin.data[2]);
	float scale = scaleX1;
	if(scaleY1 < scale)
		scale = scaleY1;
	if(scaleX2 < scale)
		scale = scaleX2;
	if(scaleY2 < scale)
		scale = scaleY2;
	if(scaleZ < scale)
		scale = scaleZ;
	if (scale > 0) {
		vertexmat *mat = new vertexmat(__FILE__, __LINE__);
		mat->append(scale,0,0);
		mat->append(0,scale,0);
		mat->append(0,0,scale);
		this->applyMatrix(*mat);
		delete mat;
	}
}

// Split splits an object with multiple meshes into different objects, where each object is a part of the original mesh that has
// connected faces. This is useful to split up plate STL files.
/*
void printableObject::split(callback)
{
	ret = [];
	for oriMesh in this->_meshList:
		ret += oriMesh.split(callback);
	return ret;
}
*/

int printableObject::canStoreAsSTL()
{
	return this->_meshList->len < 2;
}

/*
// getVertexIndexList returns an array of vertexes, and an integer array for each mesh in this object.
// the integer arrays are indexes into the vertex array for each triangle in the model.
void printableObject::getVertexIndexList()
{
	vertexMap = {};
	vertexmat* vertexList = new vertexmat;
	intvect* meshList = new intvect;
	for(int midx = 0; midx <  _meshList->size; midx++) {
		mesh* m = _meshList->data()[midx];
		verts = m.getTransformedVertexes(True)
		meshIdxList = []
		for idx in xrange(0, len(verts)):{
			v = verts[idx]
			hashNr = int(v[0] * 100) | int(v[1] * 100) << 10 | int(v[2] * 100) << 20
			vIdx = None
			if hashNr in vertexMap:{
				for idx2 in vertexMap[hashNr]:{
					if numpy.linalg.norm(v - vertexList[idx2]) < 0.001:
						vIdx = idx2
				}
			}
			if vIdx is None:{
				vIdx = len(vertexList)
				vertexMap[hashNr] = [vIdx]
				vertexList.append(v)
			}
			meshIdxList.append(vIdx)
		}
		meshList.append(numpy.array(meshIdxList, numpy.int32))
	}

	return numpy.array(vertexList, numpy.float32), meshList;
}
*/

void mesh::_addFace(float x0, float y0, float z0, float x1, float y1, float z1, float x2, float y2, float z2)
{
	int n = vertexCount;
	vertexes->data[n] = fpxyz(x0, y0, z0);
	n += 1;
	vertexes->data[n] = fpxyz(x1, y1, z1);
	n += 1;
	vertexes->data[n] = fpxyz(x2, y2, z2);

	vertexCount += 3;
}

void mesh::_prepareFaceCount(int faceNumber)
{
//		#Set the amount of faces before loading data in them. This way we can create the numpy arrays before we fill them.
#if 1
	if(vertexes != NULL)
		delete vertexes;
	vertexes = new vertexmat(faceNumber*3, __FILE__, __LINE__);
	if(normal != NULL)
		delete normal;
	this->normal = new vertexmat(faceNumber*3, __FILE__, __LINE__);
	this->vertexCount = 0;
#else
	this->vertexes = numpy.zeros((faceNumber*3, 3), numpy.float32)
	this->normal = numpy.zeros((faceNumber*3, 3), numpy.float32)
	this->vertexCount = 0
#endif
}

void mesh::_calculateNormals()
{
	if(normal != NULL)
		delete normal;
	this->normal = new vertexmat(this->vertexes->size, __FILE__, __LINE__);

	if(invNormal != NULL)
		delete invNormal;
	this->invNormal = new vertexmat(this->vertexes->size, __FILE__, __LINE__);

	for(int n = 0; n < this->vertexes->size; n += 3) {
		fpxyz v0 = this->vertexes->data[n];
		fpxyz a = this->vertexes->data[n+1];
		fpxyz b = this->vertexes->data[n+2];
		a -= v0;
		b -= v0;
		fpxyz normals;
		// a x b = (ajbk - bjak, akbi - bkai, aibj-biaj)
		normals.data[0] = a.data[1] * b.data[2] - b.data[1] * a.data[2];
		normals.data[1] = a.data[2] * b.data[0] - b.data[2] * a.data[0];
		normals.data[2] = a.data[0] * b.data[1] - b.data[0] * a.data[1];

		float lens = sqrt(normals.data[0] * normals.data[0] + normals.data[1] * normals.data[1] + normals.data[2] * normals.data[2]);
		normals /= lens;

		this->normal->data[n] = normals;
		this->normal->data[n+1] = normals;
		this->normal->data[n+2] = normals;

		this->invNormal->data[n] = normals * -1;
		this->invNormal->data[n+1] = normals * -1;
		this->invNormal->data[n+2] = normals * -1;
	}
/*
//		#Calculate the normals
	tris = this->vertexes.reshape(this->vertexCount / 3, 3, 3);
	normals = numpy.cross( tris[::,1 ] - tris[::,0]  , tris[::,2 ] - tris[::,0] );
	lens = numpy.sqrt( normals[:,0]**2 + normals[:,1]**2 + normals[:,2]**2 );
	normals[:,0] /= lens;
	normals[:,1] /= lens;
	normals[:,2] /= lens;
		
	n = numpy.zeros((this->vertexCount / 3, 9), numpy.float32);
	n[:,0:3] = normals;
	n[:,3:6] = normals;
	n[:,6:9] = normals;
	this->normal = n.reshape(this->vertexCount, 3);
	this->invNormal = -this->normal;
*/
}

int mesh::_vertexHash(int idx)
{
	fpxyz v = this->vertexes->data[idx];
	return int(v.data[0] * 100) | int(v.data[1] * 100) << 10 | int(v.data[2] * 100) << 20;
}

/*
void mesh::_idxFromHash(map, idx)
{
	vHash = this->_vertexHash(idx);
	for i in map[vHash]:{
		if numpy.linalg.norm(this->vertexes[i] - this->vertexes[idx]) < 0.001:
			return i;
	}
}
*/

vertexmat* mesh::getTransformedVertexes(int applyOffsets)
{
	if (applyOffsets){
		fpxy pxy = this->_obj->_position;
		fpxyz pos = fpxyz(pxy.x, pxy.y, 0);
		pos.data[2] = this->_obj->getSize()[2] / 2;
		fpxyz offset = this->_obj->_drawOffset;
		offset.data[2] += this->_obj->getSize()[2] / 2;
		
		vertexmat* vertex = this->vertexes->copy(__FILE__, __LINE__);
		*vertex *= *this->_obj->_matrix;
		*vertex -= offset;
		*vertex += pos;
		return vertex;
	}

	vertexmat* vertex = this->vertexes->copy(__FILE__, __LINE__);
	*vertex *= *this->_obj->_matrix;
	return vertex;
}

/*
void mesh::split(self, callback)
{
	vertexMap = {}

	vertexToFace = []
	for idx in xrange(0, this->vertexCount):{
		if (idx % 100) == 0:
			callback(idx * 100 / this->vertexCount)
		vHash = this->_vertexHash(idx)
		if vHash not in vertexMap:
			vertexMap[vHash] = []
		vertexMap[vHash].append(idx)
		vertexToFace.append([])
	}
	faceList = []
	for idx in xrange(0, this->vertexCount, 3):{
		if (idx % 100) == 0:
			callback(idx * 100 / this->vertexCount)
		f = [this->_idxFromHash(vertexMap, idx), this->_idxFromHash(vertexMap, idx+1), this->_idxFromHash(vertexMap, idx+2)]
		vertexToFace[f[0]].append(idx / 3)
		vertexToFace[f[1]].append(idx / 3)
		vertexToFace[f[2]].append(idx / 3)
		faceList.append(f)
	}
	ret = []
	doneSet = set()
	for idx in xrange(0, len(faceList)):{
		if idx in doneSet:
			continue
		doneSet.add(idx)
		todoList = [idx]
		meshFaceList = []
		while len(todoList) > 0:{
			idx = todoList.pop()
			meshFaceList.append(idx)
			for n in xrange(0, 3):{
				for i in vertexToFace[faceList[idx][n]]:{
					if not i in doneSet:{
						doneSet.add(i)
						todoList.append(i)
					}
				}
			}
		}
		obj = printableObject(this->_obj.getOriginFilename())
		obj._matrix = this->_obj._matrix.copy()
		m = obj._addMesh()
		m._prepareFaceCount(len(meshFaceList))
		for idx in meshFaceList:{
			m.vertexes[m.vertexCount] = this->vertexes[faceList[idx][0]]
			m.vertexCount += 1
			m.vertexes[m.vertexCount] = this->vertexes[faceList[idx][1]]
			m.vertexCount += 1
			m.vertexes[m.vertexCount] = this->vertexes[faceList[idx][2]]
			m.vertexCount += 1
		}
		obj._postProcessAfterLoad();
		ret.append(obj);
	}
	return ret;
}
*/

char* vertexmat::flatstr(char* delimeter)
{
	if(this->strbuf != NULL)
		free(strbuf);
	// calculate the size of strbuf
	int strsize = 0;
	for(int i = 0; i < size; i++) {
		fpxyz v = data[i];
		char tmp[16];
		sprintf_s(tmp, 16, "%f%s%f%s%f%s", v.data[0], delimeter, v.data[1], delimeter, v.data[2], delimeter);
		strsize += strlen(tmp);
	}

	strbuf = (char*)malloc(strsize);
	char* sp = strbuf;
	for(int i = 0; i < size; i++) {
		fpxyz v = data[i];
		char tmp[16];
		sprintf_s(tmp, 16, "%f%s%f%s%f%s", v.data[0], delimeter, v.data[1], delimeter, v.data[2], delimeter);
		strcpy_s(sp, strsize, tmp);
		strsize += strlen(tmp);
		if(i != size-1) {
			strcpy_s(sp, strsize, delimeter);
			sp += strlen(delimeter);
		}
	}
	*sp = 0;
	return strbuf;
}


mesh* mesh::copy()
{
	mesh *m = new mesh(this->_obj, __FILE__, __LINE__);
	m->vertexCount = this->vertexCount;
	m->vertexes = new vertexmat(vertexCount, __FILE__, __LINE__);
	for(int i = 0; i < this->vertexes->size; i++) {
		m->vertexes->data[i] = this->vertexes->data[i];
	}
	
	return m;
}

fpxyvect* operator +(fpxyvect* vect, fpxy& xy)
{
	fpxyvect *ret = vect->copy(__FILE__, __LINE__);
	for(int n = 0; n < ret->size; n++) {
		ret->data[n].x += xy.x;
		ret->data[n].y += xy.y;
	}

	return ret;
}

fpxyvect* operator -(fpxyvect* vect, fpxy& xy)
{
	fpxyvect *ret = vect->copy(__FILE__, __LINE__);
	for(int n = 0; n < ret->size; n++) {
		ret->data[n].x -= xy.x;
		ret->data[n].y -= xy.y;
	}

	return ret;
}

vertexmat* operator*(const vertexmat& a, const vertexmat& b)
{
	int i, j, k;
	int m = a.size;
	int n = 3;
	int p = 3;

	vertexmat *c = new vertexmat(m, __FILE__, __LINE__);

	for(i = 0; i < m; i++) {
		float row[3];
		for(j = 0; j < p; j++) {
			row[j] = 0;
			for(k = 0; k < n; k++) {
				row[j] += a.data[i].data[k] * b.data[k].data[j];
			}
		}
		for(int ci = 0; ci < 3; ci++) c->data[i].data[ci] = row[ci];
	}

	return c;
}

void operator-=(vertexmat& a, float *b)
{
	int i, j;
	int m = a.size;

	for(i = 0; i < m; i++) {
		for(j = 0; j < 3; j++) {
			a.data[i].data[j] -= b[j];
		}
	}
}


/*
void operator+=(Mat& a, const Vec& b)
{
	int i, j;
	int m = a.size();
	int n = b.size();

	for(i = 0; i < m; i++) {
		for(j = 0; j < n; j++) {
			a[i][j] += b[j];
		}
	}
}
*/

void operator+=(vertexmat& a, float* b)
{
	int i, j;
	int m = a.size;

	for(i = 0; i < m; i++) {
		for(j = 0; j < 3; j++) {
			a.data[i].data[j] += b[j];
		}
	}
}

mesh::~mesh()
{
	_memdel(this);
	if(vertexes != NULL)
		delete vertexes;
	vertexCount = 0;
	if(normal != NULL)
		delete normal;
	if(invNormal != NULL)
		delete invNormal;

	if(vbo != NULL)
		delete vbo;
};


int operator !=(fpxyz &a, fpxyz &b)
{
	return a.data[0] != b.data[0] || a.data[1] != b.data[1] || a.data[2] != b.data[2];
}

