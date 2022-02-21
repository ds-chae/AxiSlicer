/*
def loadSupportedExtensions():
	return ['.stl', '.obj', '.dae', '.amf']

def saveSupportedExtensions():
	return ['.amf', '.stl']

def loadWildcardFilter():
	wildcardList = ';'.join(map(lambda s: '*' + s, loadSupportedExtensions()))
	return "Mesh files (%s)|%s;%s" % (wildcardList, wildcardList, wildcardList.upper())

def saveWildcardFilter():
	wildcardList = ';'.join(map(lambda s: '*' + s, saveSupportedExtensions()))
	return "Mesh files (%s)|%s;%s" % (wildcardList, wildcardList, wildcardList.upper())
*/

//#loadMeshes loads 1 or more printableObjects from a file.
//# STL files are a single printableObject with a single mesh, these are most common.
//# OBJ files usually contain a single mesh, but they can contain multiple meshes
//# AMF can contain whole scenes of objects with each object having multiple meshes.
//# DAE files are a mess, but they can contain scenes of objects as well as grouped meshes

printableObjectList* loadMeshes(TCHAR* filename);
/*
def saveMeshes(filename, objects):
	ext = os.path.splitext(filename)[1].lower()
	if ext == '.stl':
		stl.saveScene(filename, objects)
		return
	if ext == '.amf':
		amf.saveScene(filename, objects)
		return
	print 'Error: Unknown model extension: %s' % (ext)
*/
