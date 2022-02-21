#include "stdafx.h"
#include "mesh.h"
#include "objectScene.h"
#include "polygon.h"
#include "profile.h"

/*
__copyright__ = "Copyright (C) 2013 David Braam - Released under terms of the AGPLv3 License"
import random
import numpy

from Cura.util import profile
from Cura.util import polygon
*/

_objectOrder::_objectOrder(int _order, int _todo)
{
	order = _order;
	todo = _todo;
}

_objectOrderFinder::_objectOrderFinder(Scene* scene, int leftToRight, int frontToBack, int gantryHeight)
{
	_memnew(this, "_objectOrderFinder", __FILE__, __LINE__);
	_scene = scene;
	_objs = scene->_objectList;
	_leftToRight = leftToRight;
	_frontToBack = frontToBack;
	
	intvector initialList(__FILE__, __LINE__);
	for (int n = 0; n < _objs->length; n++) {
		if( scene->checkPlatform(_objs->data[n]))
			initialList.add(n);
	}

	for(int itn = 0; itn < initialList.len; itn++) {
		int n = initialList.data[itn];
		if(_objs->data[n]->getSize()[2] > gantryHeight && initialList.len > 1) {
			if(order != NULL)
				delete order;
			order = NULL;
			return;
		}
	}
	if(initialList.len == 0) {
		if(order != NULL)
			delete order;
		order = new intvector(__FILE__, __LINE__);
		return;
	}

	int m = initialList.intmax() + 1;
	_hitMap = new pintvector;
	_hitMap->resize(m);

	for(int ai = 0; ai < initialList.len; ai++) {
		int a = initialList.data[ai];
		_hitMap->data()[a] = new intvector(__FILE__, __LINE__);
		_hitMap->data()[a]->resize(m, 0);
		for(int bi = 0; bi < initialList.len; bi++) {
			int b = initialList.data[bi];
			_hitMap->data()[a]->data[b] = _checkHit(a, b);
		}
	}

//	#Check if we have 2 files that overlap so that they can never be printed one at a time.
	for(int ai = 0; ai < initialList.len; ai++) {
		int a = initialList.data[ai];
		for(int bi = 0; bi < initialList.len; bi++) {
			int b = initialList.data[bi];
			if( a != b && _hitMap->data()[a]->data[b] && _hitMap->data()[b]->data[a]) {
				if(order != NULL)
					delete order;
				order = NULL;
				return;
			}
		}
	}

	sortInitialList(initialList);
/*
	int n = 0;
	this->_todo = [_objectOrder([], initialList)]
	while len(this->_todo) > 0:
		n += 1
		current = this->_todo.pop()
		// #print len(this->_todo), len(current.order), len(initialList), current.order
		for addIdx in current.todo:
			if not this->_checkHitFor(addIdx, current.order) and not this->_checkBlocks(addIdx, current.todo):
				todoList = current.todo[:]
				todoList.remove(addIdx)
				order = current.order[:] + [addIdx]
				if len(todoList) == 0:
					this->_todo = None
					this->order = order
					return
				this->_todo.append(_objectOrder(order, todoList))
	this->order = None
*/
}


void _objectOrderFinder::sortInitialList(intvector& pi)
{
	for(int i = 0; i < pi.len-1; i++) {
		for(int j = i + 1; j < pi.len; j++) {
			int scoreA = _hitMap->data()[i]->getsum();
			int scoreB = _hitMap->data()[j]->getsum();
			if(scoreA > scoreB) {
				int temp = pi.data[i];
				pi.data[i] = pi.data[j];
				pi.data[j] = temp;
			}
		}
	}
}


int _objectOrderFinder::_checkHitFor(int addIdx, intvector& others)
{
	for(int idxn = 0; idxn < others.len; idxn++) {
		int idx = others.data[idxn];
		if(_hitMap->data()[addIdx]->data[idx])
			return 1;
	}
	return 0;
}


int _objectOrderFinder::_checkBlocks(int addIdx, intvector& others)
{
	for(int idxn = 0; idxn < others.len; idxn++) {
		int idx = others.data[idxn];
		if (addIdx != idx && _hitMap->data()[idx]->data[addIdx])
			return 1;
	}

	return 0;
}


//	#Check if printing one object will cause printhead colission with other object.
int _objectOrderFinder::_checkHit(int addIdx, int idx)
{
	printableObject* obj = _scene->_objectList->data[idx];
	printableObject* addObj = _scene->_objectList->data[addIdx];

	fpxyvect* objvect = obj->_boundaryHull + obj->getPosition();
	fpxyvect* addobjvect = addObj->_headAreaHull + addObj->getPosition();

	int ret = polygon.polygonCollision(objvect, addobjvect);
	delete objvect;
	delete addobjvect;

	return ret;
}

