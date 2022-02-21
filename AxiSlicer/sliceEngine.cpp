#include "stdafx.h"
#include <windows.h>
#include "mesh.h"
#include "objectScene.h"
#include "sliceEngine.h"
#include "profile.h"
#include "stlio.h"
#include "stdio.h"

/*
__copyright__ = "Copyright (C) 2013 David Braam - Released under terms of the AGPLv3 License"
import subprocess
import time
import math
import numpy
import os
import warnings
import threading
import traceback
import platform
import sys
import urllib
import urllib2
import hashlib
*/

//from Cura.util import profile
//from Cura.util import version

static TCHAR _getEngineFilename[MAX_PATH];
TCHAR __file__[MAX_PATH];

int FileExist(TCHAR* path)
{
	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(path, &wfd);
	if(hFind != INVALID_HANDLE_VALUE) {
		FindClose(hFind);
		return 1;
	}

	return 0;
}


TCHAR* GetPathFromFile(TCHAR* src, TCHAR* dst)
{
	lstrcpy(dst, src);
	TCHAR* cp = dst;
	while(*cp) cp++;
	while(cp != dst) {
		cp--;
		if(*cp == '\\') {
			*cp = 0;
			break;
		}
	}

	return dst;
}


TCHAR* getEngineFilename()
{
	if(FileExist(TEXT("C:/Software/Cura_SteamEngine/_bin/Release/Cura_SteamEngine.exe")))
		return TEXT("C:/Software/Cura_SteamEngine/_bin/Release/Cura_SteamEngine.exe");

	GetPathFromFile(__file__, _getEngineFilename);
	lstrcat(_getEngineFilename, TEXT("\\CuraEngine.exe"));
	return _getEngineFilename;

	/*
	if hasattr(sys, 'frozen'):
		return os.path.abspath(os.path.join(os.path.dirname(__file__), '../../../../..', 'CuraEngine'))
	if os.path.isfile('/usr/bin/CuraEngine'):
		return '/usr/bin/CuraEngine'
	if os.path.isfile('/usr/local/bin/CuraEngine'):
		return '/usr/local/bin/CuraEngine'
	return os.path.abspath(os.path.join(os.path.dirname(__file__), '../..', 'CuraEngine'))
*/
}

TCHAR* getTempFilename(TCHAR *path)
{
	TCHAR tempPath[MAX_PATH];
	GetTempPath(MAX_PATH, tempPath);
	GetTempFileName(tempPath, TEXT("AxiTemp"), 1, path);
	return path;
}

Slicer::Slicer(CWnd* parent)
{
	this->_parent = parent;
	getTempFilename(_binaryStorageFilename);
	getTempFilename(_exportFilename);
	_objCount = 0;
	_id = 0;

	_filamentMM[0] = 0.0;
	_filamentMM[1] = 0.0;
	_printTimeSeconds = 0;

/*
	this->_process = None
		this->_thread = None
		this->_callback = progressCallback
		this->_progressSteps = ['inset', 'skin', 'export']
		this->_printTimeSeconds = None
		this->_modelHash = None
*/
}

Slicer::~Slicer()
{
}

void Slicer::cleanup()
{
	abortSlicer();
	DeleteFile(_binaryStorageFilename);
	DeleteFile(_exportFilename);
}

void Slicer::abortSlicer()
{
	/*
	if this->_process is not None:
			try:
				this->_process.terminate()
			except:
				pass
			this->_thread.join()
		this->_thread = None
*/
}


void wait()
{
	/*
	if this->_thread is not None:
			this->_thread.join()
*/
}


TCHAR* Slicer::getGCodeFilename()
{
	return _exportFilename;
}

LPCTSTR Slicer::getSliceLog()
{
	return _sliceLog;
}


int Slicer::getID()
{
	return _id;
}

float Slicer::getFilamentWeight(int e)
{
	//		#Calculates the weight of the filament in kg
	float	radius = atof(profile.getProfileSetting("filament_diameter")) / 2;
	float	volumeM3 = (this->_filamentMM[e] * (MATH_PI * radius * radius)) / (1000*1000*1000);
	return  volumeM3 * profile.getPreferenceFloat("filament_physical_density");
}


