#include "stdafx.h"
/*
__copyright__ = "Copyright (C) 2013 David Braam - Released under terms of the AGPLv3 License"

import sys
import math
import os
import time
import numpy
*/
#include "resource.h"
#include "mesh.h"
#include "opengl.h"
#include "stlio.h"
#include "profile.h"
#include "gcodeInterpreter.h"
#include "gcodefilelist.h"

char* _CUSTOM = "_CUSTOM";
char* _FILL = "_FILL";
char* _WALL_INNER = "_WALL_INNER";
char* _SKIRT = "_SKIRT";
char* _SUPPORT = "_SUPPORT";
char* _WALL_OUTER = "_WALL_OUTER";

char *_move = "_move";
char *_extrude = "_extrude";
char *_retract = "_retract";

/*
#class gcodePath(object):
#	def __init__(self, newType, pathType, layerThickness, startPoint):
#		this->type = newType
#		this->pathType = pathType
#		this->layerThickness = layerThickness
#		this->points = [startPoint]
#		this->extrusion = [0.0]
*/

/*
def gcodePath(newType, pathType, layerThickness, startPoint):
	return {'type': newType,
			'pathType': pathType,
			'layerThickness': layerThickness,
			'points': [startPoint],
			'extrusion': [0.0]}
*/
void gcode::loadList(stlfile* l)
{
	this->filename[0] = 0;
	this->_load(l);
}
	
float gcode::calculateWeight()
{
	//#Calculates the weight of the filament in kg
	float radius = atof(profile.getProfileSetting("filament_diameter")) / 2;
	float volumeM3 = (this->extrusionAmount * (MATH_PI * radius * radius)) / (1000*1000*1000);
	return volumeM3 * profile.getPreferenceFloat("filament_physical_density");
}

char _cost_str[128];

char* gcode::calculateCost()
{
	float cost_kg = profile.getPreferenceFloat("filament_cost_kg");
	float cost_meter = profile.getPreferenceFloat("filament_cost_meter");
	if (cost_kg > 0.0 && cost_meter > 0.0) {
		sprintf_s(_cost_str, 128, "%.2f / %.2f",  (this->calculateWeight() * cost_kg, this->extrusionAmount / 1000 * cost_meter));
	} else if (cost_kg > 0.0) {
		sprintf_s(_cost_str, 128, "%.2f", (this->calculateWeight() * cost_kg));
	} else if (cost_meter > 0.0) {
		sprintf_s(_cost_str, 128, "%.2f", (this->extrusionAmount / 1000 * cost_meter));
	} else {
		_cost_str[0] = 0;
	}
	return _cost_str;
}


int startswith(char *s, char* t)
{
	while(*t) {
		if(*s != *t)
			return 0;
		s++;
		t++;
	}

	return 1;
}

char* getPathType(char *s)
{
	if(startswith(s, "CUSTOM"))
		return _CUSTOM;
	if(startswith(s, "FILL"))
		return _FILL;
	if(startswith(s, "WALL-INNER"))
		return _WALL_INNER;
	if(startswith(s, "SKIRT"))
		return _SKIRT;
	if(startswith(s, "SUPPORT"))
		return _SUPPORT;
	if(startswith(s, "WALL-OUTER"))
		return _WALL_OUTER;

	return NULL;
}

char* findcomment(char* line)
{
	while(*line && *line != ';')
		line++;
	if(*line == ';') {
		*line = 0; // hide comment from parser
		return line+1;
	}
	return NULL;
}