Scene::Scene()
{
	_memnew(this, "Scene", __FILE__, __LINE__);
	_objectList = new cObjectList(__FILE__, __LINE__);

	_machineSize = fpxyz(100,100,100);
	_headSizeOffsets = fpxy(18.0,18.0);
	_extruderOffset[0] = fpxy(0,0);
	_extruderOffset[1] = fpxy(0,0);
	_extruderOffset[2] = fpxy(0,0);
	_extruderOffset[3] = fpxy(0,0);

	_sizeOffsets = fpxy(0.0, 0.0);
	_minExtruderCount = -1;
	/*
def __init__(self):
		this->_objectList = []

*/
//		#Print order variables
	this->_leftToRight = 0;
	this->_frontToBack = 1;
	this->_gantryHeight = 60;
	this->_oneAtATime = 1;
	_machinePolygons = NULL;
}

Scene::~Scene()
{
	_memdel(this);

	if(_machinePolygons != NULL)
		delete _machinePolygons;
	if(_objectList != NULL)
		delete 	_objectList;
}

//	# update the physical machine dimensions
void Scene::updateMachineDimensions()
{
		this->_machineSize = GetMachineDimensions();

		if(this->_machinePolygons != NULL)
			delete this->_machinePolygons;
		this->_machinePolygons = profile.getMachineSizePolygons();
		this->updateHeadSize();
}

//	# Size offsets are offsets caused by brim, skirt, etc.
void Scene::updateSizeOffsets(int force)
{
	fpxy newOffsets = profile.calculateObjectSizeOffsets();
	int minExtruderCount = profile.minimalExtruderCount();
	if (!force && _sizeOffsets == newOffsets && this->_minExtruderCount == minExtruderCount)
		return;
	_sizeOffsets = newOffsets;
	_minExtruderCount = minExtruderCount;

	fpxyvectvect* extends = new fpxyvectvect(__FILE__, __LINE__);
	fpxyvect *v1 = new fpxyvect(0, __FILE__, __LINE__);
	v1->append(fpxy(-newOffsets.x,-newOffsets.y));
	v1->append(fpxy(newOffsets.x,-newOffsets.y));
	v1->append(fpxy(newOffsets.x, newOffsets.y));
	v1->append(fpxy(-newOffsets.x, newOffsets.y));
	extends->add(v1);

	for(int n = 1; n < 4; n++) {
		fpxyvect* headOffset = new fpxyvect(0, __FILE__, __LINE__);
		headOffset->append(fpxy(0, 0));
		char b1[64];
		sprintf_s(b1, 64, "extruder_offset_x%d", n);
		float fx = -profile.getMachineSettingFloat(b1);
		sprintf_s(b1, 64, "extruder_offset_y%d", n);
		float fy = -profile.getMachineSettingFloat(b1);
		headOffset->append(fpxy(fx, fy));
		fpxyvect* vm = polygon.minkowskiHull( extends->data[n-1], headOffset, __FILE__, __LINE__);
		extends->add(vm); // call minkowskiHull with last polygon(n-1 th polygon)
		delete headOffset;
	}

	if(minExtruderCount > 1) {
		delete extends->data[0];
		extends->data[0] = extends->data[1]->copy(__FILE__, __LINE__);
	}

	for(int objn = 0; objn < this->_objectList->length; objn++) {
		printableObject* obj = this->_objectList->data[objn];
		obj->setPrintAreaExtends(extends->data[obj->_meshList->len - 1]);
	}

	delete extends;
}

//	#size of the printing head.
void Scene::updateHeadSize(printableObject* obj)
{
	float xMin = profile.getMachineSettingFloat("extruder_head_size_min_x");
	float xMax = profile.getMachineSettingFloat("extruder_head_size_max_x");
	float yMin = profile.getMachineSettingFloat("extruder_head_size_min_y");
	float yMax = profile.getMachineSettingFloat("extruder_head_size_max_y");
	float gantryHeight = profile.getMachineSettingFloat("extruder_head_size_height");

	this->_leftToRight = xMin < xMax;
	this->_frontToBack = yMin < yMax;
	this->_headSizeOffsets.x = min(xMin, xMax);
	this->_headSizeOffsets.y = min(yMin, yMax);
	this->_gantryHeight = gantryHeight;
	this->_oneAtATime = this->_gantryHeight > 0 && strcmp(profile.getPreference("oneAtATime"), "True") == 0;
	for(int objn = 0; objn < _objectList->length; objn++) { // for obj in this->_objectList) {
		printableObject* obj = _objectList->data[objn];
		if( obj->getSize()[2] > this->_gantryHeight)
			this->_oneAtATime = 0;
	}

	fpxyvect *headArea = new fpxyvect(0, __FILE__, __LINE__);
	headArea->append( fpxy(-xMin,-yMin));
	headArea->append( fpxy( xMax,-yMin));
	headArea->append( fpxy( xMax, yMax));
	headArea->append( fpxy(-xMin, yMax));

	if( obj == NULL) {
		for(int objn = 0; objn < _objectList->length; objn++) { //			for obj in this->_objectList)
			printableObject* obj = _objectList->data[objn];
			obj->setHeadArea(headArea, this->_headSizeOffsets);
		}
	} else {
		obj->setHeadArea(headArea, this->_headSizeOffsets);
	}

	delete headArea;
}