char* Slicer::getFilamentCost(int e)
{
	static char _coststr[128];

	float cost_kg = profile.getPreferenceFloat("filament_cost_kg");
	float cost_meter = profile.getPreferenceFloat("filament_cost_meter");
	if (cost_kg > 0.0 && cost_meter > 0.0) {
		sprintf_s(_coststr, 128, "%.2f / %.2f", this->getFilamentWeight(e) * cost_kg, this->_filamentMM[e] / 1000.0 * cost_meter);
		return _coststr;
	}

	if( cost_kg > 0.0) {
		sprintf_s(_coststr, 128, "%.2f", this->getFilamentWeight(e) * cost_kg);
		return _coststr;
	}

	if( cost_meter > 0.0) {
		sprintf_s(_coststr, 128, "%.2f", this->_filamentMM[e] / 1000.0 * cost_meter);
		return _coststr;
	}

	return NULL;
}

char* Slicer::getPrintTime()
{
	static char buf[128];
	int t = this->_printTimeSeconds / 60 / 60;
	if(t < 1) {
		sprintf_s(buf, 128, "%d minutes", (int)((this->_printTimeSeconds / 60) % 60));
		return buf;
	} else
	if(t == 1) {
		sprintf_s(buf, 128, "%d hour %d minutes", (int)(this->_printTimeSeconds / 60 / 60), (int)(this->_printTimeSeconds / 60) % 60);
	}
	else
		sprintf_s(buf, 128, "%d hours %d minutes", (int)(this->_printTimeSeconds / 60 / 60), (int)(this->_printTimeSeconds / 60) % 60);

	return buf;
}

char* Slicer::getFilamentAmount(int e)
{
	if( this->_filamentMM[e] == 0.0)
		return "";
	
	static char buf[128];
	sprintf_s(buf, 128, "%0.2f meter %0.0f gram", this->_filamentMM[e] / 1000.0, this->getFilamentWeight(e) * 1000.0);
	return buf;
}

class farray
{
public:
	int dimension;
	int dimsize[3];
	float *data;

	farray();
	farray(farray& fa);
	~farray();
	
	void operator+=(float a);
	void operator=(float a);
};

farray::farray()
{
	dimension = 0;
	data = NULL;
}

void farray::operator +=(float a)
{
	float *fp = data;
	if(dimension < 1)
		return;
	
	int totsize = dimsize[0];
	for(int i = 1; i < dimension; i++)
		totsize *= dimsize[i];
	for(int t = 0; t < totsize; t++)
		data[t] += a;
}

void farray::operator =(float a)
{
	float *fp = data;
	if(dimension < 1)
		return;
	
	int totsize = dimsize[0];
	for(int i = 1; i < dimension; i++)
		totsize *= dimsize[i];
	for(int t = 0; t < totsize; t++)
		data[t] = a;
}

HANDLE _process = NULL;
HANDLE oldThread = NULL;
CString commandName;
CString commandPara;

