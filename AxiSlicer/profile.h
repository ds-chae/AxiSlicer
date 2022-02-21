/*
from __future__ import absolute_import
from __future__ import division
__copyright__ = "Copyright (C) 2013 David Braam - Released under terms of the AGPLv3 License"

import os, traceback, math, re, zlib, base64, time, sys, platform, glob, string, stat, types
import cPickle as pickle
import numpy
if sys.version_info[0] < 3:
	import ConfigParser
else:
	import configparser as ConfigParser

from Cura.util import resources
from Cura.util import version
from Cura.util import validators
*/

//#The settings dictionary contains a key/value reference to all possible settings. With the setting name as key.
//settingsDictionary = {};
//#The settings list is used to keep a full list of all the settings. This is needed to keep the settings in the proper order,
//# as the dictionary will not contain insertion order.
//settingsList = [];

//#Currently selected machine (by index) Cura support multiple machines in the same preferences and can switch between them.
//# Each machine has it's own index and unique name.
extern int _selectedMachineIndex;

typedef struct __char128 {
	char buf[128];
} _tchar128;

#define	FloatType	1
#define	IntType		2
#define	StrType		3
#define	OptType		4
#define	BoolType	5

class OptStr
{
public:
	char* str;
	OptStr(char *_s) {
		_memnew(this, "OptStr", "", __LINE__);
		str = _s;
	}
	~OptStr()
	{
		_memdel(this);
	}
};

class cSetting
{
public:
	/*
	#A setting object contains a configuration setting. These are globally accessible trough the quick access functions
	# and trough the settingsDictionary function.
	# Settings can be:
	# * profile settings (settings that effect the slicing process and the print result)
	# * preferences (settings that effect how cura works and acts)
	# * machine settings (settings that relate to the physical configuration of your machine)
	# * alterations (bad name copied from Skeinforge. These are the start/end code pieces)
	# Settings have validators that check if the value is valid, but do not prevent invalid values!
	# Settings have conditions that enable/disable this setting depending on other settings. (Ex: Dual-extrusion)
	*/
	char *_name;
	char *_label;
	char *_tooltip;
	char *_default;
	char *_coptions;

	std::vector<_tchar128> _values;

	int  _type; // FloatType, IntType, StrType, OptType
	char *_category;
	char *_subcategory;
	std::vector<_tchar128> _validators;
	std::vector<_tchar128> _conditions;

	float _minValue;
	float _maxValue;

	cSetting(char* name, char* __default, OptStr* opt, char* category, char* subcategory);
	cSetting(char* name, char* __default, int type, char* category, char* subcategory);
	~cSetting();

	void __init(char* name, char* category, char* subcategory);
	cSetting& setLabel(char* label, char* tooltip = "");
	cSetting& setRange(float minValue=-1, float maxValue=-1);
	void getLabel();
	void getTooltip();
	void getCategory();
	void getSubCategory();
	int  isPreference();
	int isMachineSetting();
	int isAlteration();
	int isProfile();
	char* getName();
	int  getType();
	char* getDefault();
	void setValue(char* value, int index = 0);
	char* getValue(int index = 0);
	int  getValueIndex();
	void validate();
	void addCondition(char* conditionFunction);
	void checkConditions();
};

/*
#########################################################
## Profile and preferences functions
#########################################################
*/
class cProfile
{
public:
	cProfile();
//	~cProfile();
	void __init();
	void __deinit();

	void getSubCategoriesFor(char* category);
	void getSettingsForCategory(char* category, char* subCategory = NULL);
	//## Profile functions
	char* getBasePath();
	char* getAlternativeBasePaths();
	char* getDefaultProfilePath();
	void loadProfile(LPCTSTR filename);
	void saveProfile(TCHAR* filename);
	void resetProfile();
	void setProfileFromString(char* options);
	void getProfileString();
	void insertNewlines(char* string, int every=64); //: #This should be moved to a better place then profile.
	char* getPreferencesString();
	char* getProfileSetting(char* name);
	float getProfileSettingFloat(char* name);
	void putProfileSetting(char* name, char* value);
	int isProfileSetting(char* name);
	//## Preferences functions
	char* getPreferencePath();
	float getPreferenceFloat(char* name);
	void getPreferenceColour(char* name, float* colors);
	void loadPreferences(char* filename);
	void loadMachineSettings(char* filename);
	void savePreferences(char* filename);
	char* getPreference(char* name);
	void putPreference(char* name, char* value);
	void putPreference(char* name, int index);
	int  isPreference(char* name);
	float getMachineSettingFloat(char* name, int index = 0);
	char* getMachineSetting(char* name, int index = 0);
	void putMachineSetting(char* name, char* value);
	int  isMachineSetting(char* name);
	void checkAndUpdateMachineName();
	int  getMachineCount();
	void setActiveMachine(int index);
	void removeMachine(char* index);
	//## Temp overrides for multi-extruder slicing and the project planner.
	//tempOverride = {}
	void setTempOverride(char* name, char* value);
	void clearTempOverride(char* name);
	void resetTempOverride();

