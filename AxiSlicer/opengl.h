#ifndef _OPENGL_H_
#define	_OPENGL_H_

/*
glutInit()

platformMesh = None
*/
class GLReferenceCounter
{
public:
	int _refCounter;
	GLReferenceCounter()
	{
		_memnew(this, "GLReferenceCounter", __FILE__, __LINE__);
		_refCounter = 1;
	}
	~GLReferenceCounter()
	{
		_memdel(this);
	}

	void incRef() { _refCounter += 1 ; }
	int  decRef()
	{
		_refCounter -= 1;
		return _refCounter <= 0;
	}
}; // class GLReferenceCounter

int hasShaderSupport();

//#A Class that acts as an OpenGL shader, but in reality is not none.
class GLFakeShader : public GLReferenceCounter
{
public:
	GLFakeShader() : GLReferenceCounter()
	{
		_memnew(this, "GLFakeShader", __FILE__, __LINE__);
	}
	~GLFakeShader()
	{
		_memdel(this);
	}
	virtual void bind();
	virtual void unbind();
	virtual void release();
	virtual void setUniform(char* name, float value);
	virtual void setUniform(char* name, float* value);
	virtual int  isValid();
	virtual char* getVertexShader();
	virtual char* getFragmentShader();
};

class GLShader : public GLFakeShader
{
public:
	char* _vertexString;
	char* _fragmentString;
	GLuint _program;

	GLShader(char* vertexProgram, char* fragmentProgram);
	virtual void bind();
	virtual void unbind();
	virtual void release();
	virtual void setUniform(char* name, float value);
	virtual void setUniform(char* name, float* value);
	virtual int  isValid();
	virtual char* getVertexShader();
	virtual char* getFragmentShader();

	void __del__();
};

class GLVBO : public GLReferenceCounter
{
public:
	float* _vertexArray;
	float* _normalArray;
	int		_size;
	GLuint _buffer;
	int _hasNormals;

	int deletevertex;
	vertexmat* originalvertex;
	vertexmat* originalnormal;

	GLVBO(vertexmat* vertexArray, vertexmat* normalArray, int _deletevertex, char* _file, int _line); // : GLReferenceCounter();
	~GLVBO()
	{
		_memdel(this);
		if(this->deletevertex) {
			if(this->originalvertex != NULL)
				delete this->originalvertex;
			if(this->originalnormal != NULL)
				delete this->originalnormal;
		}

		_memdel(this);
	}
	void render(int render_type = GL_TRIANGLES);
	void release();
	void __del__();
}; // GLVBO

class GLVBOList
{
public:
	GLVBO **data;
	int   cap;
	int   len;

	GLVBOList()
	{
		cap = len = 0;
		data = NULL;
	}
	~GLVBOList()
	{
		for(int i = 0; i < len; i++)
			delete data[i];
		free(data);
	}

	void append(GLVBO *vbo)
	{
		if(len >= cap) {
			cap += 100;
			GLVBO** ndata = (GLVBO**)malloc(cap * sizeof(GLVBO*));
			for(int i = 0; i < len; i++)
				data[i] = ndata[i];
			if(data != NULL)
				free(data);
			data = ndata;
		}
		data[len++] = vbo;
	}

	GLVBO& operator [](const int& i)
	{
		return *data[i];
	}
};

class GLVBOListList
{
public:
	GLVBOList **data;
	int   cap;
	int   len;

	GLVBOListList()
	{
		cap = len = 0;
		data = NULL;
	}
	~GLVBOListList()
	{
		for(int i = 0; i < len; i++)
			delete data[i];
		if(data != NULL)
			free(data);
	}

	void append(GLVBOList *vbolist)
	{
		if(len >= cap) {
			cap += 100;
			GLVBOList** ndata = (GLVBOList**)malloc(cap * sizeof(GLVBOList*));
			for(int i = 0; i < len; i++)
				ndata[i] = data[i];
			if(data != NULL)
				free(data);
			data = ndata;
		}
		data[len++] = vbolist;
	}
};

void glDrawStringCenter(TCHAR* s);
fpxy glGetStringSize(TCHAR* s);
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
void glDrawStringRight(TCHAR* s);
void glDrawQuad(float x, float y, float w, float h);
void glDrawTexturedQuad(int x, int y, int w, int h, int texID, int mirror = 0);
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
fpxyz unproject(float winx, float winy, float winz, GLdouble* modelMatrix, GLdouble* projMatrix, GLdouble* viewport);

float* convert3x3MatrixTo4x4(vertexmat* matrix, float *f);
GLuint loadGLTexture(TCHAR* filename);
GLuint loadGLTexture(char* filename);
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
#endif	//_OPENGL_H_