void Slicer::runSlicer(Scene& scene)
{
	if(scene._objectList->length < 1)
		return;

	int extruderCount = 1;
//	std::vector<mesh>::iterator item; - vector samle
	for(int iten = 0; iten < scene._objectList->length; iten++) {
		printableObject* obj = scene._objectList->data[iten];
		if( scene.checkPlatform(obj))
			extruderCount = max(extruderCount, obj->_meshList->len);

		extruderCount = max(extruderCount, profile.minimalExtruderCount());

		commandName = getEngineFilename();
		commandPara = "";
		
		commandPara += "-vv ";
		cSettings* pSettings = this->_engineSettings(extruderCount);
		std::vector<cPair*>::iterator ite;
		for(ite = pSettings->pairs.begin(); ite != pSettings->pairs.end(); ite++) {
			char buf[128];
			cPair* p = *ite;
			sprintf_s(buf, 128, "%s=%s", p->key, p->tostr());
			commandPara += " -s "; 		commandPara += buf;
		}
		commandPara += " -o "; 		commandPara += _exportFilename;
		commandPara += " -b ";		commandPara += _binaryStorageFilename;

		this->_objCount = 0;
		stlfile f;
		if(f.open(this->_binaryStorageFilename, "wb") != INVALID_HANDLE_VALUE) {
//			hash = hashlib.sha512()
			intvector* order = scene.printOrder();
			if (order != NULL) {
				fpxy pos = profile.getMachineCenterCoords();
				pos.x *= 1000;
				pos.y *= 1000;
				int first = 1;
				fpxy objMin;
				fpxy objMax;

				for(int oi = 0; oi < scene._objectList->length; oi++) {
					printableObject* obj = scene._objectList->data[oi];
					if( scene.checkPlatform(obj)) {
						fpxyz minp(obj->getMinimum());
						fpxyz maxp(obj->getMaximum());
						fpxy oMin( minp.data[0] + obj->getPosition().x, minp.data[1] + obj->getPosition().y);
						fpxy oMax( maxp.data[0] + obj->getPosition().x, maxp.data[1] + obj->getPosition().y);

						if(first) {
							first = 0;
							objMin = oMin;
							objMax = oMax;
						} else {
							objMin.x = min(oMin.x, objMin.x);
							objMin.y = min(oMin.y, objMin.y);
							objMax.x = max(oMax.x, objMax.x);
							objMax.y = max(oMax.y, objMax.y);
						}
					}
				}
				pos.x += (objMin.x + objMax.x) / 2.0 * 1000;
				pos.y += (objMin.y + objMax.y) / 2.0 * 1000;

				char buf[128];
				sprintf_s(buf, 128, "posx=%d", int(pos.x));
				commandPara += " -s "; commandPara += buf;
				sprintf_s(buf, 128, "posy=%d", (int)pos.y);
				commandPara += " -s "; commandPara += buf;

				int vertexTotal[20];
				memset(vertexTotal, 0, sizeof(vertexTotal));
				int meshMax = 1;
//				std::vector<printableObject*>::iterator ite;
				for(int idx = 0; idx < scene._objectList->length; idx++) {
					printableObject* obj = scene._objectList->data[idx];
					if( scene.checkPlatform(obj)) {
						meshMax = max(meshMax, obj->_meshList->len);
						for(int n = 0; n < obj->_meshList->len; n++)
							vertexTotal[n] += obj->_meshList->data[n]->vertexCount;
					}
				}

				for(int n = 0; n < meshMax; n++) {
					f.write(&vertexTotal[n], 4);
					for(int oi = 0; oi < scene._objectList->length; oi++) {
						printableObject* obj = scene._objectList->data[oi];
						if( scene.checkPlatform(obj)) {
							if( n < obj->_meshList->len) {
								vertexmat* vertexes = *obj->_meshList->data[n]->vertexes * *obj->_matrix;
								*vertexes -= obj->_drawOffset;
								float v[3];
								v[0] = obj->getPosition().x;
								v[1] = obj->getPosition().y;
								v[2] = 0.0;
								*vertexes += v;
								f.write(vertexes);
								delete vertexes;
								//hash.update(obj._meshList[n].vertexes.tostring())
							}
						}
					}
				}
				commandPara += " # "; commandPara += " " + meshMax;
				this->_objCount = 1;
			} else {
				for(int ni = 0; ni < order->len; ni++) {
					int n = order->data[ni];
					printableObject* pobj = scene._objectList->data[n];
					for(int itm = 0; itm < pobj->_meshList->len; itm++) {
						mesh* m = pobj->_meshList->data[itm];
						f.write(&m->vertexCount, 4);
						f.write(m->vertexes);
						// hash.update(s)
					}
					fpxy pos = pobj->getPosition();
					pos.x += profile.getMachineCenterCoords().x;
					pos.y += profile.getMachineCenterCoords().y;
					pos.x *= 1000;
					pos.y *= 1000;

					// map : call function 'str' to each element of '_matrix.getA().flatten()'
					// join : make ',' delimeted connected string ex: 1,2,3,5...
					//commandList += ['-m', ','.join(map(str, obj._matrix.getA().flatten()))]
					commandPara += " -m "; commandPara += obj->_matrix->flatstr(",");
					char buf[128];
					sprintf_s(buf, 128, "posx=%d", pos.x);
					commandPara += " -s "; commandPara += buf;
					sprintf_s(buf, 128, "posy=%d", pos.y);
					commandPara += " -s "; commandPara += buf;
					commandPara += " # ";  commandPara += " " + obj->_meshList->len;
					this->_objCount += 1;
				}
			}
			// this->_modelHash = hash.hexdigest()
			f.close();
		}
		
		if( this->_objCount > 0) {
		    unsigned threadID;
			unsigned int WINAPI slicerThread(LPVOID lpVoid);
			if(oldThread != NULL) {
				if(_process)
					TerminateProcess(_process, 0);
				_process = NULL;
				WaitForSingleObject( oldThread, INFINITE );
				CloseHandle( oldThread );
				oldThread = NULL;
			}
			oldThread = (HANDLE)_beginthreadex(NULL, 0, slicerThread, NULL, 0, &threadID);
		}
	}
}
#include <windows.h> 
#include <tchar.h>
#include <stdio.h> 
#include <strsafe.h>