int Scene::isOneAtATime()
{
		return this->_oneAtATime;
}


void Scene::setExtruderOffset( int extruderNr, float offsetX, float offsetY)
{
	this->_extruderOffset[extruderNr] = fpxy(offsetX, offsetY);
}

/*
Scene::objects(self)
{
		return this->_objectList;
}
*/

float float_max(float* fa, int start, int end)
{
	float fmax = fa[start];
	for(int i = start+1; i < end; i++) {
		if(fa[i] > fmax)
			fmax = fa[i];
	}
	return fmax;
}

//	#Add new object to print area
void Scene::add( printableObject* obj)
{
	if (float_max(obj->getSize(), 0, 2) > float_max(this->_machineSize.data, 0, 2) * 2.5) {
		float scale = float_max(this->_machineSize.data, 0, 2) * 2.5 / float_max(obj->getSize(), 0, 2);
		vertexmat* matrix = new vertexmat(__FILE__, __LINE__);
		matrix->append(scale,0,0);
		matrix->append(0, scale, 0);
		matrix->append(0, 0, scale);
		obj->applyMatrix(*matrix);
		delete matrix;
	}
	this->_findFreePositionFor(obj);
	this->_objectList->append(obj); // .append(obj);
	this->updateHeadSize(obj);
	this->updateSizeOffsets(1);
	this->pushFree(obj);
}


void Scene::remove(printableObject* obj)
{
	this->_objectList->remove(obj);
}

//	#Dual(multiple) extrusion merge
void Scene::merge(printableObject* obj1, printableObject* obj2)
{
	this->remove(obj2);
	for(int n2 = 0; n2 < obj2->_meshList->len; n2++) {
		obj2->_meshList->data[n2]->_obj = obj1;
		obj1->_meshList->add(obj2->_meshList->data[n2]);
	}
/*
for m in obj2._meshList) {
		m._obj = obj1;
	}
*/
	obj1->processMatrix();
	fpxy p = obj1->getPosition() + obj2->getPosition();
	p /= 2;
	obj1->setPosition(p);
	this->pushFree(obj1);
}

void Scene::pushFree(printableObject* staticObj)
{
	if(this->checkPlatform(staticObj))
		return;

	cObjectList pushList(__FILE__, __LINE__);
	for(int n = 0; n < _objectList->length; n++) { //for obj in this->_objectList) {
		printableObject* obj = _objectList->data[n];
		if(obj == staticObj || !this->checkPlatform(obj))
			continue;

		int ret;
		fpxy v(-9876543210, -9876543210);
		if(this->_oneAtATime) {
			fpxyvect* p1 = obj->_headAreaMinHull + obj->getPosition();
			fpxyvect* p2 = staticObj->_boundaryHull + staticObj->getPosition();
			ret = polygon.polygonCollisionPushVector(p1, p2, v);
			delete p1;
			delete p2;
		} else {
			fpxyvect* p1 = obj->_boundaryHull + obj->getPosition();
			fpxyvect* p2 = staticObj->_boundaryHull + staticObj->getPosition();
			ret = polygon.polygonCollisionPushVector(p1, p2, v);
			delete p1;
			delete p2;
		}
		if(!ret)
			continue;

		fpxy fv = obj->getPosition() + v;
		fv *= 1.01;
		obj->setPosition(fv);
		pushList.append(obj);
	}
	for(int no = 0; no < pushList.length; no++) { // for obj in pushList) {
		printableObject* obj = pushList.data[no];
		this->pushFree(obj);
	}
}


void Scene::arrangeAll()
{
	// save objects
	cObjectList *oldList = new cObjectList(__FILE__, __LINE__);
	for(int i = 0; i < this->_objectList->length; i++)
		oldList->append(this->_objectList->data[i]);
	// clear original
	_objectList->length = 0;
	
	for(int on = 0; on < oldList->length; on++) { // obj in oldList) {
		printableObject* obj = oldList->data[on];
		obj->setPosition(fpxy(0,0)); // numpy.array([0,0], numpy.float32))
		this->add(obj);
	}

	oldList->length = 0;
	delete oldList;
}


