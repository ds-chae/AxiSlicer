#include "stdafx.h"
#include "mesh.h"
#include "opengl.h"

/*

__copyright__ = "Copyright (C) 2013 David Braam - Released under terms of the AGPLv3 License"

import math
import numpy
import wx
import time

from Cura.util import meshLoader
from Cura.util import util3d
from Cura.util import profile
from Cura.util.resources import getPathForMesh, getPathForImage

import OpenGL

OpenGL.ERROR_CHECKING = False
from OpenGL.GLUT import *
from OpenGL.GLU import *
from OpenGL.GL import *
from OpenGL.GL import shaders
glutInit()

platformMesh = None

class GLReferenceCounter(object):
	def __init__(self):
		this->_refCounter = 1

	def incRef(self):
		this->_refCounter += 1

	def decRef(self):
		this->_refCounter -= 1
		return this->_refCounter <= 0
*/
int hasShaderSupport()
{
	if(False /*bool(glCreateShader)*/)
		return True;
	return False;
}

GLuint compileShader(GLchar* program, GLenum shaderType)
{
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, (const GLchar**)&program, NULL);
	glCompileShader(shader);
	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	return shader;

}

GLShader::GLShader(char*vertexProgram, char* fragmentProgram ) : GLFakeShader()
{
	this->_vertexString = vertexProgram;
	this->_fragmentString = fragmentProgram;

	GLuint vertexShader = compileShader(vertexProgram, GL_VERTEX_SHADER);
	GLuint fragmentShader = compileShader(fragmentProgram, GL_FRAGMENT_SHADER);

//		#shader.compileProgram tries to return the shader program as a overloaded int. But the return value of a shader does not always fit in a int (needs to be a long). So we do raw OpenGL calls.
//		# this->_program = shaders.compileProgram(this->_vertexProgram, this->_fragmentProgram)
	this->_program = glCreateProgram();
	glAttachShader(this->_program, vertexShader);
	glAttachShader(this->_program, fragmentShader);
	glLinkProgram(this->_program);
	//# Validation has to occur *after* linking
	glValidateProgram(this->_program);
	GLint param;
	glGetProgramiv(this->_program, GL_VALIDATE_STATUS, &param);
	if (param == GL_FALSE){
		//raise RuntimeError("Validation failure: %s"%(glGetProgramInfoLog(this->_program)));
	}
	glGetProgramiv(this->_program, GL_LINK_STATUS, &param);
	if (param == GL_FALSE){
		//raise RuntimeError("Link failure: %s" % (glGetProgramInfoLog(this->_program)));
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void GLShader::bind()
{
	if (this->_program != NULL)
		glUseProgram(this->_program);
}

void GLShader::unbind()
{
	glUseProgram(0);
}

void GLShader::release()
{
	if (this->_program != NULL){
		glDeleteProgram(this->_program);
		this->_program = NULL;
	}
}

/*
void GLShader::setUniform(char* name, value)
{
	if (this->_program != NULL) {
		if type(value) is float:
			glUniform1f(glGetUniformLocation(this->_program, name), value)
		elif type(value) is numpy.matrix:
			glUniformMatrix3fv(glGetUniformLocation(this->_program, name), 1, False, value.getA().astype(numpy.float32))
		else:
			print 'Unknown type for setUniform: %s' % (str(type(value)));
	}
}
*/
void GLShader::setUniform(char* name, float value)
{
	if (this->_program != NULL) {
		glUniform1f(glGetUniformLocation(this->_program, name), value);
	}
}

void GLShader::setUniform(char* name, float* value)
{
	if (this->_program != NULL) {
		glUniformMatrix3fv(glGetUniformLocation(this->_program, name), 1, False, value);
	}
}

int GLShader::isValid()
{
	return this->_program != NULL;
}

char* GLShader::getVertexShader()
{
	return this->_vertexString;
}

char* GLShader::getFragmentShader()
{
	return this->_fragmentString;
}

void GLShader::__del__()
{
	if (this->_program != NULL && False/*bool(glDeleteProgram)*/){
		// print "Shader was not properly released!";
	}
}

//#A Class that acts as an OpenGL shader, but in reality is not none.

void GLFakeShader::bind()
{
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	float f1[4] = {1,1,1,1};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, f1);
	float f0[4] = {0,0,0,0};
	glLightfv(GL_LIGHT0, GL_AMBIENT, f0);
	glLightfv(GL_LIGHT0, GL_SPECULAR,f0);
}

void GLFakeShader::unbind()
{
		glDisable(GL_LIGHTING);
}

void GLFakeShader::release()
{
}

void GLFakeShader::setUniform(char* name, float* value)
{
}

void GLFakeShader::setUniform(char* name, float value)
{
}

int  GLFakeShader::isValid()
{
	return True;
}

char* GLFakeShader::getVertexShader()
{
	return "";
}

char* GLFakeShader::getFragmentShader()
{
	return "";
}

GLVBO::GLVBO(vertexmat* vertexArray, vertexmat* normalArray, int _deletevertex, char* _file, int _line ) : GLReferenceCounter()
{
	_memnew(this, "GLVBO", _file, _line);
#if 1
	this->deletevertex = _deletevertex;
	this->originalvertex = vertexArray;
	this->originalnormal = normalArray;
	// when you import GL, bool(glGenBuffers) is always False...
	if (! False/*bool(glGenBuffers)*/) {
	// when you import GL, bool(glGenBuffers) is always False...
		this->_vertexArray = (float*)vertexArray->data;
		if(normalArray != NULL)
			this->_normalArray = (float*)normalArray->data;
		else
			this->_normalArray = NULL;
		this->_size = vertexArray->size;
		this->_buffer = NULL;
		this->_hasNormals = this->_normalArray != NULL;
	} else {
		GLuint tex;
		glGenBuffers(1, &tex);
		this->_buffer = tex;
		this->_size = vertexArray->size;
		this->_hasNormals = normalArray != NULL;
		glBindBuffer(GL_ARRAY_BUFFER, this->_buffer);
		if (this->_hasNormals) {
			//glBufferData(GL_ARRAY_BUFFER, numpy.concatenate((vertexArray, normalArray), 1), GL_STATIC_DRAW);
		} else {
			//glBufferData(GL_ARRAY_BUFFER, vertexArray, GL_STATIC_DRAW);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
#endif
}

void GLVBO::render(int render_type)
{
	if(this->_size <= 0)
		return;

	glEnableClientState(GL_VERTEX_ARRAY);
	if (_buffer == NULL) {
		glVertexPointer(3, GL_FLOAT, 0, this->_vertexArray);
		if (this->_hasNormals) {
			glEnableClientState(GL_NORMAL_ARRAY);
			glNormalPointer(GL_FLOAT, 0, this->_normalArray);
		}
	} else {
		glBindBuffer(GL_ARRAY_BUFFER, this->_buffer);
		if (this->_hasNormals) {
			glEnableClientState(GL_NORMAL_ARRAY);
			glVertexPointer(3, GL_FLOAT, 2*3*4, NULL);
			glNormalPointer(GL_FLOAT, 2*3*4, (GLvoid*)(3 * 4)); // offset is 3*4
		} else {
			glVertexPointer(3, GL_FLOAT, 3*4, NULL);
		}
	}

	int batchSize = 996;   // #Warning, batchSize needs to be dividable by 4, 3 and 2
	int extraStartPos = int(this->_size / batchSize) * batchSize;
	int extraCount = this->_size - extraStartPos;

	for(int i = 0; i < int(this->_size / batchSize); i++) {
		glDrawArrays(render_type, i * batchSize, batchSize);
	}
	glDrawArrays(render_type, extraStartPos, extraCount);
	if (this->_buffer != NULL){
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	if (this->_hasNormals)
		glDisableClientState(GL_NORMAL_ARRAY);
}

void GLVBO::release()
{
	if (this->_buffer != NULL) {
		glBindBuffer(GL_ARRAY_BUFFER, this->_buffer);
		glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &this->_buffer);
		this->_buffer = NULL;
	}
	this->_vertexArray = NULL;
	this->_normalArray = NULL;
}

void GLVBO::__del__()
{
	if (this->_buffer != NULL && False/*bool(glDeleteBuffers)*/){
		// print "VBO was not properly released!";
	}
}

void glDrawStringCenter(TCHAR* s)
{
	glRasterPos2f(0, 0);
	glBitmap(0,0,0,0, -glGetStringSize(s).x/2, 0, NULL);
	while(*s) {
		int c = *s;
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
		s++;
	}
}

fpxy glGetStringSize(TCHAR* s)
{
	int width = 0;
	while(*s) {
		int c = *s;
		width += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, c);
		s++;
	}
	int height = 18;
	return fpxy(width, height);
}

/*
def glDrawStringLeft(s):
	glRasterPos2f(0, 0)
	n = 1
	for c in s:
		if c == '\n':
			glPushMatrix()
			glTranslate(0, 18 * n, 0)
			n += 1
			glRasterPos2f(0, 0)
			glPopMatrix()
		else:
			glutBitmapCharacter(OpenGL.GLUT.GLUT_BITMAP_HELVETICA_18, ord(c))
*/
void glDrawStringRight(TCHAR* s)
{
	glRasterPos2f(0, 0);
	glBitmap(0,0,0,0, -glGetStringSize(s).x, 0, NULL);
	while(*s) {
		int c = *s;
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
		s++;
	}
}

void glDrawQuad(float x, float y, float w, float h)
{
	glPushMatrix();
	glTranslatef(x, y, 0);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glVertex2f(w, 0);
	glVertex2f(0, 0);
	glVertex2f(0, h);
	glVertex2f(w, h);
	glEnd();
	glPopMatrix();
}

void glDrawTexturedQuad(int x, int y, int w, int h, int texID, int mirror)
{
	float tx = float(texID % 4) / 4;
	float ty = float(int(texID / 4)) / 8;
	float tsx = 0.25;
	float tsy = 0.125;
	if (mirror & 1) {
		tx += tsx;
		tsx = -tsx;
	}
	if (mirror & 2) {
		ty += tsy;
		tsy = -tsy;
	}
	glPushMatrix();
	glTranslatef(x, y, 0);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glTexCoord2f(tx+tsx, ty);
	glVertex2f(w, 0);
	glTexCoord2f(tx, ty);
	glVertex2f(0, 0);
	glTexCoord2f(tx, ty+tsy);
	glVertex2f(0, h);
	glTexCoord2f(tx+tsx, ty+tsy);
	glVertex2f(w, h);
	glEnd();
	glPopMatrix();
}

/*
def glDrawStretchedQuad(x, y, w, h, cornerSize, texID):
	tx0 = float(texID % 4) / 4
	ty0 = float(int(texID / 4)) / 8
	tx1 = tx0 + 0.25 / 2.0
	ty1 = ty0 + 0.125 / 2.0
	tx2 = tx0 + 0.25
	ty2 = ty0 + 0.125

	glPushMatrix()
	glTranslatef(x, y, 0)
	glEnable(GL_TEXTURE_2D)
	glBegin(GL_QUADS)
	#TopLeft
	glTexCoord2f(tx1, ty0)
	glVertex2f( cornerSize, 0)
	glTexCoord2f(tx0, ty0)
	glVertex2f( 0, 0)
	glTexCoord2f(tx0, ty1)
	glVertex2f( 0, cornerSize)
	glTexCoord2f(tx1, ty1)
	glVertex2f( cornerSize, cornerSize)
	#TopRight
	glTexCoord2f(tx2, ty0)
	glVertex2f( w, 0)
	glTexCoord2f(tx1, ty0)
	glVertex2f( w - cornerSize, 0)
	glTexCoord2f(tx1, ty1)
	glVertex2f( w - cornerSize, cornerSize)
	glTexCoord2f(tx2, ty1)
	glVertex2f( w, cornerSize)
	#BottomLeft
	glTexCoord2f(tx1, ty1)
	glVertex2f( cornerSize, h - cornerSize)
	glTexCoord2f(tx0, ty1)
	glVertex2f( 0, h - cornerSize)
	glTexCoord2f(tx0, ty2)
	glVertex2f( 0, h)
	glTexCoord2f(tx1, ty2)
	glVertex2f( cornerSize, h)
	#BottomRight
	glTexCoord2f(tx2, ty1)
	glVertex2f( w, h - cornerSize)
	glTexCoord2f(tx1, ty1)
	glVertex2f( w - cornerSize, h - cornerSize)
	glTexCoord2f(tx1, ty2)
	glVertex2f( w - cornerSize, h)
	glTexCoord2f(tx2, ty2)
	glVertex2f( w, h)

	#Center
	glTexCoord2f(tx1, ty1)
	glVertex2f( w-cornerSize, cornerSize)
	glTexCoord2f(tx1, ty1)
	glVertex2f( cornerSize, cornerSize)
	glTexCoord2f(tx1, ty1)
	glVertex2f( cornerSize, h-cornerSize)
	glTexCoord2f(tx1, ty1)
	glVertex2f( w-cornerSize, h-cornerSize)

	#Right
	glTexCoord2f(tx2, ty1)
	glVertex2f( w, cornerSize)
	glTexCoord2f(tx1, ty1)
	glVertex2f( w-cornerSize, cornerSize)
	glTexCoord2f(tx1, ty1)
	glVertex2f( w-cornerSize, h-cornerSize)
	glTexCoord2f(tx2, ty1)
	glVertex2f( w, h-cornerSize)

	#Left
	glTexCoord2f(tx1, ty1)
	glVertex2f( cornerSize, cornerSize)
	glTexCoord2f(tx0, ty1)
	glVertex2f( 0, cornerSize)
	glTexCoord2f(tx0, ty1)
	glVertex2f( 0, h-cornerSize)
	glTexCoord2f(tx1, ty1)
	glVertex2f( cornerSize, h-cornerSize)

	#Top
	glTexCoord2f(tx1, ty0)
	glVertex2f( w-cornerSize, 0)
	glTexCoord2f(tx1, ty0)
	glVertex2f( cornerSize, 0)
	glTexCoord2f(tx1, ty1)
	glVertex2f( cornerSize, cornerSize)
	glTexCoord2f(tx1, ty1)
	glVertex2f( w-cornerSize, cornerSize)

	#Bottom
	glTexCoord2f(tx1, ty1)
	glVertex2f( w-cornerSize, h-cornerSize)
	glTexCoord2f(tx1, ty1)
	glVertex2f( cornerSize, h-cornerSize)
	glTexCoord2f(tx1, ty2)
	glVertex2f( cornerSize, h)
	glTexCoord2f(tx1, ty2)
	glVertex2f( w-cornerSize, h)

	glEnd()
	glDisable(GL_TEXTURE_2D)
	glPopMatrix()
*/

/*
----------------------------------
Matrix A    Matrix D    Result C
1 2 3        1 2        22  28
4 5 6        3 4        49  64
7 8 9        5 6        76 100
----------------------------------
*/

/*
#define A 3
#define B 3
#define K 2

int main(void) {
   int b, e, f, g=0;
   int a[B][A];
   int d[A][K];
   int c[B][K];

   // 행렬 A 에 대한 값을 입력 받음 
    for (b=0; b<B; b++) {
       for (e=0; e<A; e++) {
          printf("a[%d][%d]", b, e);
         scanf("%d", &a[b][e]);
      }
   }

   // 행렬 D 에 대한 값을 입력 받음 
   for (b=0; b<A; b++) {
      for (e=0; e<K; e++) {
         printf("d[%d][%d]", b, e);
         scanf("%d", &d[b][e]);
      }
   }

   // 두 행렬 A, D 를 곱하여 결과를 행렬 C 에 저장 
   for (e=0; e<B; e++) {
      for (b=0; b<K; b++) {
         for (f=0; f<A; f++) {
            g += a[e][f] * d[f][b];
         }
         c[e][b] = g;
         g = 0;
         printf("%d ", c[e][b]);
      }
      printf("\n");
   }
   
   return (0);
} 
*/

void multiplyMatrix(double*a, double* d, double*c, int A, int B, int K)
{
	//   int a[B][A]; 3 3
	//   int d[A][K]; 3 2
	//   int c[B][K]; 3 2

	// 두 행렬 A, D 를 곱하여 결과를 행렬 C 에 저장 
	for (int e=0; e<B; e++) {
		for (int b=0; b<K; b++) {
			double g = 0;
			for (int f=0; f<A; f++) {
				g += a[e*A + f] * d[f*K + b];
			}
			c[e*K + b] = g;
		}
	}
}

void Invert(double* src, double* dst, int size);

fpxyz unproject(float winx, float winy, float winz, GLdouble* modelMatrix, GLdouble* projMatrix, GLdouble* viewport)
{
	fpxyz ret = fpxyz(0,0,0);
	double tempMatrix[32];//16dschae
	multiplyMatrix(modelMatrix, projMatrix, tempMatrix, 4, 4, 4);
	double finalMatrix[32];//16dschae
	Invert(tempMatrix, finalMatrix, 4); // = numpy.linalg.inv(finalMatrix)

//	viewport = map(float, viewport)
	double tvector[32];//16dschae
	tvector[0] = (winx - viewport[0]) / viewport[2] * 2.0 - 1.0;
	tvector[1] = (winy - viewport[1]) / viewport[3] * 2.0 - 1.0;
	tvector[2] = winz * 2.0 - 1.0;
	tvector[3] = 1;

	// vector = (numpy.matrix(vector) * finalMatrix).getA().flatten();
	double vector[16];
	multiplyMatrix(tvector, finalMatrix, vector, 4, 1, 4);

	ret.data[0]= vector[0] / vector[3];
	ret.data[1]= vector[1] / vector[3];
	ret.data[2]= vector[2] / vector[3];

	return ret;
}

float* convert3x3MatrixTo4x4(vertexmat* matrix, float *f)
{
	for(int r = 0; r < matrix->size; r++) {
		fpxyz row = matrix->data[r];
		for(int c = 0; c < 3; c++) {
			f[r*4 + c] = row.data[c];
		}
		f[r*4 + 3] = 0;
	}
	f[3*4 + 0] = 0;
	f[3*4 + 1] = 0;
	f[3*4 + 2] = 0;
	f[3*4 + 3] = 1;

	return f;
}

void loadImage(wchar_t* file, GLuint tex)
{
    ULONG_PTR token;
    Gdiplus::GdiplusStartupInput input;
    Gdiplus::GdiplusStartup(&token, &input, NULL);
    {
        Gdiplus::Bitmap bmp(file);
        Gdiplus::Rect rect(0,0,bmp.GetWidth(), bmp.GetHeight());
        Gdiplus::BitmapData data;
        bmp.LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &data);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, data.Width);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, data.Width, data.Height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, data.Scan0);
        bmp.UnlockBits(&data);
    }
    Gdiplus::GdiplusShutdown(token);
}