void gcode::_load(stlfile* gcodeFile)
{
	this->layerList = new PathListList();
	fpxyz pos = fpxyz(0.0,0.0,0.0);
	fpxyz posOffset = fpxyz(0.0, 0.0, 0.0);
	float currentE = 0.0;
	float totalExtrusion = 0.0;
	float maxExtrusion = 0.0;
	int currentExtruder = 0;
	float extrudeAmountMultiply = 1.0;
	float totalMoveTimeMinute = 0.0;
	int absoluteE = True;
	float scale = 1.0;
	int posAbs = True;
	float feedRate = 3600.0;
	char* moveType = _move;
	float layerThickness = 0.1;
	char* pathType = _CUSTOM;
	fpxyz lastPos = fpxyz(0,0,0);
	SYSTEMTIME st;

	GetSystemTime(&st);
	PathList* currentLayer = new PathList();
	gcodePath* currentPath = new gcodePath(_move, pathType, layerThickness, pos, __FILE__, __LINE__);
	currentPath->extruder = currentExtruder;

	char line[256];
	while(gcodeFile->gets(line, 256) != NULL) {
//		if (type(line) is tuple)
//			line = line[0];

		//#Parse Cura_SF comments
		if( startswith(line, ";TYPE:")) {
			pathType = getPathType(line + 6);
			if(pathType == NULL)
				break;
		}

		char* cp = line;
		char* comment = findcomment(line);
		if(comment != NULL) {
			if(startswith(comment, "fill"))
				pathType = _FILL;
			else if (startswith(comment, "perimeter"))
				pathType = _WALL_INNER;
			else if (startswith(comment, "skirt"))
				pathType = _SKIRT;
			if (startswith(comment, "LAYER:")){
//				fpxyz lastPos = currentPath->points->data[currentPath->points->size-1];
				if(currentPath->points->size > 1)
					currentLayer->append(currentPath);
				else
					delete currentPath;
				if(currentLayer->len > 0)
					layerList->append(currentLayer);
				else
					delete currentLayer;
				currentPath = new gcodePath(moveType, pathType, layerThickness, lastPos, __FILE__, __LINE__);
				currentPath->extruder = currentExtruder;
//						for (path in currentLayer){
//							path['points'] = numpy.array(path['points'], numpy.float32);
//							path['extrusion'] = numpy.array(path['extrusion'], numpy.float32);
//						}
				SYSTEMTIME ct;
				GetSystemTime(&ct);
				if(ct.wSecond != st.wSecond) {
					st = ct;
					if (this->progressCallback != NULL) {
						this->progressCallback->UpdateWindow();//->SendMessage(WM_COMMAND, ID_REPAINT, 0);
					}
				}
				currentLayer = new PathList();
			}
		}
		
		vint T;
		if(getCodeInt(line, 'T', T)) {
			if (currentExtruder > 0) {
				char buf[128];
				sprintf_s(buf, 128, "extruder_offset_x%d",currentExtruder);
				posOffset.data[0] -= profile.getMachineSettingFloat(buf);
				sprintf_s(buf, 128, "extruder_offset_y%d", currentExtruder);
				posOffset.data[1] -= profile.getMachineSettingFloat(buf);
			}
			currentExtruder = T.ivalue;
			if (currentExtruder > 0){
				char buf[128];
				sprintf_s(buf, 128, "extruder_offset_x%d", currentExtruder);
				posOffset.data[0] += profile.getMachineSettingFloat(buf);
				sprintf_s(buf, 128, "extruder_offset_y%d", currentExtruder);
				posOffset.data[1] += profile.getMachineSettingFloat(buf);
			}
		}
			
		vfloat x, y, z, e;
		vint G;
//		fpxyz lastPos;
		if(getCodeInt(line, 'G', G)) {
			switch(G.ivalue){
			case 0 :
			case 1 :
				getCodeFloat(line, 'X', x);
				getCodeFloat(line, 'Y', y);
				getCodeFloat(line, 'Z', z);
				getCodeFloat(line, 'E', e);
				//#f = getCodeFloat(line, 'F');
				//pos = pos[:];
				if (posAbs) {
					if (x.valid) pos.data[0] = x.fvalue * scale + posOffset.data[0];
					if (y.valid) pos.data[1] = y.fvalue * scale + posOffset.data[1];
					if (z.valid) pos.data[2] = z.fvalue * scale + posOffset.data[2];
				} else {
					if (x.valid) pos.data[0] += x.fvalue * scale;
					if (y.valid) pos.data[1] += y.fvalue * scale;
					if (z.valid) pos.data[2] += z.fvalue * scale;
				}
				//#if f is not None:
				//#	feedRate = f
				//#if x is not None or y is not None or z is not None:
				//#	diffX = oldPos[0] - pos[0]
				//#	diffY = oldPos[1] - pos[1]
				//#	totalMoveTimeMinute += math.sqrt(diffX * diffX + diffY * diffY) / feedRate
				moveType = _move;
				if (e.valid){
					if (absoluteE)
						e.fvalue -= currentE;
					if (e.fvalue > 0.0)
						moveType = _extrude;
					if (e.fvalue < 0.0)
						moveType = _retract;
					totalExtrusion += e.fvalue;
					currentE += e.fvalue;
					if (totalExtrusion > maxExtrusion)
						maxExtrusion = totalExtrusion;
				}
				else {
					e.fvalue = 0.0;
				}
				if (moveType == _move && lastPos.data[2] != pos.data[2]) {
					if(e.fvalue > 0) layerThickness = abs(lastPos.data[2] - pos.data[2]);
				}
				if (currentPath->type != moveType || currentPath->pathType != pathType) {
					if(currentPath->points->size > 1)
						currentLayer->append(currentPath);
					else
						delete currentPath;
					currentPath = new gcodePath(moveType, pathType, layerThickness, lastPos, __FILE__, __LINE__);
					currentPath->extruder = currentExtruder;
				}

				if(pos != lastPos) {
					currentPath->points->append(pos.data[0], pos.data[1], pos.data[2]);
					currentPath->extrusion->append(e.fvalue * extrudeAmountMultiply);
				}
				lastPos = pos;
				break;
			case 4 : // #Delay
				vfloat S;
				getCodeFloat(line, 'S', S);
				if (S.valid)
					totalMoveTimeMinute += S.fvalue / 60.0;
				vfloat P;
				getCodeFloat(line, 'P', P);
				if (P.valid)
					totalMoveTimeMinute += P.fvalue / 60.0 / 1000.0;
				break;
			case 10 : //	#Retract
				lastPos = currentPath->points->data[currentPath->points->size-1];
				currentPath = new gcodePath(_retract, pathType, layerThickness, lastPos, __FILE__, __LINE__);
				currentPath->extruder = currentExtruder;
				currentPath->points->append(currentPath->points->data[0].data);
				break;
			case 11 : //:	#Push back after retract
				break;
			case 20 : //	#Units are inches
				scale = 25.4;
				break;
			case 21 : //	#Units are mm
				scale = 1.0;
				break;
			case 28 : //	#Home
				getCodeFloat(line, 'X', x);
				getCodeFloat(line, 'Y', y);
				getCodeFloat(line, 'Z', z);
				{
					fpxyz center = fpxyz(0.0,0.0,0.0);
					if(!x.valid && !y.valid && !z.valid) {
						pos = center; // no coordinate, all home
					} else {
						if (x.valid) // x home
							pos.data[0] = center.data[0];
						if (y.valid) // y home
							pos.data[1] = center.data[1];
						if (z.valid) // z home
							pos.data[2] = center.data[2];
					}
				}
				lastPos = pos;
				break;
			case 90 : // ) {	//#Absolute position
				posAbs = True;
				break;
			case 91 : //) {//	#Relative position
				posAbs = False;
				break;
			case 92 :
				getCodeFloat(line, 'X', x);
				getCodeFloat(line, 'Y', y);
				getCodeFloat(line, 'Z', z);
				getCodeFloat(line, 'E', e);
				if (e.valid)
					currentE = e.fvalue;
				if (x.valid)
					posOffset.data[0] = pos.data[0] - x.fvalue;
				if (y.valid)
					posOffset.data[1] = pos.data[1] - y.fvalue;
				if (z.valid)
					posOffset.data[2] = pos.data[2] - z.fvalue;
				break;
			default:
				//DebugOutputString(OutPutDebugString(print "Unknown G code:" + str(G);
				break;
			}
		}
		else
		{
			vint M;
			if(getCodeInt(line, 'M', M)) {
				switch(M.ivalue) {
				case 0 : // == 0:	//#Message with possible wait (ignored)
				case 1 :	//#Message with possible wait (ignored)
					break;
				case 25:	//#Stop SD printing
				case 80:	//#Enable power supply
				case 81:	//#Suicide/disable power supply
					break;
				case 82:  // #Absolute E
					absoluteE = True;
					break;
				case 83:   //#Relative E
					absoluteE = False;
					break;
				case 84:	//#Disable step drivers
				case 92:	//#Set steps per unit
				case 101:	//#Enable extruder
				case 103:	//#Disable extruder
				case 104:	//#Set temperature, no wait
				case 105:	//#Get temperature
				case 106:	//#Enable fan
				case 107:	//#Disable fan
				case 108:	//#Extruder RPM (these should not be in the final GCode, but they are)
				case 109:	//#Set temperature, wait
				case 110:	//#Reset N counter
				case 113:	//#Extruder PWM (these should not be in the final GCode, but they are)
				case 117:	//#LCD message
				case 140:	//#Set bed temperature
				case 190:	//#Set bed temperature & wait
					break;
				case 221:	//#Extrude amount multiplier
					vfloat s;
					getCodeFloat(line, 'S', s);
					if (s.valid)
						extrudeAmountMultiply = s.fvalue / 100.0;
					break;
				default:
					// print "Unknown M code:" + str(M);
					break;
				}
			}
		}
	}
/*-- this is not needed in c++, as the data is an array already...
     python requires conversion from list to array --
	for (path in currentLayer) {
		path['points'] = numpy.array(path['points'], numpy.float32);
		path['extrusion'] = numpy.array(path['extrusion'], numpy.float32);
	}
*/
	if(currentPath->points->size > 1)
		currentLayer->append(currentPath);
	else
		delete currentPath;
	if(currentLayer->len > 0)
		this->layerList->append(currentLayer);
	else
		delete currentLayer;
/*
	if (this->progressCallback is not None and this->_fileSize > 0){
		this->progressCallback(float(gcodeFile.tell()) / float(this->_fileSize));
	}
*/
	this->extrusionAmount = maxExtrusion;
	this->totalMoveTimeMinute = totalMoveTimeMinute;
	//#print "Extruded a total of: %d mm of filament" % (this->extrusionAmount)
	//#print "Estimated print duration: %.2f minutes" % (this->totalMoveTimeMinute)
	dumpLayers();
}