	/*
	#########################################################
	## Utility functions to calculate common profile values
	#########################################################
	*/
	float calculateEdgeWidth();
	float calculateLineCount();
	float calculateSolidLayerCount();
	fpxy calculateObjectSizeOffsets();
	fpxy getMachineCenterCoords();

	//#Returns a list of convex polygons, first polygon is the allowed area of the machine,
	//# the rest of the polygons are the dis-allowed areas of the machine.
	fpxyvectvect* cProfile::getMachineSizePolygons();

	//#returns the number of extruders minimal used. Normally this returns 1, but with dual-extrusion support material it returns 2
	int minimalExtruderCount();

	/*#########################################################
	## Alteration file functions
	#########################################################*/
	void replaceTagMatch(char* m);

	void replaceGCodeTags(char* filename, int gcodeInt);

	void replaceGCodeTagsFromSlicer(char* filename, int slicerInt);
};

extern cProfile profile;

//char* getPreference(char* name);
void setActiveMachine(int index);
void checkAndUpdateMachineName();
int getMachineCount();
char* getProfileSetting(char* name);
char* getPreferencePath();
char* getDefaultProfilePath();

#define	DEFAULT_WIDTH	200
#define	DEFAULT_LENGTH	200
#define	DEFAULT_HEIGHT	200

fpxyz GetMachineDimensions();