#define BUFSIZE 4096 
 
HANDLE g_hChildStd_IN_Rd = NULL;
HANDLE g_hChildStd_IN_Wr = NULL;
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;

void CreateChildProcess(void); 
void WriteToPipe(void); 
void ReadFromPipe(void); 
void ErrorExit(PTSTR); 
 
int _tmain(int argc, TCHAR *argv[]) 
{ 
   printf("\n->End of parent execution.\n");

// The remaining open handles are cleaned up when this process terminates. 
// To avoid resource leaks in a larger application, close handles explicitly. 

   return 0; 
} 
 
void CreateChildProcess(TCHAR* szCmdline)
// Create a child process that uses the previously created pipes for STDIN and STDOUT.
{ 
   PROCESS_INFORMATION piProcInfo; 
   STARTUPINFO siStartInfo;
   BOOL bSuccess = FALSE; 
 
// Set up members of the PROCESS_INFORMATION structure. 
 
   ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );
 
// Set up members of the STARTUPINFO structure. 
// This structure specifies the STDIN and STDOUT handles for redirection.
 
   ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
   siStartInfo.cb = sizeof(STARTUPINFO); 
   siStartInfo.hStdError = g_hChildStd_OUT_Wr;
   siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
   siStartInfo.hStdInput = g_hChildStd_IN_Rd;
   siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
 
// Create the child process. 
    
   bSuccess = CreateProcess(NULL, 
      szCmdline,     // command line 
      NULL,          // process security attributes 
      NULL,          // primary thread security attributes 
      TRUE,          // handles are inherited 
      0,             // creation flags 
      NULL,          // use parent's environment 
      NULL,          // use parent's current directory 
      &siStartInfo,  // STARTUPINFO pointer 
      &piProcInfo);  // receives PROCESS_INFORMATION 
   
   // If an error occurs, exit the application. 
   if ( ! bSuccess ) 
      ErrorExit(TEXT("CreateProcess"));
   else 
   {
      // Close handles to the child process and its primary thread.
      // Some applications might keep these handles to monitor the status
      // of the child process, for example. 
		_process = piProcInfo.hProcess;//      CloseHandle(piProcInfo.hProcess);
      CloseHandle(piProcInfo.hThread);
   }
}
 
void ErrorExit(PTSTR lpszFunction) 

// Format a readable error message, display a message box, 
// and exit from the application.
{ 
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)lpszFunction)+40)*sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(1);
}


unsigned int WINAPI slicerThread(LPVOID lpVoid)
{
	SECURITY_ATTRIBUTES saAttr; 
// Set the bInheritHandle flag so pipe handles are inherited. 
   saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
   saAttr.bInheritHandle = TRUE; 
   saAttr.lpSecurityDescriptor = NULL; 

// Create a pipe for the child process's STDOUT. 
   if ( ! CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0) ) 
		ErrorExit(TEXT("StdoutRd CreatePipe")); 

