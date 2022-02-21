#include <vector>
#include <array>

#include "mesh.h"

#ifndef _OBJECTSCENE_H_
#define _OBJECTSCENE_H_

class Scene;

class _objectOrder
{
public:
	_objectOrder(int _order, int _todo);

	int order;
	int todo;
};

class cObjectList
{
public:
#define	OBJL_SIZE	20
	int length;
	printableObject* data[OBJL_SIZE];

	cObjectList(char* _file, int _line)
	{
		_memnew(this, "cObjectList", _file, _line);
		length = 0;
	}

	~cObjectList()
	{
		_memdel(this);
		for(int i = 0; i < length; i++) {
			printableObject* obj = data[i];
			if(obj != NULL)
				delete obj;
		}
	}
/*
	printableObject& operator [](const int& i)
	{
		return *data[i];
	}
*/
	void append(printableObject* o)
	{
		if(length < OBJL_SIZE) {
			data[length] = o;
			length++;
		}
	}

	void remove(printableObject* o)
	{
		for(int n = 0; n < length; n++) {
			if(data[n] == o) {
				while(n < length-1) {
					data[n] = data[n+1];
				}
				length--;
				return;
			}
		}
	}
};

class _objectOrderFinder
{
public:
	_objectOrderFinder(Scene* scene, int leftToRight, int frontToBack, int gantryHeight);
	~_objectOrderFinder()
	{
		_memdel(this);
	}

	Scene *_scene;
	cObjectList *_objs;
	int _leftToRight;
	int _frontToBack;
	intvector* order;
	pintvector* _hitMap;

	void sortInitialList(intvector& pi);
	int   _checkBlocks(int addIdx, intvector& others);
	int   _checkHitFor(int addIdx, intvector& others);
	// #Check if printing one object will cause printhead colission with other object.
	int _checkHit(int addIdx, int idx);
};

class Scene
{
public:
	Scene();
	~Scene();

	cObjectList *_objectList;
	fpxyz _machineSize;
	fpxy  _headSizeOffsets;
	fpxyvectvect* _machinePolygons;
	fpxy _sizeOffsets;
	int  _minExtruderCount;

	fpxy _extruderOffset[4];

	//		#Print order variables
	int _leftToRight;
	int _frontToBack;
	float _gantryHeight;
	int _oneAtATime;

	// # update the physical machine dimensions
	void updateMachineDimensions();

	// # Size offsets are offsets caused by brim, skirt, etc.
	void updateSizeOffsets(int force=0);

	// #size of the printing head.
	void updateHeadSize(printableObject* obj = NULL);

	int isOneAtATime();

	void setExtruderOffset(int extruderNr, float offsetX, float offsetY);
/*
	def objects(self):
		return self._objectList
*/
	// #Add new object to print area
	void add(printableObject* obj);

	void remove(printableObject* obj);

	// #Dual(multiple) extrusion merge
	void merge(printableObject* obj1, printableObject* obj2);

	void pushFree(printableObject* staticObj);

	void arrangeAll();

	void centerAll();

	intvector* printOrder();

	// #Check if two objects are hitting each-other (+ head space).
	int _checkHit(printableObject* a, printableObject* b);

	int checkPlatform(printableObject* obj);

	void _findFreePositionFor(printableObject* obj);
};

#endif //  _OBJECTSCENE_H_