/*
### Get aleration raw contents. (Used internally in Cura)
def getAlterationFile(filename):
	if filename in tempOverride:
		return tempOverride[filename]
	global settingsDictionary
	if filename in settingsDictionary and settingsDictionary[filename].isAlteration():
		return settingsDictionary[filename].getValue()
	traceback.print_stack()
	sys.stderr.write('Error: "%s" not found in alteration settings\n' % (filename))
	return ''

def setAlterationFile(name, value):
	#Check if we have a configuration file loaded, else load the default.
	global settingsDictionary
	if name in settingsDictionary and settingsDictionary[name].isAlteration():
		settingsDictionary[name].setValue(value)
	saveProfile(getDefaultProfilePath())

def isTagIn(tag, contents):
	contents = re.sub(';[^\n]*\n', '', contents)
	return tag in contents
*/
//### Get the alteration file for output. (Used by Skeinforge)
char* getAlterationFileContents(char* filename, int extruderCount = 1);
/*	prefix = ''
	postfix = ''
	alterationContents = getAlterationFile(filename)
	if getMachineSetting('gcode_flavor') == 'UltiGCode':
		if filename == 'end.gcode':
			return 'M25 ;Stop reading from this point on.\n;CURA_PROFILE_STRING:%s\n' % (getProfileString())
		return ''
	if filename == 'start.gcode':
		if extruderCount > 1:
			alterationContents = getAlterationFile("start%d.gcode" % (extruderCount))
		#For the start code, hack the temperature and the steps per E value into it. So the temperature is reached before the start code extrusion.
		#We also set our steps per E here, if configured.
		eSteps = getMachineSettingFloat('steps_per_e')
		if eSteps > 0:
			prefix += 'M92 E%f\n' % (eSteps)
		temp = getProfileSettingFloat('print_temperature')
		bedTemp = 0
		if getMachineSetting('has_heated_bed') == 'True':
			bedTemp = getProfileSettingFloat('print_bed_temperature')

		if bedTemp > 0 and not isTagIn('{print_bed_temperature}', alterationContents):
			prefix += 'M140 S%f\n' % (bedTemp)
		if temp > 0 and not isTagIn('{print_temperature}', alterationContents):
			if extruderCount > 0:
				for n in xrange(1, extruderCount):
					t = temp
					if n > 0 and getProfileSettingFloat('print_temperature%d' % (n+1)) > 0:
						t = getProfileSettingFloat('print_temperature%d' % (n+1))
					prefix += 'M104 T%d S%f\n' % (n, t)
				for n in xrange(0, extruderCount):
					t = temp
					if n > 0 and getProfileSettingFloat('print_temperature%d' % (n+1)) > 0:
						t = getProfileSettingFloat('print_temperature%d' % (n+1))
					prefix += 'M109 T%d S%f\n' % (n, t)
				prefix += 'T0\n'
			else:
				prefix += 'M109 S%f\n' % (temp)
		if bedTemp > 0 and not isTagIn('{print_bed_temperature}', alterationContents):
			prefix += 'M190 S%f\n' % (bedTemp)
	elif filename == 'end.gcode':
		if extruderCount > 1:
			alterationContents = getAlterationFile("end%d.gcode" % (extruderCount))
		#Append the profile string to the end of the GCode, so we can load it from the GCode file later.
		postfix = ';CURA_PROFILE_STRING:%s\n' % (getProfileString())
	return unicode(prefix + re.sub("(.)\{([^\}]*)\}", replaceTagMatch, alterationContents).rstrip() + '\n' + postfix).strip().encode('utf-8') + '\n'

###### PLUGIN #####

def getPluginConfig():
	try:
		return pickle.loads(str(getProfileSetting('plugin_config')))
	except:
		return []

def setPluginConfig(config):
	putProfileSetting('plugin_config', pickle.dumps(config))

def getPluginBasePaths():
	ret = []
	if platform.system() != "Windows":
		ret.append(os.path.expanduser('~/.cura/plugins/'))
	if platform.system() == "Darwin" and hasattr(sys, 'frozen'):
		ret.append(os.path.normpath(os.path.join(resources.resourceBasePath, "Cura/plugins")))
	else:
		ret.append(os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'plugins')))
	return ret

def getPluginList():
	ret = []
	for basePath in getPluginBasePaths():
		for filename in glob.glob(os.path.join(basePath, '*.py')):
			filename = os.path.basename(filename)
			if filename.startswith('_'):
				continue
			with open(os.path.join(basePath, filename), "r") as f:
				item = {'filename': filename, 'name': None, 'info': None, 'type': None, 'params': []}
				for line in f:
					line = line.strip()
					if not line.startswith('#'):
						break
					line = line[1:].split(':', 1)
					if len(line) != 2:
						continue
					if line[0].upper() == 'NAME':
						item['name'] = line[1].strip()
					elif line[0].upper() == 'INFO':
						item['info'] = line[1].strip()
					elif line[0].upper() == 'TYPE':
						item['type'] = line[1].strip()
					elif line[0].upper() == 'DEPEND':
						pass
					elif line[0].upper() == 'PARAM':
						m = re.match('([a-zA-Z][a-zA-Z0-9_]*)\(([a-zA-Z_]*)(?::([^\)]*))?\) +(.*)', line[1].strip())
						if m is not None:
							item['params'].append({'name': m.group(1), 'type': m.group(2), 'default': m.group(3), 'description': m.group(4)})
					else:
						print "Unknown item in effect meta data: %s %s" % (line[0], line[1])
				if item['name'] is not None and item['type'] == 'postprocess':
					ret.append(item)
	return ret

def runPostProcessingPlugins(gcodefilename):
	pluginConfigList = getPluginConfig()
	pluginList = getPluginList()

	for pluginConfig in pluginConfigList:
		plugin = None
		for pluginTest in pluginList:
			if pluginTest['filename'] == pluginConfig['filename']:
				plugin = pluginTest
		if plugin is None:
			continue

		pythonFile = None
		for basePath in getPluginBasePaths():
			testFilename = os.path.join(basePath, pluginConfig['filename'])
			if os.path.isfile(testFilename):
				pythonFile = testFilename
		if pythonFile is None:
			continue

		locals = {'filename': gcodefilename}
		for param in plugin['params']:
			value = param['default']
			if param['name'] in pluginConfig['params']:
				value = pluginConfig['params'][param['name']]

			if param['type'] == 'float':
				try:
					value = float(value)
				except:
					value = float(param['default'])

			locals[param['name']] = value
		try:
			execfile(pythonFile, locals)
		except:
			locationInfo = traceback.extract_tb(sys.exc_info()[2])[-1]
			return "%s: '%s' @ %s:%s:%d" % (str(sys.exc_info()[0].__name__), str(sys.exc_info()[1]), os.path.basename(locationInfo[0]), locationInfo[2], locationInfo[1])
	return None
*/