void gcode::ReloadFiles()
{
	this->layerList = new PathListList();
	fpxyz pos = fpxyz(0.0,0.0,0.0);
	fpxyz posOffset = fpxyz(0.0, 0.0, 0.0);
	float currentE = 0.0;
	float totalExtrusion = 0.0;
	float maxExtrusion = 0.0;
	int currentExtruder = 0;
	float extrudeAmountMultiply = 1.0;
	float totalMoveTimeMinute = 0.0;
	int absoluteE = True;
	float scale = 1.0;
	int posAbs = True;
	float feedRate = 3600.0;
	char* moveType = _move;
	float layerThickness = 0.1;
	char* pathType = _CUSTOM;
	fpxyz lastPos = fpxyz(0,0,0);
	SYSTEMTIME st;

	GetSystemTime(&st);
	PathList* currentLayer = new PathList();
	gcodePath* currentPath = new gcodePath(_move, pathType, layerThickness, pos, __FILE__, __LINE__);
	currentPath->extruder = currentExtruder;

	for(int fi = 0; fi < gcodeFileList.filecount; fi++) {
		stlfile *gcodefile = new stlfile();
		gcodefile->open(gcodeFileList.filenames[fi], "rt");

		float x_offset = gcodeFileList.x_offset[fi];
		float y_offset = gcodeFileList.y_offset[fi];
		if(fi == 0) {
			if(x_offset == OFFSET_UNDEFINED) {
				x_offset = 0;
				y_offset = 0;
			}
		} else {
			if(x_offset == OFFSET_UNDEFINED) {
				x_offset = total_minmax._xmax;
				y_offset = 0;
			}
		}
		gcodeFileList.x_offset[fi] = x_offset;
		gcodeFileList.y_offset[fi] = y_offset;

		char line[256];
		while(gcodefile->gets(line, 256) != NULL) {
			char* cp = line;
			while(1) {
				if(*cp == 0 || *cp == ';' || *cp == '(')
					break;
			
				if(*cp == 'X' || *cp == 'Y' || *cp == 'Z' || *cp == 'x' || *cp == 'y' || *cp == 'z') {
					int axis = *cp++;

					int sign = 1;
					int v = 0;
					int decimal = -1;

					if(*cp == '-') {
						cp++;
						sign = -1;
					}
					if(*cp == '+')
						cp++;

					while(*cp) {
						if( *cp == '.') {
							cp++;
							decimal = 0;
						} else if(*cp >= '0' && *cp <= '9') {
							v = v * 10 + (*cp - '0');
							if(decimal >= 0) decimal++;
							cp++;
						} else {
							break;
						}
					}

					float fv = v * sign;
					float fdiv = 1.0;
					while(decimal > 0) {
						fdiv *= 10;
						decimal--;
					}
					fv /= fdiv;

					switch(axis){
					case 'X':
					case 'x':
						pos.data[0] = fv + x_offset;
						this->file_minmax.testminmax_x(fv);
						this->total_minmax.testminmax_x(pos.data[0]);
						break;
					case 'Y':
					case 'y':
						pos.data[1] = fv + y_offset;
						this->file_minmax.testminmax_y(fv);
						this->total_minmax.testminmax_y(pos.data[1]);
						break;
					case 'Z':
					case 'z':
						pos.data[2] = fv;
						this->file_minmax.testminmax_z(fv);
						this->total_minmax.testminmax_z(fv);
					}
				} else {
					cp++;
				}
			}

			moveType = _move;
			if (currentPath->type != moveType || currentPath->pathType != pathType) {
				if(currentPath->points->size > 1)
					currentLayer->append(currentPath);
				else
					delete currentPath;
				currentPath = new gcodePath(moveType, pathType, layerThickness, lastPos, __FILE__, __LINE__);
				currentPath->extruder = currentExtruder;
			}
			if(pos != lastPos) {
				currentPath->points->append(pos.data[0], pos.data[1], pos.data[2]);
			}
			lastPos = pos;
		}
	}


	if(currentPath->points->size > 1)
		currentLayer->append(currentPath);
	else
		delete currentPath;
	if(currentLayer->len > 0)
		this->layerList->append(currentLayer);
	else
		delete currentLayer;

	this->extrusionAmount = maxExtrusion;
	this->totalMoveTimeMinute = totalMoveTimeMinute;
	//#print "Extruded a total of: %d mm of filament" % (this->extrusionAmount)
	//#print "Estimated print duration: %.2f minutes" % (this->totalMoveTimeMinute)
	dumpLayers();
}