void Scene::centerAll()
{
	fpxy minPos(9999999, 9999999);
	fpxy maxPos(-9999999,-9999999);

	for(int obn = 0; obn < _objectList->length; obn++) { //	for obj in this->_objectList) {
		printableObject* obj = _objectList->data[obn];
		fpxy pos = obj->getPosition();
		fpxy size(obj->getSize()[0], obj->getSize()[1]);
		
		minPos.x = min(minPos.x, pos.x - size.x / 2);
		minPos.y = min(minPos.y, pos.y - size.y / 2);
		maxPos.x = max(maxPos.x, pos.x + size.x / 2);
		maxPos.y = max(maxPos.y, pos.y + size.y / 2);
	}

	fpxy offset = maxPos + minPos;
	offset /= 2;
	offset *= -1;

	for(int on = 0; on < this->_objectList->length; on++) {
		printableObject* obj = _objectList->data[on];
		obj->setPosition(obj->getPosition() + offset);
	}
}


intvector* Scene::printOrder()
{
	intvector* order = NULL;
	if (this->_oneAtATime) {
		_objectOrderFinder of( this, this->_leftToRight, this->_frontToBack, this->_gantryHeight);
		order = of.order;
	} else
		order = NULL;
	return order;
}

//	#Check if two objects are hitting each-other (+ head space).
int Scene::_checkHit(printableObject* a, printableObject* b)
{
	if (a == b)
		return False;
	if (this->_oneAtATime) {
		fpxyvect* p1 = a->_headAreaMinHull + a->getPosition();
		fpxyvect* p2 = b->_boundaryHull + b->getPosition();
		int ret = polygon.polygonCollision(p1, p2);
		delete p1;
		delete p2;
		return ret;
	} else {
		fpxyvect* p1 = a->_boundaryHull + a->getPosition();
		fpxyvect* p2 = b->_boundaryHull + b->getPosition();
		int ret = polygon.polygonCollision(p1, p2);
		delete p1;
		delete p2;
		return ret;
	}
}

int Scene::checkPlatform(printableObject* obj)
{
	int ret = False;
	fpxyvect* area = obj->_printAreaHull + obj->getPosition();

	if (! polygon.fullInside(area, this->_machinePolygons->data[0])) {
		ret = False;
		goto _return;
	}
//		#Check the "no go zones"
	for(int pn = 0; pn < this->_machinePolygons->len; pn++) {
		fpxyvect* poly = this->_machinePolygons->data[pn];
		if (polygon.polygonCollision(poly, area)) {
			ret = False;
			goto _return;
		}
	}

	ret = True;

_return:
	delete area;
	return ret;
}

void Scene::_findFreePositionFor(printableObject* obj)
{
	fpxyvect posList(0, __FILE__, __LINE__);
	for(int an = 0; an <this->_objectList->length; an++) {
		printableObject*a = _objectList->data[an];
		fpxy p = a->getPosition();
		fpxy s;
		if (this->_oneAtATime) {
			s = (fpxy(a->getSize()) + fpxy(obj->getSize())) / 2 + this->_sizeOffsets + this->_headSizeOffsets + fpxy(3,3);
		} else {
			s = (fpxy(a->getSize()) + fpxy(obj->getSize())) / 2 + fpxy(3,3);
		}
		posList.append(p + s * ( 1.0, 1.0));
		posList.append(p + s * ( 0.0, 1.0));
		posList.append(p + s * (-1.0, 1.0));
		posList.append(p + s * ( 1.0, 0.0));
		posList.append(p + s * (-1.0, 0.0));
		posList.append(p + s * ( 1.0,-1.0));
		posList.append(p + s * ( 0.0,-1.0));
		posList.append(p + s * (-1.0,-1.0));
	}
	fpxy best;
	float bestDist = 0;
	int found = 0;

	for(int pn = 0; pn < posList.size; pn++) {
		fpxy p = posList.data[pn];
		obj->setPosition(p);
		int ok = True;
		for(int an = 0; an < this->_objectList->length; an++) {
			printableObject* a = _objectList->data[an];
			if (this->_checkHit(a, obj)) {
				ok = False;
				break;
			}
		}
		if (!ok )
			continue;
		float dist = norm(p);
		if (!this->checkPlatform(obj))
			dist *= 3;
		if (!found || dist < bestDist) {
			best = p;
			bestDist = dist;
		}
	}
	if (found)
		obj->setPosition(best);
}