// Ensure the read handle to the pipe for STDOUT is not inherited.
	if ( ! SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0) )
		ErrorExit(TEXT("Stdout SetHandleInformation")); 

// Create a pipe for the child process's STDIN. 
   if (! CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0)) 
		ErrorExit(TEXT("Stdin CreatePipe")); 

// Ensure the write handle to the pipe for STDIN is not inherited. 
	if ( ! SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0) )
		ErrorExit(TEXT("Stdin SetHandleInformation")); 
 
// Create the child process. 
	int mlen = lstrlen((LPCTSTR)commandPara)+1;
	TCHAR *paraBuf = (TCHAR*)malloc(mlen*sizeof(TCHAR));
	StringCchCopy(paraBuf, mlen, (LPCTSTR)commandPara);
	CreateChildProcess(paraBuf);
	free(paraBuf);

	char line[1024];
	int  lcnt = 0;
//	objectNr = 0

	for (;;) 
	{ 
		CHAR chBuf[BUFSIZE]; 
		BOOL bSuccess = FALSE;
		DWORD dwRead = 0;
		bSuccess = ReadFile( g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
		if( ! bSuccess || dwRead == 0 )
			break; 
/*
	while len(line):
		line = line.strip()
		if line.startswith('Progress:'):
			line = line.split(':')
			if line[1] == 'process':
				objectNr += 1
			elif line[1] in this->_progressSteps:
				progressValue = float(line[2]) / float(line[3])
				progressValue /= len(this->_progressSteps)
				progressValue += 1.0 / len(this->_progressSteps) * this->_progressSteps.index(line[1])

				progressValue /= this->_objCount
				progressValue += 1.0 / this->_objCount * objectNr
				try:
					this->_callback(progressValue, False)
				except:
					pass
		elif line.startswith('Print time:'):
			this->_printTimeSeconds = int(line.split(':')[1].strip())
		elif line.startswith('Filament:'):
			this->_filamentMM[0] = int(line.split(':')[1].strip())
			if profile.getMachineSetting('gcode_flavor') == 'UltiGCode':
				radius = profile.getProfileSettingFloat('filament_diameter') / 2.0
				this->_filamentMM[0] /= (math.pi * radius * radius)
		elif line.startswith('Filament2:'):
			this->_filamentMM[1] = int(line.split(':')[1].strip())
			if profile.getMachineSetting('gcode_flavor') == 'UltiGCode':
				radius = profile.getProfileSettingFloat('filament_diameter') / 2.0
				this->_filamentMM[1] /= (math.pi * radius * radius)
		else:
			this->_sliceLog.append(line.strip())
		line = this->_process.stdout.readline()
	for line in this->_process.stderr:
		this->_sliceLog.append(line.strip())
*/
	} 
/*
	returnCode = this->_process.wait()
	try:
		if returnCode == 0:
			pluginError = profile.runPostProcessingPlugins(this->_exportFilename)
			if pluginError is not None:
				print pluginError
				this->_sliceLog.append(pluginError)
			this->_callback(1.0, True)
		else:
			for line in this->_sliceLog:
				print line
			this->_callback(-1.0, False)
	except:
		pass
*/
	_process = NULL;

	return 0;
}

void cSettings::add(char* key, int iv)
{
	cPair* p = find(key);
	if(p != NULL)
		p->set(iv);
	else
		this->pairs.push_back(new cPair(key, iv));
}

void cSettings::add(char* key, char* str)
{
	cPair* p = find(key);
	if(p != NULL)
		p->set(str);
	else
		this->pairs.push_back(new cPair(key, str));
}

void cSettings::set(char* key, int iv)
{
	cPair* p = find(key);
	if(p != NULL)
		p->iv = iv;
	else
		this->pairs.push_back(new cPair(key, iv));
}

void cSettings::set(char* key, char* str)
{
	cPair* p = find(key);
	if(p != NULL)
		strcpy_s(p->sv, 128, str);
	else
		this->pairs.push_back(new cPair(key, str));
}

