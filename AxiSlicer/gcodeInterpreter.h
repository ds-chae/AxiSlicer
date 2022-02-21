/*
from __future__ import absolute_import
__copyright__ = "Copyright (C) 2013 David Braam - Released under terms of the AGPLv3 License"

import sys
import math
import os
import time
import numpy

from Cura.util import profile
*/
class gcodePath
{
public:
	char* type;
	char* pathType;
	float layerThickness;
	vertexmat* points;
	fpvector* extrusion;
	int extruder;

	gcodePath(char* _newType, char* _pathType, float _layerThickness, fpxyz& _startPoint, char* _file, int _line)
	{
		_memnew(this, "gcodePath", _file, _line);
		this->type = _newType;
		this->pathType = _pathType;
		this->layerThickness = _layerThickness;
		this->points = new vertexmat(__FILE__, __LINE__);
		this->points->append(_startPoint.data[0], _startPoint.data[1], _startPoint.data[2]);
		this->extrusion = new fpvector(__FILE__, __LINE__);
		this->extrusion->append(0.0);
	}

	~gcodePath()
	{
		_memdel(this);
		if(extrusion != NULL)
			delete extrusion;
		if(points != NULL)
			delete points;
	}
};

class PathList
{
public:
	gcodePath **data;
	int  cap;
	int  len;

	PathList()
	{
		_memnew(this, "PathList", __FILE__, __LINE__);
		cap = 0;
		len = 0;
		data = NULL;
	}

	~PathList()
	{
		_memdel(this);
		for(int i = 0; i < len; i++)
			delete data[i];
		if(data != NULL)
			free(data);
	}

	void append(gcodePath* path)
	{
		if(len >= cap) {
			cap += 100;
			gcodePath** ndata = (gcodePath**)malloc(cap * sizeof(gcodePath*));
			for(int i = 0; i < len; i++)
				ndata[i] = data[i];
			if(data != NULL)
				free(data);
			data = ndata;
		}
		data[len++] = path;
	}
};

class PathListList
{
public:
	PathList **data;
	int  cap;
	int  len;

	PathListList()
	{
		_memnew(this, "PathListList", __FILE__, __LINE__);
		cap = 0;
		len = 0;
		data = NULL;
	}

	~PathListList()
	{
		_memdel(this);
		for(int i = 0; i < len; i++)
			delete data[i];
		if(data != NULL)
			free(data);
	}

	void append(PathList* path)
	{
		if(len >= cap) {
			cap += 100;
			PathList** ndata = (PathList**)malloc(cap * sizeof(PathList*));
			for(int i = 0; i < len; i++)
				ndata[i] = data[i];
			if(data != NULL)
				free(data);
			data = ndata;
		}
		data[len++] = path;
	}
};

/*
def gcodePath(newType, pathType, layerThickness, startPoint):
	return {'type': newType,
			'pathType': pathType,
			'layerThickness': layerThickness,
			'points': [startPoint],
			'extrusion': [0.0]}
*/

class c_minmax {
public:
	int xfirst, yfirst, zfirst;

	float _xmin, _xmax;
	float _ymin, _ymax;
	float _zmin, _zmax;

	c_minmax()
	{
		xfirst = 1, yfirst = 1, zfirst = 1;
		_xmin = _xmax = _ymin = _ymax = _zmin = _zmax = 0;
	}

	void testminmax_x(float x)
	{
		if(xfirst) {
			xfirst = 0;
			_xmin = _xmax = x;
		} else {
			if(x < _xmin) _xmin = x;
			if(x > _xmax) _xmax = x;
		}
	}

	void testminmax_y(float y)
	{
		if(yfirst) {
			yfirst = 0;
			_ymin = _ymax = y;
		} else {
			if(y < _ymin) _ymin = y;
			if(y > _ymax) _ymax = y;
		}
	}

	void testminmax_z(float z)
	{
		if(zfirst) {
			zfirst = 0;
			_zmin = _zmax = z;
		} else {
			if(z < _zmin) _zmin = z;
			if(z > _zmax) _zmax = z;
		}
	}
};

class gcode
{
public:
	float regMatch;
	PathListList* layerList;
	float extrusionAmount;
	int totalMoveTimeMinute;
	TCHAR filename[MAX_PATH];
	CWnd *progressCallback;
	int _fileSize;

	c_minmax total_minmax;
	c_minmax file_minmax;

	gcode(CWnd* callbackWnd);
	~gcode();
	void load(TCHAR* filename);
	void loadList(stlfile* l);
	float calculateWeight();
	char* calculateCost();
	void _load(stlfile* gcodeFile);
	void ReloadFiles();
	void dumpLayers();
};

typedef struct {
	int valid;
	int ivalue;
} vint;

typedef struct {
	int valid;
	float fvalue;
} vfloat;

int getCodeInt(char* line, int code, vint& v);
int getCodeFloat(char* line, int code, vfloat& v);
char* getPathType(char *s);

extern char *_move;
extern char *_extrude;
extern char *_retract;

extern char* _CUSTOM;
extern char* _FILL;
extern char* _WALL_INNER;
extern char* _SKIRT;
extern char* _SUPPORT;
extern char* _WALL_OUTER;