/*
AUX_RGBImageRec requires glaux.h
AUX_RGBImageRec *LoadBMPFile(char *filename) 
{ 
    FILE *hFile = NULL; 
    if(!filename) return NULL; 
     
    hFile = fopen(filename, "r"); 
    if(hFile) { 
        fclose(hFile); 
        return auxDIBImageLoad(filename); 
    } 
     
    return NULL; 
}
*/
GLuint loadGLTexture(char* filename)
{
	TCHAR tcspath[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, filename, strlen(filename)+1, tcspath, MAX_PATH);
	return loadGLTexture(tcspath);
}

GLuint loadGLTexture(TCHAR* filename)
{
	GLuint tex[2];
	glGenTextures(1, tex);
	loadImage(filename, tex[0]);
/*
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	img = wx.ImageFromBitmap(wx.Bitmap(getPathForImage(filename)));
	rgbData = img.GetData();
	alphaData = img.GetAlphaData();
	if alphaData is not None) {
		data = ''
		for i in xrange(0, len(alphaData)):
			data += rgbData[i*3:i*3+3] + alphaData[i]
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.GetWidth(), img.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.GetWidth(), img.GetHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, rgbData);
	}
*/
	return tex[0];
}

/*
def ResetMatrixRotationAndScale():
	matrix = glGetFloatv(GL_MODELVIEW_MATRIX)
	noZ = False
	if matrix[3][2] > 0:
		return False
	scale2D = matrix[0][0]
	matrix[0][0] = 1.0
	matrix[1][0] = 0.0
	matrix[2][0] = 0.0
	matrix[0][1] = 0.0
	matrix[1][1] = 1.0
	matrix[2][1] = 0.0
	matrix[0][2] = 0.0
	matrix[1][2] = 0.0
	matrix[2][2] = 1.0

	if matrix[3][2] != 0.0:
		matrix[3][0] = matrix[3][0] / (-matrix[3][2] / 100)
		matrix[3][1] = matrix[3][1] / (-matrix[3][2] / 100)
		matrix[3][2] = -100
	else:
		matrix[0][0] = scale2D
		matrix[1][1] = scale2D
		matrix[2][2] = scale2D
		matrix[3][2] = -100
		noZ = True

	glLoadMatrixf(matrix)
	return noZ


def DrawBox(vMin, vMax):
	glBegin(GL_LINE_LOOP)
	glVertex3f(vMin[0], vMin[1], vMin[2])
	glVertex3f(vMax[0], vMin[1], vMin[2])
	glVertex3f(vMax[0], vMax[1], vMin[2])
	glVertex3f(vMin[0], vMax[1], vMin[2])
	glEnd()

	glBegin(GL_LINE_LOOP)
	glVertex3f(vMin[0], vMin[1], vMax[2])
	glVertex3f(vMax[0], vMin[1], vMax[2])
	glVertex3f(vMax[0], vMax[1], vMax[2])
	glVertex3f(vMin[0], vMax[1], vMax[2])
	glEnd()
	glBegin(GL_LINES)
	glVertex3f(vMin[0], vMin[1], vMin[2])
	glVertex3f(vMin[0], vMin[1], vMax[2])
	glVertex3f(vMax[0], vMin[1], vMin[2])
	glVertex3f(vMax[0], vMin[1], vMax[2])
	glVertex3f(vMax[0], vMax[1], vMin[2])
	glVertex3f(vMax[0], vMax[1], vMax[2])
	glVertex3f(vMin[0], vMax[1], vMin[2])
	glVertex3f(vMin[0], vMax[1], vMax[2])
	glEnd()


def DrawMeshOutline(mesh):
	glEnable(GL_CULL_FACE)
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, mesh.vertexes)

	glCullFace(GL_FRONT)
	glLineWidth(3)
	glPolygonMode(GL_BACK, GL_LINE)
	glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount)
	glPolygonMode(GL_BACK, GL_FILL)
	glCullFace(GL_BACK)

	glDisableClientState(GL_VERTEX_ARRAY)


def DrawMesh(mesh, insideOut = False):
	glEnable(GL_CULL_FACE)
	glEnableClientState(GL_VERTEX_ARRAY)
	glEnableClientState(GL_NORMAL_ARRAY)
	for m in mesh._meshList:
		glVertexPointer(3, GL_FLOAT, 0, m.vertexes)
		if insideOut:
			glNormalPointer(GL_FLOAT, 0, m.invNormal)
		else:
			glNormalPointer(GL_FLOAT, 0, m.normal)

		#Odd, drawing in batchs is a LOT faster then drawing it all at once.
		batchSize = 999    #Warning, batchSize needs to be dividable by 3
		extraStartPos = int(m.vertexCount / batchSize) * batchSize
		extraCount = m.vertexCount - extraStartPos

		glCullFace(GL_BACK)
		for i in xrange(0, int(m.vertexCount / batchSize)):
			glDrawArrays(GL_TRIANGLES, i * batchSize, batchSize)
		glDrawArrays(GL_TRIANGLES, extraStartPos, extraCount)

		glCullFace(GL_FRONT)
		if insideOut:
			glNormalPointer(GL_FLOAT, 0, m.normal)
		else:
			glNormalPointer(GL_FLOAT, 0, m.invNormal)
		for i in xrange(0, int(m.vertexCount / batchSize)):
			glDrawArrays(GL_TRIANGLES, i * batchSize, batchSize)
		extraStartPos = int(m.vertexCount / batchSize) * batchSize
		extraCount = m.vertexCount - extraStartPos
		glDrawArrays(GL_TRIANGLES, extraStartPos, extraCount)
		glCullFace(GL_BACK)

	glDisableClientState(GL_VERTEX_ARRAY)
	glDisableClientState(GL_NORMAL_ARRAY)


def DrawMeshSteep(mesh, matrix, angle):
	cosAngle = math.sin(angle / 180.0 * math.pi)
	glDisable(GL_LIGHTING)
	glDepthFunc(GL_EQUAL)
	normals = (numpy.matrix(mesh.normal, copy = False) * matrix).getA()
	for i in xrange(0, int(mesh.vertexCount), 3):
		if normals[i][2] < -0.999999:
			if mesh.vertexes[i + 0][2] > 0.01:
				glColor3f(0.5, 0, 0)
				glBegin(GL_TRIANGLES)
				glVertex3f(mesh.vertexes[i + 0][0], mesh.vertexes[i + 0][1], mesh.vertexes[i + 0][2])
				glVertex3f(mesh.vertexes[i + 1][0], mesh.vertexes[i + 1][1], mesh.vertexes[i + 1][2])
				glVertex3f(mesh.vertexes[i + 2][0], mesh.vertexes[i + 2][1], mesh.vertexes[i + 2][2])
				glEnd()
		elif normals[i][2] < -cosAngle:
			glColor3f(-normals[i][2], 0, 0)
			glBegin(GL_TRIANGLES)
			glVertex3f(mesh.vertexes[i + 0][0], mesh.vertexes[i + 0][1], mesh.vertexes[i + 0][2])
			glVertex3f(mesh.vertexes[i + 1][0], mesh.vertexes[i + 1][1], mesh.vertexes[i + 1][2])
			glVertex3f(mesh.vertexes[i + 2][0], mesh.vertexes[i + 2][1], mesh.vertexes[i + 2][2])
			glEnd()
		elif normals[i][2] > 0.999999:
			if mesh.vertexes[i + 0][2] > 0.01:
				glColor3f(0.5, 0, 0)
				glBegin(GL_TRIANGLES)
				glVertex3f(mesh.vertexes[i + 0][0], mesh.vertexes[i + 0][1], mesh.vertexes[i + 0][2])
				glVertex3f(mesh.vertexes[i + 2][0], mesh.vertexes[i + 2][1], mesh.vertexes[i + 2][2])
				glVertex3f(mesh.vertexes[i + 1][0], mesh.vertexes[i + 1][1], mesh.vertexes[i + 1][2])
				glEnd()
		elif normals[i][2] > cosAngle:
			glColor3f(normals[i][2], 0, 0)
			glBegin(GL_TRIANGLES)
			glVertex3f(mesh.vertexes[i + 0][0], mesh.vertexes[i + 0][1], mesh.vertexes[i + 0][2])
			glVertex3f(mesh.vertexes[i + 2][0], mesh.vertexes[i + 2][1], mesh.vertexes[i + 2][2])
			glVertex3f(mesh.vertexes[i + 1][0], mesh.vertexes[i + 1][1], mesh.vertexes[i + 1][2])
			glEnd()
	glDepthFunc(GL_LESS)

def DrawGCodeLayer(layer, drawQuick = True):
	filamentRadius = profile.getProfileSettingFloat('filament_diameter') / 2
	filamentArea = math.pi * filamentRadius * filamentRadius
	lineWidth = profile.getProfileSettingFloat('nozzle_size') / 2 / 10

	fillCycle = 0
	fillColorCycle = [[0.5, 0.5, 0.0, 1], [0.0, 0.5, 0.5, 1], [0.5, 0.0, 0.5, 1]]
	moveColor = [0, 0, 1, 0.5]
	retractColor = [1, 0, 0.5, 0.5]
	supportColor = [0, 1, 1, 1]
	extrudeColor = [[1, 0, 0, 1], [0, 1, 1, 1], [1, 1, 0, 1], [1, 0, 1, 1]]
	innerWallColor = [0, 1, 0, 1]
	skirtColor = [0, 0.5, 0.5, 1]
	prevPathWasRetract = False

	glDisable(GL_CULL_FACE)
	for path in layer:
		if path.type == 'move':
			if prevPathWasRetract:
				c = retractColor
			else:
				c = moveColor
			if drawQuick:
				continue
		zOffset = 0.01
		if path.type == 'extrude':
			if path.pathType == 'FILL':
				c = fillColorCycle[fillCycle]
				fillCycle = (fillCycle + 1) % len(fillColorCycle)
			elif path.pathType == 'WALL-INNER':
				c = innerWallColor
				zOffset = 0.02
			elif path.pathType == 'SUPPORT':
				c = supportColor
			elif path.pathType == 'SKIRT':
				c = skirtColor
			else:
				c = extrudeColor[path.extruder]
		if path.type == 'retract':
			c = retractColor
		if path.type == 'extrude' and not drawQuick:
			drawLength = 0.0
			prevNormal = None
			for i in xrange(0, len(path.points) - 1):
				v0 = path.points[i]
				v1 = path.points[i + 1]

				# Calculate line width from ePerDistance (needs layer thickness and filament diameter)
				dist = (v0 - v1).vsize()
				if dist > 0 and path.layerThickness > 0:
					extrusionMMperDist = (v1.e - v0.e) / dist
					lineWidth = extrusionMMperDist * filamentArea / path.layerThickness / 2 * v1.extrudeAmountMultiply

				drawLength += (v0 - v1).vsize()
				normal = (v0 - v1).cross(util3d.Vector3(0, 0, 1))
				normal.normalize()

				vv2 = v0 + normal * lineWidth
				vv3 = v1 + normal * lineWidth
				vv0 = v0 - normal * lineWidth
				vv1 = v1 - normal * lineWidth

				glBegin(GL_QUADS)
				glColor4fv(c)
				glVertex3f(vv0.x, vv0.y, vv0.z - zOffset)
				glVertex3f(vv1.x, vv1.y, vv1.z - zOffset)
				glVertex3f(vv3.x, vv3.y, vv3.z - zOffset)
				glVertex3f(vv2.x, vv2.y, vv2.z - zOffset)
				glEnd()
				if prevNormal is not None:
					n = (normal + prevNormal)
					n.normalize()
					vv4 = v0 + n * lineWidth
					vv5 = v0 - n * lineWidth
					glBegin(GL_QUADS)
					glColor4fv(c)
					glVertex3f(vv2.x, vv2.y, vv2.z - zOffset)
					glVertex3f(vv4.x, vv4.y, vv4.z - zOffset)
					glVertex3f(prevVv3.x, prevVv3.y, prevVv3.z - zOffset)
					glVertex3f(v0.x, v0.y, v0.z - zOffset)

					glVertex3f(vv0.x, vv0.y, vv0.z - zOffset)
					glVertex3f(vv5.x, vv5.y, vv5.z - zOffset)
					glVertex3f(prevVv1.x, prevVv1.y, prevVv1.z - zOffset)
					glVertex3f(v0.x, v0.y, v0.z - zOffset)
					glEnd()

				prevNormal = normal
				prevVv1 = vv1
				prevVv3 = vv3
		else:
			glColor4fv(c)
			glBegin(GL_TRIANGLES)
			for v in path.points:
				glVertex3f(v[0], v[1], v[2])
			glEnd()

		if not path.type == 'move':
			prevPathWasRetract = False
		#if path.type == 'retract' and path.points[0].almostEqual(path.points[-1]):
		#	prevPathWasRetract = True
	glEnable(GL_CULL_FACE)
*/