cPair* cSettings::find(char* key)
{
	std::vector<cPair*>::iterator ite;
	for(ite = pairs.begin(); ite != pairs.end(); ite++) {
		cPair* p = *ite;
		if(strcmp(key, p->key) == 0)
			return p;
	}
	return NULL;
}

int cSettings::getInt(char* key)
{
	cPair* p = find(key);

	if(p == NULL)
		return 0;

	switch(p->vt) {
	case PTYPE_INT :
		return p->iv;
	case PTYPE_FLOAT :
		return (int)p->fv;
	case PTYPE_STR :
		return atol(p->sv);
	}

	return 0;
}

float cSettings::getFloat(char* key)
{
	cPair* p = find(key);

	if(p == NULL)
		return 0;

	switch(p->vt) {
	case PTYPE_INT :
		return (float)p->iv;
	case PTYPE_FLOAT :
		return (int)p->fv;
	case PTYPE_STR :
		return atof(p->sv);
	}

	return 0;
}

cSettings* Slicer::_engineSettings(int extruderCount)
{
	cSettings *settings = new cSettings();
	settings->add("layerThickness", (int)(profile.getProfileSettingFloat("layer_height") * 1000));
	if(profile.getProfileSettingFloat("bottom_thickness") > 0.0)
		settings->add("initialLayerThickness", (int)(profile.getProfileSettingFloat("bottom_thickness") * 1000));
	else
		settings->add("initialLayerThickness", int(profile.getProfileSettingFloat("layer_height") * 1000));
	settings->add("filamentDiameter", int(profile.getProfileSettingFloat("filament_diameter") * 1000));
	settings->add("filamentFlow", int(profile.getProfileSettingFloat("filament_flow")));
	settings->add("extrusionWidth", int(profile.calculateEdgeWidth() * 1000));
	settings->add("insetCount", int(profile.calculateLineCount()));
	settings->add("downSkinCount", strcmp(profile.getProfileSetting("solid_bottom") ,"True") == 0 ?
		int(profile.calculateSolidLayerCount()) : 0);
	settings->add("upSkinCount", strcmp(profile.getProfileSetting("solid_top"), "True") == 0 ?
		int(profile.calculateSolidLayerCount())  : 0);
	settings->add("infillOverlap", int(profile.getProfileSettingFloat("fill_overlap")));
	settings->add("initialSpeedupLayers", int(4));
	settings->add("initialLayerSpeed", int(profile.getProfileSettingFloat("bottom_layer_speed")));
	settings->add("printSpeed", int(profile.getProfileSettingFloat("print_speed")));
	settings->add("infillSpeed", int(profile.getProfileSettingFloat("infill_speed")) > 0 ?
		int(profile.getProfileSettingFloat("infill_speed")) : int(profile.getProfileSettingFloat("print_speed")));
	settings->add("moveSpeed", int(profile.getProfileSettingFloat("travel_speed")));
	settings->add("fanSpeedMin", strcmp( profile.getProfileSetting("fan_enabled") , "True") == 0 ?
		int(profile.getProfileSettingFloat("fan_speed"))  : 0);
	settings->add("fanSpeedMax", strcmp( profile.getProfileSetting("fan_enabled") , "True" ) == 0 ?
		int(profile.getProfileSettingFloat("fan_speed_max")) : 0);
	settings->add("supportAngle", strcmp( profile.getProfileSetting("support") , "None" ) == 0 ?
		int(-1) : int(60));
	settings->add("supportEverywhere", strcmp( profile.getProfileSetting("support") , "Everywhere" ) == 0 ?
		int(1) : int(0));
	settings->add("supportLineDistance",  profile.getProfileSettingFloat("support_fill_rate") > 0 ?
		int(100 * profile.calculateEdgeWidth() * 1000 / profile.getProfileSettingFloat("support_fill_rate")) : -1);
	settings->add("supportXYDistance", int(1000 * profile.getProfileSettingFloat("support_xy_distance")));
	settings->add("supportZDistance", int(1000 * profile.getProfileSettingFloat("support_z_distance")));

	if(strcmp(profile.getProfileSetting("support_dual_extrusion"), "First extruder") == 0)
		settings->add("supportExtruder", 0);
	else {
		if(strcmp(profile.getProfileSetting("support_dual_extrusion"), "Second extruder") == 0 && profile.minimalExtruderCount() > 1)
			settings->add("supportExtruder", 1);
		else
			settings->add("supportExtruder", -1);
	}
	settings->add("retractionAmount", strcmp(profile.getProfileSetting("retraction_enable"), "True" ) == 0 ?
		int(profile.getProfileSettingFloat("retraction_amount") * 1000) : 0);
	settings->add("retractionSpeed", int(profile.getProfileSettingFloat("retraction_speed")));
	settings->add("retractionMinimalDistance", int(profile.getProfileSettingFloat("retraction_min_travel") * 1000));
	settings->add("retractionAmountExtruderSwitch", int(profile.getProfileSettingFloat("retraction_dual_amount") * 1000));
	settings->add("minimalExtrusionBeforeRetraction", int(profile.getProfileSettingFloat("retraction_minimal_extrusion") * 1000));
	settings->add("enableCombing", strcmp( profile.getProfileSetting("retraction_combing") , "True" ) == 0 ? 1 : 0);
	settings->add("multiVolumeOverlap", int(profile.getProfileSettingFloat("overlap_dual") * 1000));
	settings->add("objectSink", int(profile.getProfileSettingFloat("object_sink") * 1000));
	settings->add("minimalLayerTime", int(profile.getProfileSettingFloat("cool_min_layer_time")));
	settings->add("minimalFeedrate", int(profile.getProfileSettingFloat("cool_min_feedrate")));
	settings->add("coolHeadLift", strcmp(profile.getProfileSetting("cool_head_lift") , "True" ) == 0 ? 1 : 0);
	settings->add("startCode", getAlterationFileContents("start.gcode", extruderCount));
	settings->add("endCode", getAlterationFileContents("end.gcode", extruderCount));

	settings->add("extruderOffset[1].X", int(profile.getMachineSettingFloat("extruder_offset_x1") * 1000));
	settings->add("extruderOffset[1].Y", int(profile.getMachineSettingFloat("extruder_offset_y1") * 1000));
	settings->add("extruderOffset[2].X", int(profile.getMachineSettingFloat("extruder_offset_x2") * 1000));
	settings->add("extruderOffset[2].Y", int(profile.getMachineSettingFloat("extruder_offset_y2") * 1000));
	settings->add("extruderOffset[3].X", int(profile.getMachineSettingFloat("extruder_offset_x3") * 1000));
	settings->add("extruderOffset[3].Y", int(profile.getMachineSettingFloat("extruder_offset_y3") * 1000));
	settings->add("fixHorrible", 0);

	int fanFullHeight = int(profile.getProfileSettingFloat("fan_full_height") * 1000);
	settings->add("fanFullOnLayerNr", (fanFullHeight - settings->getInt("initialLayerThickness") - 1) / settings->getInt("layerThickness") + 1);
	if( settings->getInt("fanFullOnLayerNr") < 0)
		settings->set("fanFullOnLayerNr", 0);

	if( profile.getProfileSettingFloat("fill_density") == 0)
		settings->add("sparseInfillLineDistance", -1);
	else if(profile.getProfileSettingFloat("fill_density") == 100) {
		settings->set("sparseInfillLineDistance", settings->getInt("extrusionWidth"));
		//	#Set the up/down skins height to 10000 if we want a 100% filled object.
		//	# This gives better results then normal 100% infill as the sparse and up/down skin have some overlap.
		settings->set("downSkinCount", 10000);
		settings->set("upSkinCount", 10000);
	} else {
		settings->set("sparseInfillLineDistance", int(100 * profile.calculateEdgeWidth() * 1000 / profile.getProfileSettingFloat("fill_density")));
	}

	if(strcmp( profile.getProfileSetting("platform_adhesion") , "Brim") == 0) {
		settings->set("skirtDistance", 0);
		settings->set("skirtLineCount", int(profile.getProfileSettingFloat("brim_line_count")));
	} else if(strcmp(profile.getProfileSetting("platform_adhesion") , "Raft") == 0) {
		settings->set("skirtDistance", 0);
		settings->set("skirtLineCount", 0);
		settings->set("raftMargin", int(profile.getProfileSettingFloat("raft_margin") * 1000));
		settings->set("raftLineSpacing", int(profile.getProfileSettingFloat("raft_line_spacing") * 1000));
		settings->set("raftBaseThickness", int(profile.getProfileSettingFloat("raft_base_thickness") * 1000));
		settings->set("raftBaseLinewidth", int(profile.getProfileSettingFloat("raft_base_linewidth") * 1000));
		settings->set("raftInterfaceThickness", int(profile.getProfileSettingFloat("raft_interface_thickness") * 1000));
		settings->set("raftInterfaceLinewidth", int(profile.getProfileSettingFloat("raft_interface_linewidth") * 1000));
	} else {
		settings->set("skirtDistance", int(profile.getProfileSettingFloat("skirt_gap") * 1000));
		settings->set("skirtLineCount", int(profile.getProfileSettingFloat("skirt_line_count")));
		settings->set("skirtMinLength", int(profile.getProfileSettingFloat("skirt_minimal_length") * 1000));
	}

	int fh = settings->getInt("fixHorrible");
	if(strcmp( profile.getProfileSetting("fix_horrible_union_all_type_a") , "True") == 0)
		fh |= 0x01;
	if(strcmp(profile.getProfileSetting("fix_horrible_union_all_type_b"), "True") == 0)
		fh |= 0x02;
	if(strcmp(profile.getProfileSetting("fix_horrible_use_open_bits"), "True") == 0)
		fh |= 0x10;
	if(strcmp(profile.getProfileSetting("fix_horrible_extensive_stitching"), "True") == 0)
		fh |= 0x04;
	settings->set("fixHorrible", fh); 

	if(settings->getInt("layerThickness") <= 0)
		settings->set("layerThickness", 1000);
	if(strcmp(profile.getMachineSetting("gcode_flavor"), "UltiGCode") == 0)
		settings->set("gcodeFlavor", 1);
	if(strcmp(profile.getProfileSetting("spiralize"), "True") == 0)
		settings->set("spiralizeMode", 1);
	if(strcmp(profile.getProfileSetting("wipe_tower"), "True") == 0)
		settings->set("wipeTowerSize", int(sqrt(profile.getProfileSettingFloat("wipe_tower_volume") * 1000 * 1000 * 1000 / settings->getFloat("layerThickness"))));
	if (strcmp(profile.getProfileSetting("ooze_shield"), "True") == 0)
		settings->set("enableOozeShield", 1);
	return settings;
}

/*
int Slicer::_runSliceProcess(cmdList)
{
		kwargs = {}
		if subprocess.mswindows:
			su = subprocess.STARTUPINFO()
			su.dwFlags |= subprocess.STARTF_USESHOWWINDOW
			su.wShowWindow = subprocess.SW_HIDE
			kwargs['startupinfo'] = su
			kwargs['creationflags'] = 0x00004000 #BELOW_NORMAL_PRIORITY_CLASS
		return subprocess.Popen(cmdList, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, **kwargs)
}
*/

/*
void Slicer::submitSliceInfoOnline(self)
{
	if profile.getPreference('submit_slice_information') != 'True':
			return
		if version.isDevVersion():
			return
		data = {
			'processor': platform.processor(),
			'machine': platform.machine(),
			'platform': platform.platform(),
			'profile': profile.getProfileString(),
			'preferences': profile.getPreferencesString(),
			'modelhash': this->_modelHash,
			'version': version.getVersion(),
		}
		try:
			f = urllib2.urlopen("http://www.youmagine.com/curastats/", data = urllib.urlencode(data), timeout = 1)
			f.read()
			f.close()
		except:
			pass
}
*/