void gcode::dumpLayers()
{
	TCHAR fname[MAX_PATH];
	lstrcpy(fname, this->filename);
	lstrcat(fname, L".txt");
	HANDLE h = CreateFile(fname, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(h == INVALID_HANDLE_VALUE)
		return;

	DWORD dwio = 0;
	char sbuf[128];
	for(int li = 0; li < this->layerList->len; li++) {
		PathList *pl = this->layerList->data[li];
		sprintf_s(sbuf, 128, "LAYER:%d\r\n", li);
		WriteFile(h, sbuf, strlen(sbuf), &dwio, NULL);
		for(int pi = 0; pi < pl->len; pi++) {
			gcodePath* path = pl->data[pi];
			sprintf_s(sbuf, 128, "PATH:%s %s pointsize:%d extsize:%d\r\n",
				path->pathType, path->type, path->points->size, path->extrusion->len);
			WriteFile(h, sbuf, strlen(sbuf), &dwio, NULL);
			vertexmat* points = path->points;
			for(int pidx = 0; pidx < points->size; pidx++) {
				fpxyz f = points->data[pidx];
				sprintf_s(sbuf, 128, "G1 X%f Y%f Z%f\r\n", f.data[0], f.data[1], f.data[2]);
				WriteFile(h, sbuf, strlen(sbuf), &dwio, NULL);
			}
		}
	}
	CloseHandle(h);
}

int getCodeInt(char* line, int code, vint& v)
{
	while(*line) {
		if(*line == code) {
			line++;
			v.ivalue = 0;
			while(*line >= '0' && *line <= '9') {
				v.ivalue = v.ivalue * 10 + (*line - '0');
				line++;
			}
			v.valid = 1;
			return 1;
		}
		line++;
	}

	v.valid = 0;
	return 0;
}

int isfloatnumber(int c)
{
	return (c >= '0' && c <= '9') || c == '.' || c == '+' || c == '-' || c == 'e' || c == 'E';
}

int getCodeFloat(char* line, int code, vfloat& v)
{
	char sbuf[16];
	int  slen = 0;
	while(*line) {
		if(*line == code) {
			line++;
			while(isfloatnumber(*line)) {
				if(slen < 15)
					sbuf[slen++] = *line;
				line++;
			}
			sbuf[slen] = 0;
			sscanf(sbuf, "%f", &v.fvalue);
			v.valid = 1;
			return 1;
		}
		line++;
	}

	v.valid = 0;
	return 0;
}

gcode::gcode(CWnd* callbackWnd)
{
	_memnew(this, "gcode", __FILE__, __LINE__);
	this->regMatch = 0;
	this->layerList = NULL;
	this->extrusionAmount = 0;
	this->totalMoveTimeMinute = 0;
	this->filename[0] = 0;
	this->progressCallback = callbackWnd;
}

gcode::~gcode()
{
	_memdel(this);
	if(this->layerList != NULL)
		delete this->layerList;
}

void gcode::load(TCHAR* filename)
{
	if (isFile(filename)) {
		_tcscpy(this->filename, filename);
		stlfile* gcodeFile = new stlfile();
		HANDLE h = gcodeFile->open(filename, "r");
		if(h != INVALID_HANDLE_VALUE) {
			this->_fileSize = GetFileSize(h, NULL);
			this->_load(gcodeFile);
			gcodeFile->close();
		}
		delete gcodeFile;
	}
}
