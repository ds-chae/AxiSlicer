#include "stdafx.h"
#include "mesh.h"
#include "profile.h"


cProfile profile;

//#The settings dictionary contains a key/value reference to all possible settings. With the setting name as key.

class cSettingsDictionary {
public:
	cSetting** data;
	int cap;
	int len;

	cSettingsDictionary()
	{
		cap = 0;
		len = 0;
		data = NULL;
	}
	~cSettingsDictionary()
	{
		free(data);
	}

	void clear()
	{
	}

	void append(cSetting* pSetting)
	{
		if(len >= cap) {
			cap += 100;
			cSetting** ndata = (cSetting**)malloc(cap*sizeof(cSetting*));
			for(int i = 0; i < len; i++)
				ndata[i] = data[i];
			if(data != NULL)
				free(data);
			data = ndata;
		}
		data[len++] = pSetting;
	}

	cSetting* find(char* __name)
	{
		for(int i = 0; i < len; i++) {
			cSetting* pSetting = data[i];
			if(strcmp(__name, pSetting->_name) == 0) {
				return pSetting;
			}
		}
		return NULL;
	}
};

cSettingsDictionary settingsDictionary;

cSettingsDictionary tempOverride;

/*
#The settings list is used to keep a full list of all the settings. This is needed to keep the settings in the proper order,
# as the dictionary will not contain insertion order.
settingsList = []

#Currently selected machine (by index) Cura support multiple machines in the same preferences and can switch between them.
# Each machine has it"s own index and unique name.
*/
int _selectedMachineIndex = 0;
/*
class setting(object):
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
#define	_CRT_SECURE_NO_WARNINGS	1

cSetting::cSetting(char* name, char* __default, int type, char* category, char* subcategory)
{
	_default = __default;
	_type = type;
	__init(name, category, subcategory);
}

cSetting::cSetting(char* name, char* __default, OptStr* opt, char* category, char* subcategory)
{
	_default = __default;
	_type = OptType;
	this->_coptions = opt->str;
	delete opt; // _str in opt is static string, deleting opt has no side effect.
	__init(name, category, subcategory);
}

void cSetting::__init(char* name, char* category, char* subcategory)
{
	_name = name;
	_label = name;
	_tooltip = "";
	_category = category;
	_subcategory = subcategory;
	/*
	if (type == FloatType)
		validators.validFloat(self);
	else if (type == IntType)
		validators.validInt(self);
	*/
//	global settingsDictionary
	settingsDictionary.append(this);
//	settingsList.append(this);
}

cSetting& cSetting::setLabel(char* label, char* tooltip)
{
	_label = label;
	_tooltip = tooltip;

	return *this;
}

cSetting& cSetting::setRange(float minValue, float maxValue)
{
//	if len(self._validators) < 1:
//		return
	_minValue = minValue;
	_maxValue = maxValue;

	return *this;
}
/*
	def getLabel(self):
		return _(self._label)

	def getTooltip(self):
		return _(self._tooltip)

	def getCategory(self):
		return self._category

	def getSubCategory(self):
		return self._subcategory
*/
int cSetting::isPreference()
{
	return strcmp(_category, "preference") == 0;
}

int cSetting::isMachineSetting()
{
	return strcmp(_category , "machine") == 0;
}

int cSetting::isAlteration()
{
	return strcmp(_category , "alteration") == 0;
}

int cSetting::isProfile()
{
	return !isAlteration() && !isPreference() && !isMachineSetting();
}

char* cSetting::getName()
{
	return _name;
}

int cSetting::getType()
{
	return _type;
}

char* cSetting::getValue(int index)
{
	if (index == -1)
		index = getValueIndex();
	if (index >= _values.size())
		return _default;

	return _values.data()[index].buf;
}

char* cSetting::getDefault()
{
	return _default;
}

void cSetting::setValue(char* value, int index)
{
/*
	if (index == -1)
		index = getValueIndex();
	while index >= len(self._values) {
			self._values.append(self._default)
		self._values[index] = unicode(value);
	}
*/
}

int cSetting::getValueIndex()
{
	if (isMachineSetting()) {
		return _selectedMachineIndex;
	}
	return 0;
}

cSetting::~cSetting()
{
}

/*
	def validate(self):
		result = validators.SUCCESS
		msgs = []
		for validator in self._validators:
			res, err = validator.validate()
			if res == validators.ERROR:
				result = res
			elif res == validators.WARNING and result != validators.ERROR:
				result = res
			if res != validators.SUCCESS:
				msgs.append(err)
		return result, "\n".join(msgs)

	def addCondition(self, conditionFunction):
		self._conditions.append(conditionFunction)

	def checkConditions(self):
		for condition in self._conditions:
			if not condition():
				return False
		return True

#########################################################
## Settings
#########################################################

#Define a fake _() function to fake the gettext tools in to generating strings for the profile settings.
def _(n):
	return n
*/
int profileInitialized = 0;

cSetting& setting(char* name, char* _default, int type, char* category, char* subcategory)
{
	cSetting *s = new cSetting(name, _default, type, category, subcategory);
	return *s;
}

cSetting& setting(char* name, char* _default, OptStr* opt, char* category, char* subcategory)
{
	cSetting *s = new cSetting(name, _default, opt, category, subcategory);
	return *s;
}

cProfile::cProfile()
{
}

void cProfile::__deinit()
{
	for(int n = 0; n < settingsDictionary.len; n++) {
		cSetting *pset = settingsDictionary.data[n];
		delete pset;
	}
}

void cProfile::__init()
{
	if(profileInitialized)
		return;

	profileInitialized = 1;

	setting("layer_height",              "0.1", FloatType, "basic",    ("Quality"))
		.setRange(0.0001).setLabel(("Layer height (mm)"),
			("Layer height in millimeters.\nThis is the most important setting to determine the quality of your print. Normal quality prints are 0.1mm, high quality is 0.06mm. You can go up to 0.25mm with an Ultimaker for very fast prints at low quality."));
	setting("wall_thickness",            "0.8", FloatType, "basic",    
		("Quality")).setRange(0.0).setLabel(("Shell thickness (mm)"),
		("Thickness of the outside shell in the horizontal direction.\nThis is used in combination with the nozzle size to define the number\nof perimeter lines and the thickness of those perimeter lines."));
	setting("retraction_enable",        "True", BoolType,  "basic",    
		("Quality")).setLabel(("Enable retraction"), ("Retract the filament when the nozzle is moving over a none-printed area. Details about the retraction can be configured in the advanced tab."));
	setting("solid_layer_thickness",     "0.6", FloatType, "basic",    
		("Fill")).setRange(0).setLabel(("Bottom/Top thickness (mm)"), 
		("This controls the thickness of the bottom and top layers, the amount of solid layers put down is calculated by the layer thickness and this value.\nHaving this value a multiple of the layer thickness makes sense. And keep it near your wall thickness to make an evenly strong part."));
	setting("fill_density",               "20", FloatType, "basic",("Fill")).setRange(0, 100).setLabel(("Fill Density (%)"),
		("This controls how densely filled the insides of your print will be. For a solid part use 100%, for an empty part use 0%. A value around 20% is usually enough.\nThis won't affect the outside of the print and only adjusts how strong the part becomes."));
	setting("nozzle_size",               "0.4", FloatType, "advanced",
		("Machine")).setRange(0.1,10).setLabel(("Nozzle size (mm)"), ("The nozzle size is very important, this is used to calculate the line width of the infill, and used to calculate the amount of outside wall lines and thickness for the wall thickness you entered in the print settings."));
	setting("print_speed",                "50", FloatType, "basic",    
		("Speed and Temperature")).setRange(1).setLabel(("Print speed (mm/s)"),
		("Speed at which printing happens. A well adjusted Ultimaker can reach 150mm/s, but for good quality prints you want to print slower. Printing speed depends on a lot of factors. So you will be experimenting with optimal settings for this."));
	setting("print_temperature",         "220", IntType,   "basic",    ("Speed and Temperature")).setRange(0,340)
		.setLabel(("Printing temperature (C)"), ("Temperature used for printing. Set at 0 to pre-heat yourself.\nFor PLA a value of 210C is usually used.\nFor ABS a value of 230C or higher is required."));
	setting("print_temperature2",          "0", IntType,   "basic",    ("Speed and Temperature")).setRange(0,340)
		.setLabel(("2nd nozzle temperature (C)"), ("Temperature used for printing. Set at 0 to pre-heat yourself.\nFor PLA a value of 210C is usually used.\nFor ABS a value of 230C or higher is required."));
	setting("print_temperature3",          "0", IntType,   "basic",    ("Speed and Temperature")).setRange(0,340)
		.setLabel(("3th nozzle temperature (C)"), ("Temperature used for printing. Set at 0 to pre-heat yourself.\nFor PLA a value of 210C is usually used.\nFor ABS a value of 230C or higher is required."));
	setting("print_temperature4",          "0", IntType,   "basic",    ("Speed and Temperature")).setRange(0,340)
		.setLabel(("4th nozzle temperature (C)"), ("Temperature used for printing. Set at 0 to pre-heat yourself.\nFor PLA a value of 210C is usually used.\nFor ABS a value of 230C or higher is required."));
	setting("print_bed_temperature",      "70", IntType,   "basic",    ("Speed and Temperature")).setRange(0,340)
		.setLabel(("Bed temperature (C)"), ("Temperature used for the heated printer bed. Set at 0 to pre-heat yourself."));
	OptStr* optstr = new OptStr("None\0Touching buildplate\0Everywhere\0");
	setting("support",                "None", optstr, "basic", ("Support"))
		.setLabel(("Support type"), ("Type of support structure build.\n\"Touching buildplate\" is the most commonly used support setting.\n\nNone does not do any support.\nTouching buildplate only creates support where the support structure will touch the build platform.\nEverywhere creates support even on top of parts of the model."));
	optstr = new OptStr("None\0Brim\0Raft");
	setting("platform_adhesion",      "None", optstr, "basic", ("Support"))
		.setLabel(("Platform adhesion type"), ("Different options that help in preventing corners from lifting due to warping.\nBrim adds a single layer thick flat area around your object which is easy to cut off afterwards, and the recommended option.\nRaft adds a thick raster at below the object and a thin interface between this and your object.\n(Note that enabling the brim or raft disables the skirt)"));
	optstr = new OptStr("Both\0First extruder\0Second extruder");
	setting("support_dual_extrusion",  "Both", optstr, "basic", ("Support"))
		.setLabel(("Support dual extrusion"), ("Which extruder to use for support material, for break-away support you can use both extruders.\nBut if one of the materials is more expensive then the other you could select an extruder to use for support material. This causes more extruder switches.\nYou can also use the 2nd extruder for soluble support materials."));
	setting("wipe_tower",              "False", BoolType,  "basic",    ("Dual extrusion"))
		.setLabel(("Wipe&prime tower"), ("The wipe-tower is a tower printed on every layer when switching between nozzles.\nThe old nozzle is wiped off on the tower before the new nozzle is used to print the 2nd color."));
	setting("wipe_tower_volume",          "15", FloatType, "expert",   ("Dual extrusion"))
		.setLabel(("Wipe&prime tower volume per layer (mm3)"),
		("The amount of material put in the wipe/prime tower.\nThis is done in volume because in general you want to extrude a\ncertain amount of volume to get the extruder going, independent on the layer height.\nThis means that with thinner layers, your tower gets bigger."));
	setting("ooze_shield",             "False", BoolType,  "basic",    ("Dual extrusion"))
		.setLabel(("Ooze shield"), ("The ooze shield is a 1 line thick shell around the object which stands a few mm from the object.\nThis shield catches any oozing from the unused nozzle in dual-extrusion."));
	setting("filament_diameter",        "2.85", FloatType, "basic",    ("Filament")).setRange(1)
		.setLabel(("Diameter (mm)"), ("Diameter of your filament, as accurately as possible.\nIf you cannot measure this value you will have to calibrate it, a higher number means less extrusion, a smaller number generates more extrusion."));
	setting("filament_diameter2",          "0", FloatType, "basic",    ("Filament")).setRange(0)
		.setLabel(("Diameter2 (mm)"), ("Diameter of your filament for the 2nd nozzle. Use 0 to use the same diameter as for nozzle 1."));
	setting("filament_diameter3",          "0", FloatType, "basic",    ("Filament")).setRange(0)
		.setLabel(("Diameter3 (mm)"), ("Diameter of your filament for the 3th nozzle. Use 0 to use the same diameter as for nozzle 1."));
	setting("filament_diameter4",          "0", FloatType, "basic",    ("Filament")).setRange(0)
		.setLabel(("Diameter4 (mm)"), ("Diameter of your filament for the 4th nozzle. Use 0 to use the same diameter as for nozzle 1."));
	setting("filament_flow",            "100.", FloatType, "basic",    ("Filament")).setRange(5,300)
		.setLabel(("Flow (%)"), ("Flow compensation, the amount of material extruded is multiplied by this value"));
	setting("retraction_speed",         "40.0", FloatType, "advanced", ("Retraction")).setRange(0.1)
		.setLabel(("Speed (mm/s)"), ("Speed at which the filament is retracted, a higher retraction speed works better. But a very high retraction speed can lead to filament grinding."));
	setting("retraction_amount",         "4.5", FloatType, "advanced", ("Retraction")).setRange(0)
		.setLabel(("Distance (mm)"), ("Amount of retraction, set at 0 for no retraction at all. A value of 4.5mm seems to generate good results."));
	setting("retraction_dual_amount",   "16.5", FloatType, "advanced", ("Retraction")).setRange(0)
		.setLabel(("Dual extrusion switch amount (mm)"), ("Amount of retraction when switching nozzle with dual-extrusion, set at 0 for no retraction at all. A value of 16.0mm seems to generate good results."));
	setting("retraction_min_travel",     "1.5", FloatType, "expert",   ("Retraction")).setRange(0)
		.setLabel(("Minimum travel (mm)"), ("Minimum amount of travel needed for a retraction to happen at all. To make sure you do not get a lot of retractions in a small area."));
	setting("retraction_combing",       "True", BoolType,  "expert",   ("Retraction")).setLabel(("Enable combing"), ("Combing is the act of avoiding holes in the print for the head to travel over. If combing is disabled the printer head moves straight from the start point to the end point and it will always retract."));
	setting("retraction_minimal_extrusion","0.02", FloatType,"expert",  ("Retraction")).setRange(0).setLabel(("Minimal extrusion before retracting (mm)"), ("The minimal amount of extrusion that needs to be done before retracting again if a retraction needs to happen before this minimal is reached the retraction is ignored.\nThis avoids retraction a lot on the same piece of filament which flattens the filament and causes grinding issues."));
	setting("bottom_thickness",          "0.3", FloatType, "advanced", ("Quality")).setRange(0).setLabel(("Initial layer thickness (mm)"), ("Layer thickness of the bottom layer. A thicker bottom layer makes sticking to the bed easier. Set to 0.0 to have the bottom layer thickness the same as the other layers."));
	setting("object_sink",               "0.0", FloatType, "advanced", ("Quality")).setLabel(("Cut off object bottom (mm)"), ("Sinks the object into the platform, this can be used for objects that do not have a flat bottom and thus create a too small first layer."));
//	#setting("enable_skin",             "False", BoolType,  "advanced", ("Quality")).setLabel(("Duplicate outlines"), ("Skin prints the outer lines of the prints twice, each time with half the thickness. This gives the illusion of a higher print quality."));
	setting("overlap_dual",             "0.15", FloatType, "advanced", ("Quality")).setLabel(("Dual extrusion overlap (mm)"), ("Add a certain amount of overlapping extrusion on dual-extrusion prints. This bonds the different colors better together."));
	setting("travel_speed",            "150.0", FloatType, "advanced", ("Speed")).setRange(0.1).setLabel(("Travel speed (mm/s)"), ("Speed at which travel moves are done, a high quality build Ultimaker can reach speeds of 250mm/s. But some machines might miss steps then."));
	setting("bottom_layer_speed",         "20", FloatType, "advanced", ("Speed")).setRange(0.1).setLabel(("Bottom layer speed (mm/s)"), ("Print speed for the bottom layer, you want to print the first layer slower so it sticks better to the printer bed."));
	setting("infill_speed",              "0.0", FloatType, "advanced", ("Speed")).setRange(0.0).setLabel(("Infill speed (mm/s)"), ("Speed at which infill parts are printed. If set to 0 then the print speed is used for the infill. Printing the infill faster can greatly reduce printing, but this can negatively effect print quality.."));
	setting("cool_min_layer_time",         "5", FloatType, "advanced", ("Cool")).setRange(0).setLabel(("Minimal layer time (sec)"), ("Minimum time spend in a layer, gives the layer time to cool down before the next layer is put on top. If the layer will be placed down too fast the printer will slow down to make sure it has spend at least this amount of seconds printing this layer."));
	setting("fan_enabled",              "True", BoolType,  "advanced", ("Cool")).setLabel(("Enable cooling fan"), ("Enable the cooling fan during the print. The extra cooling from the cooling fan is essential during faster prints."));

	setting("skirt_line_count",            "1", IntType,   "expert", "Skirt").setRange(0).setLabel(("Line count"), ("The skirt is a line drawn around the object at the first layer. This helps to prime your extruder, and to see if the object fits on your platform.\nSetting this to 0 will disable the skirt. Multiple skirt lines can help priming your extruder better for small objects."));
	setting("skirt_gap",                 "3.0", FloatType, "expert", "Skirt").setRange(0).setLabel(("Start distance (mm)"), ("The distance between the skirt and the first layer.\nThis is the minimal distance, multiple skirt lines will be put outwards from this distance."));
	setting("skirt_minimal_length",    "150.0", FloatType, "expert", "Skirt").setRange(0).setLabel(("Minimal length (mm)"), ("The minimal length of the skirt, if this minimal length is not reached it will add more skirt lines to reach this minimal lenght.\nNote: If the line count is set to 0 this is ignored."));
//#setting("max_z_speed",               3.0, FloatType, "expert",   ("Speed")).setRange(0.1).setLabel(("Max Z speed (mm/s)"), ("Speed at which Z moves are done. When you Z axis is properly lubricated you can increase this for less Z blob."));
//#setting("retract_on_jumps_only",    True, IntType,  "expert",   ("Retraction")).setLabel(("Retract on jumps only"), ("Only retract when we are making a move that is over a hole in the model, else retract on every move. This effects print quality in different ways."));
	setting("fan_full_height",           "0.5", FloatType, "expert",   ("Cool")).setRange(0).setLabel(("Fan full on at height (mm)"), ("The height at which the fan is turned on completely. For the layers below this the fan speed is scaled linear with the fan off at layer 0."));
	setting("fan_speed",                 "100", IntType,   "expert",   ("Cool")).setRange(0,100).setLabel(("Fan speed min (%)"), ("When the fan is turned on, it is enabled at this speed setting. If cool slows down the layer, the fan is adjusted between the min and max speed. Minimal fan speed is used if the layer is not slowed down due to cooling."));
	setting("fan_speed_max",             "100", IntType,   "expert",   ("Cool")).setRange(0,100).setLabel(("Fan speed max (%)"), ("When the fan is turned on, it is enabled at this speed setting. If cool slows down the layer, the fan is adjusted between the min and max speed. Maximal fan speed is used if the layer is slowed down due to cooling by more than 200%."));
	setting("cool_min_feedrate",          "10", FloatType, "expert",   ("Cool")).setRange(0).setLabel(("Minimum speed (mm/s)"), ("The minimal layer time can cause the print to slow down so much it starts to ooze. The minimal feedrate protects against this. Even if a print gets slown down it will never be slower than this minimal speed."));
	setting("cool_head_lift",          "False", BoolType,  "expert",   ("Cool")).setLabel(("Cool head lift"), ("Lift the head if the minimal speed is hit because of cool slowdown, and wait the extra time so the minimal layer time is always hit."));
//#setting("extra_base_wall_thickness", 0.0, FloatType, "expert",   "Accuracy").setRange(0).setLabel(("Extra Wall thickness for bottom/top (mm)"), ("Additional wall thickness of the bottom and top layers."));
//#setting("sequence", "Loops > Perimeter > Infill", ["Loops > Perimeter > Infill", "Loops > Infill > Perimeter", "Infill > Loops > Perimeter", "Infill > Perimeter > Loops", "Perimeter > Infill > Loops", "Perimeter > Loops > Infill"], "expert", "Sequence");
//#setting("force_first_layer_sequence", True, IntType, "expert", "Sequence").setLabel(("Force first layer sequence"), ("This setting forces the order of the first layer to be \"Perimeter > Loops > Infill\""));
//#setting("infill_type", "Line", ["Line", "Grid Circular", "Grid Hexagonal", "Grid Rectangular"], "expert", ("Infill")).setLabel(("Infill pattern"), ("Pattern of the none-solid infill. Line is default, but grids can provide a strong print."));
	setting("solid_top", "True", BoolType, "expert", ("Infill")).setLabel(("Solid infill top"), ("Create a solid top surface, if set to false the top is filled with the fill percentage. Useful for cups/vases."));
	setting("solid_bottom", "True", BoolType, "expert", ("Infill")).setLabel(("Solid infill bottom"), ("Create a solid bottom surface, if set to false the bottom is filled with the fill percentage. Useful for buildings."));
	setting("fill_overlap", "15", IntType, "expert", ("Infill")).setRange(0,100).setLabel(("Infill overlap (%)"), ("Amount of overlap between the infill and the walls. There is a slight overlap with the walls and the infill so the walls connect firmly to the infill."));
	setting("support_fill_rate", "15", IntType, "expert", ("Support")).setRange(0,100).setLabel(("Fill amount (%)"), ("Amount of infill structure in the support material, less material gives weaker support which is easier to remove. 15% seems to be a good average."));
	setting("support_xy_distance", "0.7", FloatType, "expert", ("Support")).setRange(0,10).setLabel(("Distance X/Y (mm)"), ("Distance of the support material from the print, in the X/Y directions.\n0.7mm gives a nice distance from the print so the support does not stick to the print."));
	setting("support_z_distance", "0.15", FloatType, "expert", ("Support")).setRange(0,10).setLabel(("Distance Z (mm)"), ("Distance from the top/bottom of the support to the print. A small gap here makes it easier to remove the support but makes the print a bit uglier.\n0.15mm gives a good seperation of the support material."));
	setting("spiralize", "False", BoolType, "expert", "Spiralize").setLabel(("Spiralize the outer contour"), ("Spiralize is smoothing out the Z move of the outer edge. This will create a steady Z increase over the whole print. This feature turns a solid object into a single walled print with a solid bottom."));
//#setting("bridge_speed", 100, IntType, "expert", "Bridge").setRange(0,100).setLabel(("Bridge speed (%)"), ("Speed at which layers with bridges are printed, compared to normal printing speed."));
	setting("brim_line_count", "20", IntType, "expert", ("Brim")).setRange(1,100).setLabel(("Brim line amount"), ("The amount of lines used for a brim, more lines means a larger brim which sticks better, but this also makes your effective print area smaller."));
	setting("raft_margin", "5", FloatType, "expert", ("Raft")).setRange(0).setLabel(("Extra margin (mm)"), ("If the raft is enabled, this is the extra raft area around the object which is also rafted. Increasing this margin will create a stronger raft while using more material and leaving less are for your print."));
	setting("raft_line_spacing", "1.0", FloatType, "expert", ("Raft")).setRange(0).setLabel(("Line spacing (mm)"), ("When you are using the raft this is the distance between the centerlines of the raft line."));
	setting("raft_base_thickness", "0.3", FloatType, "expert", ("Raft")).setRange(0).setLabel(("Base thickness (mm)"), ("When you are using the raft this is the thickness of the base layer which is put down."));
	setting("raft_base_linewidth", "0.7", FloatType, "expert", ("Raft")).setRange(0).setLabel(("Base line width (mm)"), ("When you are using the raft this is the width of the base layer lines which are put down."));
	setting("raft_interface_thickness", "0.2", FloatType, "expert", ("Raft")).setRange(0).setLabel(("Interface thickness (mm)"), ("When you are using the raft this is the thickness of the interface layer which is put down."));
	setting("raft_interface_linewidth", "0.2", FloatType, "expert", ("Raft")).setRange(0).setLabel(("Interface line width (mm)"), ("When you are using the raft this is the width of the interface layer lines which are put down."));
//#setting("hop_on_move", "False", BoolType, "expert", "Hop").setLabel(("Enable hop on move"), ("When moving from print position to print position, raise the printer head 0.2mm so it does not knock off the print (experimental)."));
	setting("fix_horrible_union_all_type_a", "True",  BoolType, "expert", ("Fix horrible")).setLabel(("Combine everything (Type-A)"), ("This expert option adds all parts of the model together. The result is usually that internal cavities disappear. Depending on the model this can be intended or not. Enabling this option is at your own risk. Type-A is depended on the model normals and tries to keep some internal holes intact. Type-B ignores all internal holes and only keeps the outside shape per layer."));
	setting("fix_horrible_union_all_type_b", "False", BoolType, "expert", ("Fix horrible")).setLabel(("Combine everything (Type-B)"), ("This expert option adds all parts of the model together. The result is usually that internal cavities disappear. Depending on the model this can be intended or not. Enabling this option is at your own risk. Type-A is depended on the model normals and tries to keep some internal holes intact. Type-B ignores all internal holes and only keeps the outside shape per layer."));
	setting("fix_horrible_use_open_bits", "False", BoolType, "expert", ("Fix horrible")).setLabel(("Keep open faces"), ("This expert option keeps all the open bits of the model intact. Normally Cura tries to stitch up small holes and remove everything with big holes, but this option keeps bits that are not properly part of anything and just goes with whatever it is left. This option is usually not what you want, but it might enable you to slice models otherwise failing to produce proper paths.\nAs with all \"Fix horrible\" options, results may vary and use at your own risk."));
	setting("fix_horrible_extensive_stitching", "False", BoolType, "expert", ("Fix horrible")).setLabel(("Extensive stitching"), ("Extrensive stitching tries to fix up open holes in the model by closing the hole with touching polygons. This algorthm is quite expensive and could introduce a lot of processing time.\nAs with all \"Fix horrible\" options, results may vary and use at your own risk."));

	setting("plugin_config", "", StrType, "hidden", "hidden");
	setting("object_center_x", "-1", FloatType, "hidden", "hidden");
	setting("object_center_y", "-1", FloatType, "hidden", "hidden");

char* start_gcode =
	";Sliced at: {day} {date} {time};\n"
	";Basic settings: Layer height: {layer_height} Walls: {wall_thickness} Fill: {fill_density}\n"
	";Print time: {print_time}\n"
	";Filament used: {filament_amount}m {filament_weight}g\n"
	";Filament cost: {filament_cost}\n"
	";M190 S{print_bed_temperature} ;Uncomment to add your own bed temperature line\n"
	";M109 S{print_temperature} ;Uncomment to add your own temperature line\n"
	"G21        ;metric values\n"
	"G90        ;absolute positioning\n"
	"M107       ;start with the fan off\n"
	"\n"
	"G28 X0 Y0  ;move X/Y to min endstops\n"
	"G28 Z0     ;move Z to min endstops\n"
	"\n"
	"G1 Z15.0 F{travel_speed} ;move the platform down 15mm\n"
	"\n"
	"G92 E0                  ;zero the extruded length\n"
	"G1 F200 E3              ;extrude 3mm of feed stock\n"
	"G92 E0                  ;zero the extruded length again\n"
	"G1 F{travel_speed}\n"
	";Put printing message on LCD screen\n"
	"M117 Printing...\n";

setting("start.gcode", start_gcode, StrType, "alteration", "alteration");

//#######################################################################################
char* end_gcode =
	";End GCode\n"
	"M104 S0                     ;extruder heater off\n"
	"M140 S0                     ;heated bed heater off (if you have it)\n"
	"\n"
	"G91                                    ;relative positioning\n"
	"G1 E-1 F300                            ;retract the filament a bit before lifting the nozzle, to release some of the pressure\n"
	"G1 Z+0.5 E-5 X-20 Y-20 F{travel_speed} ;move Z up a bit and retract filament even more\n"
	"G28 X0 Y0                              ;move X/Y to min endstops, so the head is out of the way\n"
	"\n"
	"M84                         ;steppers off\n"
	"G90                         ;absolute positioning\n";

setting("end.gcode", end_gcode, StrType, "alteration", "alteration");

//#######################################################################################
char* start2_gcode = 
	";Sliced at: {day} {date} {time}\n"
	";Basic settings: Layer height: {layer_height} Walls: {wall_thickness} Fill: {fill_density}\n"
	";Print time: {print_time}\n"
	";Filament used: {filament_amount}m {filament_weight}g\n"
	";Filament cost: {filament_cost}\n"
	";M190 S{print_bed_temperature} ;Uncomment to add your own bed temperature line\n"
	";M104 S{print_temperature} ;Uncomment to add your own temperature line\n"
	";M109 T1 S{print_temperature2} ;Uncomment to add your own temperature line\n"
	";M109 T0 S{print_temperature} ;Uncomment to add your own temperature line\n"
	"G21        ;metric values\n"
	"G90        ;absolute positioning\n"
	"M107       ;start with the fan off\n"
	"\n"
	"G28 X0 Y0  ;move X/Y to min endstops\n"
	"G28 Z0     ;move Z to min endstops\n"
	"\n"
	"G1 Z15.0 F{travel_speed} ;move the platform down 15mm\n"
	"\n"
	"T1                      ;Switch to the 2nd extruder\n"
	"G92 E0                  ;zero the extruded length\n"
	"G1 F200 E10             ;extrude 10mm of feed stock\n"
	"G92 E0                  ;zero the extruded length again\n"
	"G1 F200 E-{retraction_dual_amount}\n"
	"\n"
	"T0                      ;Switch to the first extruder\n"
	"G92 E0                  ;zero the extruded length\n"
	"G1 F200 E10             ;extrude 10mm of feed stock\n"
	"G92 E0                  ;zero the extruded length again\n"
	"G1 F{travel_speed}\n"
	";Put printing message on LCD screen\n"
	"M117 Printing...\n";

	setting("start2.gcode", start2_gcode, StrType, "alteration", "alteration");

//#######################################################################################
char* end2_gcode =
	";End GCode\n"
	"M104 T0 S0                     ;extruder heater off\n"
	"M104 T1 S0                     ;extruder heater off\n"
	"M140 S0                     ;heated bed heater off (if you have it)\n"
	"\n"
	"G91                                    ;relative positioning\n"
	"G1 E-1 F300                            ;retract the filament a bit before lifting the nozzle, to release some of the pressure\n"
	"G1 Z+0.5 E-5 X-20 Y-20 F{travel_speed} ;move Z up a bit and retract filament even more\n"
	"G28 X0 Y0                              ;move X/Y to min endstops, so the head is out of the way\n"
	"\n"
	"M84                         ;steppers off\n"
	"G90                         ;absolute positioning\n";

	setting("end2.gcode", end2_gcode, StrType, "alteration", "alteration");

//#######################################################################################
setting("support_start.gcode", "", StrType, "alteration", "alteration");
setting("support_end.gcode", "", StrType, "alteration", "alteration");
setting("cool_start.gcode", "", StrType, "alteration", "alteration");
setting("cool_end.gcode", "", StrType, "alteration", "alteration");
setting("replace.csv", "", StrType, "alteration", "alteration");

//#######################################################################################
char* switche_gcode = 
	";Switch between the current extruder and the next extruder, when printing with multiple extruders.\n"
	"G92 E0\n"
	"G1 E-36 F5000\n"
	"G92 E0\n"
	"T{extruder}\n"
	"G1 X{new_x} Y{new_y} Z{new_z} F{travel_speed}\n"
	"G1 E36 F5000\n"
	"G92 E0\n";

	setting("switchExtruder.gcode", switche_gcode, StrType, "alteration", "alteration");

	optstr = new OptStr("Simple\0Normal\0");
	setting("startMode", "Simple", optstr, "preference", "hidden");
	setting("oneAtATime", "True", BoolType, "preference", "hidden");
//setting("lastFile", os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "resources", "example", "UltimakerRobot_support.stl")), str, "preference", "hidden");
	setting("lastFile", "", StrType, "preference", "hidden");
	setting("save_profile", "False", BoolType, "preference", "hidden").setLabel(("Save profile on slice"), ("When slicing save the profile as [stl_file]_profile.ini next to the model."));
	setting("filament_cost_kg", "0", FloatType, "preference", "hidden").setLabel(("Cost (price/kg)"), ("Cost of your filament per kg, to estimate the cost of the final print."));
	setting("filament_cost_meter", "0", FloatType, "preference", "hidden").setLabel(("Cost (price/m)"), ("Cost of your filament per meter, to estimate the cost of the final print."));
	setting("auto_detect_sd", "True", BoolType, "preference", "hidden").setLabel(("Auto detect SD card drive"), ("Auto detect the SD card. You can disable this because on some systems external hard-drives or USB sticks are detected as SD card."));
	setting("check_for_updates", "True", BoolType, "preference", "hidden").setLabel(("Check for updates"), ("Check for newer versions of Cura on startup"));
	setting("submit_slice_information", "False", BoolType, "preference", "hidden").setLabel(("Send usage statistics"), ("Submit anonymous usage information to improve next versions of Cura"));
	setting("youmagine_token", "", StrType, "preference", "hidden");
	setting("filament_physical_density", "1240", FloatType, "preference", "hidden").setRange(500.0, 3000.0).setLabel(("Density (kg/m3)"), ("Weight of the filament per m3. Around 1240 for PLA. And around 1040 for ABS. This value is used to estimate the weight if the filament used for the print."));
	setting("language", "English", StrType, "preference", "hidden").setLabel(("Language"), ("Change the language in which Cura runs. Switching language requires a restart of Cura"));
	setting("active_machine", "0", IntType, "preference", "hidden");

	setting("model_colour", "#FFC924", StrType, "preference", "hidden").setLabel(("Model colour"));
	setting("model_colour2", "#CB3030", StrType, "preference", "hidden").setLabel(("Model colour (2)"));
	setting("model_colour3", "#DDD93C", StrType, "preference", "hidden").setLabel(("Model colour (3)"));
	setting("model_colour4", "#4550D3", StrType, "preference", "hidden").setLabel(("Model colour (4)"));

	setting("window_maximized", "True", BoolType, "preference", "hidden");
	setting("window_pos_x", "-1", FloatType, "preference", "hidden");
	setting("window_pos_y", "-1", FloatType, "preference", "hidden");
	setting("window_width", "-1", FloatType, "preference", "hidden");
	setting("window_height", "-1", FloatType, "preference", "hidden");
	setting("window_normal_sash", "320", FloatType, "preference", "hidden");
	setting("last_run_version", "", StrType, "preference", "hidden");

	setting("machine_name", "", StrType, "machine", "hidden");
	setting("machine_type", "unknown", StrType, "machine", "hidden"); // #Ultimaker, Ultimaker2, RepRap
	setting("machine_width", "205", FloatType, "machine", "hidden").setLabel(("Maximum width (mm)"), ("Size of the machine in mm"));
	setting("machine_depth", "205", FloatType, "machine", "hidden").setLabel(("Maximum depth (mm)"), ("Size of the machine in mm"));
	setting("machine_height", "200", FloatType, "machine", "hidden").setLabel(("Maximum height (mm)"), ("Size of the machine in mm"));
	setting("machine_center_is_zero", "False", BoolType, "machine", "hidden").setLabel(("Machine center 0,0"), ("Machines firmware defines the center of the bed as 0,0 instead of the front left corner."));
	setting("ultimaker_extruder_upgrade", "False", BoolType, "machine", "hidden");
	setting("has_heated_bed", "False", BoolType, "machine", "hidden").setLabel(("Heated bed"), ("If you have an heated bed, this enabled heated bed settings (requires restart)"));
	optstr = new OptStr("RepRap (Marlin/Sprinter)\0UltiGCode\0MakerBot\0");
	setting("gcode_flavor", "RepRap (Marlin/Sprinter)", optstr, "machine", "hidden")
		.setLabel(("GCode Flavor"), ("Flavor of generated GCode.\nRepRap is normal 5D GCode which works on Marlin/Sprinter based firmwares.\nUltiGCode is a variation of the RepRap GCode which puts more settings in the machine instead of the slicer.\nMakerBot GCode has a few changes in the way GCode is generated, but still requires MakerWare to generate to X3G."));
	optstr = new OptStr("1\02\03\04\0");
	setting("extruder_amount", "1", optstr, "machine", "hidden").setLabel(("Extruder count"), ("Amount of extruders in your machine."));
	setting("extruder_offset_x1", "0.0", FloatType, "machine", "hidden").setLabel(("Offset X"), ("The offset of your secondary extruder compared to the primary."));
	setting("extruder_offset_y1", "-21.6", FloatType, "machine", "hidden").setLabel(("Offset Y"), ("The offset of your secondary extruder compared to the primary."));
	setting("extruder_offset_x2", "0.0", FloatType, "machine", "hidden").setLabel(("Offset X"), ("The offset of your tertiary extruder compared to the primary."));
	setting("extruder_offset_y2", "0.0", FloatType, "machine", "hidden").setLabel(("Offset Y"), ("The offset of your tertiary extruder compared to the primary."));
	setting("extruder_offset_x3", "0.0", FloatType, "machine", "hidden").setLabel(("Offset X"), ("The offset of your forth extruder compared to the primary."));
	setting("extruder_offset_y3", "0.0", FloatType, "machine", "hidden").setLabel(("Offset Y"), ("The offset of your forth extruder compared to the primary."));
	setting("steps_per_e", "0", FloatType, "machine", "hidden").setLabel(("E-Steps per 1mm filament"), ("Amount of steps per mm filament extrusion. If set to 0 then this value is ignored and the value in your firmware is used."));
	setting("serial_port", "AUTO", StrType, "machine", "hidden").setLabel(("Serial port"), ("Serial port to use for communication with the printer"));
	setting("serial_port_auto", "", StrType, "machine", "hidden");
	setting("serial_baud", "AUTO", StrType, "machine", "hidden").setLabel(("Baudrate"), ("Speed of the serial port communication\nNeeds to match your firmware settings\nCommon values are 250000, 115200, 57600"));
	setting("serial_baud_auto", "", IntType, "machine", "hidden");

	setting("extruder_head_size_min_x", "0.0", FloatType, "machine", "hidden").setLabel(("Head size towards X min (mm)"), ("The head size when printing multiple objects, measured from the tip of the nozzle towards the outer part of the head. 75mm for an Ultimaker if the fan is on the left side."));
	setting("extruder_head_size_min_y", "0.0", FloatType, "machine", "hidden").setLabel(("Head size towards Y min (mm)"), ("The head size when printing multiple objects, measured from the tip of the nozzle towards the outer part of the head. 18mm for an Ultimaker if the fan is on the left side."));
	setting("extruder_head_size_max_x", "0.0", FloatType, "machine", "hidden").setLabel(("Head size towards X max (mm)"), ("The head size when printing multiple objects, measured from the tip of the nozzle towards the outer part of the head. 18mm for an Ultimaker if the fan is on the left side."));
	setting("extruder_head_size_max_y", "0.0", FloatType, "machine", "hidden").setLabel(("Head size towards Y max (mm)"), ("The head size when printing multiple objects, measured from the tip of the nozzle towards the outer part of the head. 35mm for an Ultimaker if the fan is on the left side."));
	setting("extruder_head_size_height", "0.0", FloatType, "machine", "hidden").setLabel(("Printer gantry height (mm)"), ("The height of the gantry holding up the printer head. If an object is higher then this then you cannot print multiple objects one for one. 60mm for an Ultimaker."));

/*
validators.warningAbove(settingsDictionary["filament_flow"], 150, ("More flow than 150% is rare and usually not recommended."));
validators.warningBelow(settingsDictionary["filament_flow"], 50, ("Less flow than 50% is rare and usually not recommended."));
validators.warningAbove(settingsDictionary["layer_height"], lambda : (float(getProfileSetting("nozzle_size")) * 80.0 / 100.0), ("Thicker layers then %.2fmm (80%% nozzle size) usually give bad results and are not recommended."));
validators.wallThicknessValidator(settingsDictionary["wall_thickness"]);
validators.warningAbove(settingsDictionary["print_speed"], 150.0, ("It is highly unlikely that your machine can achieve a printing speed above 150mm/s"));
validators.printSpeedValidator(settingsDictionary["print_speed"]);
validators.warningAbove(settingsDictionary["print_temperature"], 260.0, ("Temperatures above 260C could damage your machine, be careful!"));
validators.warningAbove(settingsDictionary["print_temperature2"], 260.0, ("Temperatures above 260C could damage your machine, be careful!"));
validators.warningAbove(settingsDictionary["print_temperature3"], 260.0, ("Temperatures above 260C could damage your machine, be careful!"));
validators.warningAbove(settingsDictionary["print_temperature4"], 260.0, ("Temperatures above 260C could damage your machine, be careful!"));
validators.warningAbove(settingsDictionary["filament_diameter"], 3.5, ("Are you sure your filament is that thick? Normal filament is around 3mm or 1.75mm."));
validators.warningAbove(settingsDictionary["filament_diameter2"], 3.5, ("Are you sure your filament is that thick? Normal filament is around 3mm or 1.75mm."));
validators.warningAbove(settingsDictionary["filament_diameter3"], 3.5, ("Are you sure your filament is that thick? Normal filament is around 3mm or 1.75mm."));
validators.warningAbove(settingsDictionary["filament_diameter4"], 3.5, ("Are you sure your filament is that thick? Normal filament is around 3mm or 1.75mm."));
validators.warningAbove(settingsDictionary["travel_speed"], 300.0, ("It is highly unlikely that your machine can achieve a travel speed above 300mm/s"));
validators.warningAbove(settingsDictionary["bottom_thickness"], lambda : (float(getProfileSetting("nozzle_size")) * 3.0 / 4.0), ("A bottom layer of more then %.2fmm (3/4 nozzle size) usually give bad results and is not recommended."));
*/

//#Conditions for multiple extruders
/*
settingsDictionary["print_temperature2"].addCondition(lambda : int(getMachineSetting("extruder_amount")) > 1);
settingsDictionary["print_temperature3"].addCondition(lambda : int(getMachineSetting("extruder_amount")) > 2);
settingsDictionary["print_temperature4"].addCondition(lambda : int(getMachineSetting("extruder_amount")) > 3);
settingsDictionary["filament_diameter2"].addCondition(lambda : int(getMachineSetting("extruder_amount")) > 1);
settingsDictionary["filament_diameter3"].addCondition(lambda : int(getMachineSetting("extruder_amount")) > 2);
settingsDictionary["filament_diameter4"].addCondition(lambda : int(getMachineSetting("extruder_amount")) > 3);
settingsDictionary["support_dual_extrusion"].addCondition(lambda : int(getMachineSetting("extruder_amount")) > 1);
settingsDictionary["retraction_dual_amount"].addCondition(lambda : int(getMachineSetting("extruder_amount")) > 1);
settingsDictionary["wipe_tower"].addCondition(lambda : int(getMachineSetting("extruder_amount")) > 1);
settingsDictionary["wipe_tower_volume"].addCondition(lambda : int(getMachineSetting("extruder_amount")) > 1);
settingsDictionary["ooze_shield"].addCondition(lambda : int(getMachineSetting("extruder_amount")) > 1);
//#Heated bed
settingsDictionary["print_bed_temperature"].addCondition(lambda : getMachineSetting("has_heated_bed") == "True");

//#UltiGCode uses less settings, as these settings are located inside the machine instead of gcode.
settingsDictionary["print_temperature"].addCondition(lambda : getMachineSetting("gcode_flavor") != "UltiGCode");
settingsDictionary["print_temperature2"].addCondition(lambda : getMachineSetting("gcode_flavor") != "UltiGCode");
settingsDictionary["print_temperature3"].addCondition(lambda : getMachineSetting("gcode_flavor") != "UltiGCode");
settingsDictionary["print_temperature4"].addCondition(lambda : getMachineSetting("gcode_flavor") != "UltiGCode");
settingsDictionary["filament_diameter"].addCondition(lambda : getMachineSetting("gcode_flavor") != "UltiGCode");
settingsDictionary["filament_diameter2"].addCondition(lambda : getMachineSetting("gcode_flavor") != "UltiGCode");
settingsDictionary["filament_diameter3"].addCondition(lambda : getMachineSetting("gcode_flavor") != "UltiGCode");
settingsDictionary["filament_diameter4"].addCondition(lambda : getMachineSetting("gcode_flavor") != "UltiGCode");
settingsDictionary["filament_flow"].addCondition(lambda : getMachineSetting("gcode_flavor") != "UltiGCode");
settingsDictionary["print_bed_temperature"].addCondition(lambda : getMachineSetting("gcode_flavor") != "UltiGCode");
settingsDictionary["retraction_speed"].addCondition(lambda : getMachineSetting("gcode_flavor") != "UltiGCode");
settingsDictionary["retraction_amount"].addCondition(lambda : getMachineSetting("gcode_flavor") != "UltiGCode");
settingsDictionary["retraction_dual_amount"].addCondition(lambda : getMachineSetting("gcode_flavor") != "UltiGCode");
*/
}

//#Remove fake defined _() because later the localization will define a global _()
//del _
/*
#########################################################
## Profile and preferences functions
#########################################################

def getSubCategoriesFor(category):
	done = {}
	ret = []
	for s in settingsList:
		if s.getCategory() == category and not s.getSubCategory() in done and s.checkConditions():
			done[s.getSubCategory()] = True
			ret.append(s.getSubCategory())
	return ret

def getSettingsForCategory(category, subCategory = None):
	ret = []
	for s in settingsList:
		if s.getCategory() == category and (subCategory is None or s.getSubCategory() == subCategory) and s.checkConditions():
			ret.append(s)
	return ret
*/
//## Profile functions
char _basePath[MAX_PATH];

char* cProfile::getBasePath()
{
	return _basePath;
}

char* getAlternativeBasePaths()
{
	return _basePath;
}

char profilePath[MAX_PATH];

char* cProfile::getDefaultProfilePath()
{
	sprintf_s(profilePath, MAX_PATH, "%s\\current_profile.ini", getBasePath());
	return profilePath;
}

void cProfile::loadProfile(LPCTSTR filename)
{
/*
//	#Read a configuration file as global config
	profileParser = ConfigParser.ConfigParser()
	try:
		profileParser.read(filename)
	except ConfigParser.ParsingError:
		return
	global settingsList
	for set in settingsList:
		if set.isPreference():
			continue
		section = "profile"
		if set.isAlteration():
			section = "alterations"
		if profileParser.has_option(section, set.getName()):
			set.setValue(unicode(profileParser.get(section, set.getName()), "utf-8", "replace"))
*/
}

void saveProfile(char* filename)
{
/*
//	#Save the current profile to an ini file
	profileParser = ConfigParser.ConfigParser()
	profileParser.add_section("profile")
	profileParser.add_section("alterations")
	global settingsList
	for set in settingsList:
		if set.isPreference() or set.isMachineSetting():
			continue
		if set.isAlteration():
			profileParser.set("alterations", set.getName(), set.getValue().encode("utf-8"))
		else:
			profileParser.set("profile", set.getName(), set.getValue().encode("utf-8"))

	profileParser.write(open(filename, "w"))
*/
}


void resetProfile()
{
/*
//	#Read a configuration file as global config
	for set in settingsList:
		if not set.isProfile():
			continue
		set.setValue(set.getDefault())

	if getMachineSetting("machine_type") == "ultimaker":
		putProfileSetting("nozzle_size", "0.4")
		if getMachineSetting("ultimaker_extruder_upgrade") == "True":
			putProfileSetting("retraction_enable", "True")
	elif getMachineSetting("machine_type") == "ultimaker2":
		putProfileSetting("nozzle_size", "0.4")
		putProfileSetting("retraction_enable", "True")
	else:
		putProfileSetting("nozzle_size", "0.5")
		putProfileSetting("retraction_enable", "True")
*/
}

void setProfileFromString(char* options)
{
/*
options = base64.b64decode(options)
	options = zlib.decompress(options)
	(profileOpts, alt) = options.split("\f", 1)
	global settingsDictionary
	for option in profileOpts.split("\b"):
		if len(option) > 0:
			(key, value) = option.split("=", 1)
			if key in settingsDictionary:
				if settingsDictionary[key].isProfile():
					settingsDictionary[key].setValue(value)
	for option in alt.split("\b"):
		if len(option) > 0:
			(key, value) = option.split("=", 1)
			if key in settingsDictionary:
				if settingsDictionary[key].isAlteration():
					settingsDictionary[key].setValue(value)
*/
}

char* getProfileString()
{
/*
p = []
	alt = []
	global settingsList
	for set in settingsList:
		if set.isProfile():
			if set.getName() in tempOverride:
				p.append(set.getName() + "=" + tempOverride[set.getName()])
			else:
				p.append(set.getName() + "=" + set.getValue())
		elif set.isAlteration():
			if set.getName() in tempOverride:
				alt.append(set.getName() + "=" + tempOverride[set.getName()])
			else:
				alt.append(set.getName() + "=" + set.getValue())
	ret = "\b".join(p) + "\f" + "\b".join(alt)
	ret = base64.b64encode(zlib.compress(ret, 9))
	return ret
*/
	return "";
}

char* insertNewlines(char* string, int every)
{
/*
//	: #This should be moved to a better place then profile.
	lines = []
	for i in xrange(0, len(string), every):
		lines.append(string[i:i+every])
	return "\n".join(lines)
*/
	return "";
}

char* cProfile::getPreferencesString()
{
/*
p = []
	global settingsList
	for set in settingsList:
		if set.isPreference():
			p.append(set.getName() + "=" + set.getValue())
	ret = "\b".join(p)
	ret = base64.b64encode(zlib.compress(ret, 9))
	return ret
*/
	return "";
}

char* cProfile::getProfileSetting(char* name)
{
/*
if name in tempOverride:
		return tempOverride[name]
	global settingsDictionary
	if name in settingsDictionary and settingsDictionary[name].isProfile():
		return settingsDictionary[name].getValue()
	traceback.print_stack()
	sys.stderr.write("Error: "%s" not found in profile settings\n" % (name))
	return ""
*/
	return "";
}

float cProfile::getProfileSettingFloat(char* name)
{
/*
try:
		setting = getProfileSetting(name).replace(",", ".")
		return float(eval(setting, {}, {}))
	except:
		return 0.0
*/
	return 0.0;
}

void cProfile::putProfileSetting(char* name, char* value)
{
/*
#Check if we have a configuration file loaded, else load the default.
	global settingsDictionary
	if name in settingsDictionary and settingsDictionary[name].isProfile():
		settingsDictionary[name].setValue(value)
*/
}

int cProfile::isProfileSetting(char* name)
{
	cSetting *setting =settingsDictionary.find(name);
	if(setting && setting->isProfile())
		return True;
	return False;
}

//## Preferences functions

char prefPath[MAX_PATH];
char* cProfile::getPreferencePath()
{
	sprintf_s(prefPath, "%s\\preferences.ini", getBasePath());
	return prefPath;
}


float cProfile::getPreferenceFloat(char* name)
{
	char* value = getPreference(name);
	return atof(value);
}

int hexint(char* s, int b, int e)
{
	int iv = 0;
	while(b < e) {
		char c = s[b];
		if(c >= '0' && c <= '9')
			iv = iv * 16 + (c - '0');
		else if(c >= 'a' && c <= 'f')
			iv = iv * 16 + (c - 'a' + 10);
		else if(c >= 'A' && c <= 'F')
			iv = iv * 16 + (c - 'A' + 10);
		b++;
	}
	return iv;
}

void cProfile::getPreferenceColour(char* name, float* color)
{
	char* colorString = profile.getPreference(name);
	color[0] = float(hexint(colorString, 1, 3)) / 255;
	color[1] = float(hexint(colorString, 3, 5)) / 255;
	color[2] = float(hexint(colorString, 5, 7)) / 255;
}

void cProfile::loadPreferences(char* filename)
{
/*
//	#Read a configuration file as global config
	profileParser = ConfigParser.ConfigParser()
	try:
		profileParser.read(filename)
	except ConfigParser.ParsingError:
		return

	for set in settingsList:
		if set.isPreference():
			if profileParser.has_option("preference", set.getName()):
				set.setValue(unicode(profileParser.get("preference", set.getName()), "utf-8", "replace"))

	n = 0
	while profileParser.has_section("machine_%d" % (n)):
		for set in settingsList:
			if set.isMachineSetting():
				if profileParser.has_option("machine_%d" % (n), set.getName()):
					set.setValue(unicode(profileParser.get("machine_%d" % (n), set.getName()), "utf-8", "replace"), n)
		n += 1

*/
	setActiveMachine(int(getPreferenceFloat("active_machine")));
}

void loadMachineSettings(char* filename)
{
//	#Read a configuration file as global config
/*
profileParser = ConfigParser.ConfigParser()
	try:
		profileParser.read(filename)
	except ConfigParser.ParsingError:
		return

	for set in settingsList:
		if set.isMachineSetting():
			if profileParser.has_option("machine", set.getName()):
				set.setValue(unicode(profileParser.get("machine", set.getName()), "utf-8", "replace"))
*/
	checkAndUpdateMachineName();
}

void cProfile::savePreferences(char* filename)
{
/*
//	#Save the current profile to an ini file
	parser = ConfigParser.ConfigParser()
	parser.add_section("preference")

	for set in settingsList:
		if set.isPreference():
			parser.set("preference", set.getName(), set.getValue().encode("utf-8"))

	n = 0
	while getMachineSetting("machine_name", n) != "":
		parser.add_section("machine_%d" % (n))
		for set in settingsList:
			if set.isMachineSetting():
				parser.set("machine_%d" % (n), set.getName(), set.getValue(n).encode("utf-8"))
		n += 1
	parser.write(open(filename, "w"))
*/
}

char* cProfile::getPreference(char* name)
{
	cSetting* setting = tempOverride.find(name);
	if(setting)
		return setting->getValue(0);

	setting = settingsDictionary.find(name );
	if(setting && setting->isPreference())
		return setting->getValue();

	return "";
}

void cProfile::putPreference(char* name, char* value)
{
/*
//	#Check if we have a configuration file loaded, else load the default.
	if name in settingsDictionary and settingsDictionary[name].isPreference()) {
		settingsDictionary[name].setValue(value);
		savePreferences(getPreferencePath());
		return;
	}
*/
}

int cProfile::isPreference(char* name)
{
	cSetting *setting = settingsDictionary.find(name);
	if(setting && setting->isPreference())
		return True;
	return False;
}

float cProfile::getMachineSettingFloat(char* name, int index)
{
	char* setting = getMachineSetting(name, index);
	return atof(setting);
}

char* cProfile::getMachineSetting(char* name, int index)
{
	if (tempOverride.find(name))
		return tempOverride.find(name)->getValue(index);

	cSetting* setting = settingsDictionary.find(name);
	if (setting && setting->isMachineSetting()) {
		return setting->getValue(index);
	}

	return "";
}

void cProfile::putMachineSetting(char* name, char* value)
{
//	#Check if we have a configuration file loaded, else load the default.
	cSetting* setting = settingsDictionary.find(name);
	if(setting && setting->isMachineSetting())
		setting->setValue(value);
	savePreferences(getPreferencePath());
}

int cProfile::isMachineSetting(char* name)
{
	cSetting* setting = settingsDictionary.find(name);
	if (setting && setting->isMachineSetting())
		return True;

	return False;
}

void checkAndUpdateMachineName()
{
/*
char* name = getMachineSetting("machine_name");
	int index = -1;
	if (*name == 0)
		name = getMachineSetting("machine_type");
	for(int n = 0; n < getMachineCount(); n++) {
		if( n == _selectedMachineIndex)
			continue;
		if (index == -1) {
			if name == getMachineSetting("machine_name", n):
				index = 1
		} else {
			if "%s (%d)" % (name, index) == getMachineSetting("machine_name", n):
				index += 1;
		}
	}
	if (index != -1) {
		name = "%s (%d)" % (name, index);
	}
	putMachineSetting("machine_name", name);
	putPreference("active_machine", _selectedMachineIndex)
*/
}

int getMachineCount()
{
/*
n = 0
	while getMachineSetting("machine_name", n) != "":
		n += 1
	if n < 1:
		return 1
	return n;
*/
	return 1;
}

void cProfile::putPreference(char* name, int index)
{
	char s[16];
	sprintf_s(s, 16, "%d", index);
	putPreference(name, s);
}

void cProfile::setActiveMachine(int index)
{
	_selectedMachineIndex = index;
	putPreference("active_machine", _selectedMachineIndex);
}

void removeMachine(int index)
{
/*
if getMachineCount() < 2:
		return
	for n in xrange(index, getMachineCount()):
		for setting in settingsList:
			if setting.isMachineSetting():
				setting.setValue(setting.getValue(n+1), n)

	if _selectedMachineIndex >= index:
		setActiveMachine(getMachineCount() - 1)
*/
}

//## Temp overrides for multi-extruder slicing and the project planner.
void setTempOverride(char* name, char* value)
{
	cSetting* setting = new cSetting(name, value, IntType, "", "");
	tempOverride.append(setting);
}

void clearTempOverride(char* name)
{
//	del tempOverride[name]
}

void resetTempOverride()
{
	tempOverride.clear();
}


/*#########################################################
## Utility functions to calculate common profile values
#########################################################*/
float cProfile::calculateEdgeWidth()
{
	float wallThickness = getProfileSettingFloat("wall_thickness");
	float nozzleSize = getProfileSettingFloat("nozzle_size");

	if (strcmp(getProfileSetting("spiralize"), "True") == 0)
		return wallThickness;

	if (wallThickness < 0.01)
		return nozzleSize;
	if (wallThickness < nozzleSize)
		return wallThickness;

	float lineCount = int(wallThickness / (nozzleSize - 0.0001));
	if( lineCount == 0)
		return nozzleSize;
	float lineWidth = wallThickness / lineCount;
	float lineWidthAlt = wallThickness / (lineCount + 1);
	if (lineWidth > nozzleSize * 1.5)
		return lineWidthAlt;
	return lineWidth;
}

float cProfile::calculateLineCount()
{
	float wallThickness = getProfileSettingFloat("wall_thickness");
	float nozzleSize = getProfileSettingFloat("nozzle_size");

	if (wallThickness < 0.01)
		return 0;
	if (wallThickness < nozzleSize)
		return 1;

	if (strcmp(getProfileSetting("spiralize"), "True") == 0)
		return 1;

	float lineCount = int(wallThickness / (nozzleSize - 0.0001));
	if (lineCount < 1)
		lineCount = 1;
	float lineWidth = wallThickness / lineCount;
	float lineWidthAlt = wallThickness / (lineCount + 1);
	if( lineWidth > nozzleSize * 1.5)
		return lineCount + 1;

	return lineCount;
}

float cProfile::calculateSolidLayerCount()
{
	float layerHeight = getProfileSettingFloat("layer_height");
	float solidThickness = getProfileSettingFloat("solid_layer_thickness");
	if (layerHeight == 0.0)
		return 1;
	return int(ceil(solidThickness / (layerHeight - 0.0001)));
}


fpxy cProfile::calculateObjectSizeOffsets()
{
	float size = 0.0;

	if(strcmp(getProfileSetting("platform_adhesion") , "Brim") == 0) {
		size += getProfileSettingFloat("brim_line_count") * calculateEdgeWidth();
	} else if(strcmp( getProfileSetting("platform_adhesion"), "Raft") == 0) {
	} else {
		if( getProfileSettingFloat("skirt_line_count") > 0) {
			size += getProfileSettingFloat("skirt_line_count") * calculateEdgeWidth() + getProfileSettingFloat("skirt_gap");
		}
	}
//	#if getProfileSetting("enable_raft") != "False":
//	#	size += profile.getProfileSettingFloat("raft_margin") * 2
//	#if getProfileSetting("support") != "None":
//	#	extraSizeMin = extraSizeMin + numpy.array([3.0, 0, 0])
//	#	extraSizeMax = extraSizeMax + numpy.array([3.0, 0, 0])
	return fpxy(size, size);
}

fpxy cProfile::getMachineCenterCoords()
{
	if (strcmp(getMachineSetting("machine_center_is_zero") , "True") == 0)
		return fpxy(0,0);

	//return fpxy(getMachineSettingFloat("machine_width") / 2, getMachineSettingFloat("machine_depth") / 2);
	fpxyz size = GetMachineDimensions(); 
	return fpxy(size.data[0] / 2, size.data[1] / 2);
}

fpxyvect* makerect(fpxy a, fpxy b, fpxy c, fpxy d)
{
	fpxyvect* v = new fpxyvect(0, __FILE__, __LINE__);

	v->append(a);
	v->append(b);
	v->append(c);
	v->append(d);

	return v;
}

//#Returns a list of convex polygons, first polygon is the allowed area of the machine,
//# the rest of the polygons are the dis-allowed areas of the machine.
fpxyvectvect* cProfile::getMachineSizePolygons()
{
	fpxyvectvect *ret = new fpxyvectvect(__FILE__, __LINE__);
	fpxyz size = GetMachineDimensions(); // fpxyz(getMachineSettingFloat("machine_width"), getMachineSettingFloat("machine_depth"), getMachineSettingFloat("machine_height"));
	ret->add(makerect(fpxy(-size.data[0]/2,-size.data[1]/2),
		fpxy( size.data[0]/2,-size.data[1]/2),
		fpxy( size.data[0]/2, size.data[1]/2), 
		fpxy(-size.data[0]/2, size.data[1]/2)));

//	# Circle platform for delta printers...
//	# circle = []
//	# steps = 16
//	# for n in xrange(0, steps):
//	# 	circle.append([math.cos(float(n)/steps*2*math.pi) * size[0]/2, math.sin(float(n)/steps*2*math.pi) * size[0]/2])
//	# ret.append(numpy.array(circle, numpy.float32))

	if(strcmp(getMachineSetting("machine_type"), "ultimaker2") == 0) {
//		#UM2 no-go zones
		int w = 25;
		int h = 10;
		ret->add(makerect(fpxy(-size.data[0]/2,-size.data[1]/2),
			fpxy(-size.data[0]/2+w+2,-size.data[1]/2),
			fpxy(-size.data[0]/2+w,-size.data[1]/2+h),
			fpxy(-size.data[0]/2,-size.data[1]/2+h)));
		ret->add(makerect(fpxy(size.data[0]/2-w-2,-size.data[1]/2),
			fpxy(size.data[0]/2,-size.data[1]/2),
			fpxy(size.data[0]/2,-size.data[1]/2+h),
			fpxy(size.data[0]/2-w,-size.data[1]/2+h)));
		ret->add(makerect(fpxy(-size.data[0]/2+w+2, size.data[1]/2),
			fpxy(-size.data[0]/2, size.data[1]/2),
			fpxy(-size.data[0]/2, size.data[1]/2-h),
			fpxy(-size.data[0]/2+w, size.data[1]/2-h)));
		ret->add(makerect(fpxy(size.data[0]/2, size.data[1]/2),
			fpxy(size.data[0]/2-w-2, size.data[1]/2),
			fpxy(size.data[0]/2-w, size.data[1]/2-h),
			fpxy(size.data[0]/2, size.data[1]/2-h)));
	}
	
	return ret;
}

//#returns the number of extruders minimal used. Normally this returns 1, but with dual-extrusion support material it returns 2
int cProfile::minimalExtruderCount()
{
	if (int(getMachineSetting("extruder_amount")) < 2)
		return 1;
	if (strcmp(getProfileSetting("support"), "None") == 0)
		return 1;
	if (strcmp(getProfileSetting("support_dual_extrusion"), "Second extruder") == 0)
		return 2;
	return 1;
}

/*
#########################################################
## Alteration file functions
#########################################################
def replaceTagMatch(m):
	pre = m.group(1)
	tag = m.group(2)
	if tag == "time":
		return pre + time.strftime("%H:%M:%S")
	if tag == "date":
		return pre + time.strftime("%d-%m-%Y")
	if tag == "day":
		return pre + ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"][int(time.strftime("%w"))]
	if tag == "print_time":
		return pre + "#P_TIME#"
	if tag == "filament_amount":
		return pre + "#F_AMNT#"
	if tag == "filament_weight":
		return pre + "#F_WGHT#"
	if tag == "filament_cost":
		return pre + "#F_COST#"
	if pre == "F" and tag == "max_z_speed":
		f = getProfileSettingFloat("travel_speed") * 60
	if pre == "F" and tag in ["print_speed", "retraction_speed", "travel_speed", "bottom_layer_speed", "cool_min_feedrate"]:
		f = getProfileSettingFloat(tag) * 60
	elif isProfileSetting(tag):
		f = getProfileSettingFloat(tag)
	elif isPreference(tag):
		f = getProfileSettingFloat(tag)
	else:
		return "%s?%s?" % (pre, tag)
	if (f % 1) == 0:
		return pre + str(int(f))
	return pre + str(f)

def replaceGCodeTags(filename, gcodeInt):
	f = open(filename, "r+")
	data = f.read(2048)
	data = data.replace("#P_TIME#", ("%5d:%02d" % (int(gcodeInt.totalMoveTimeMinute / 60), int(gcodeInt.totalMoveTimeMinute % 60)))[-8:])
	data = data.replace("#F_AMNT#", ("%8.2f" % (gcodeInt.extrusionAmount / 1000))[-8:])
	data = data.replace("#F_WGHT#", ("%8.2f" % (gcodeInt.calculateWeight() * 1000))[-8:])
	cost = gcodeInt.calculateCost()
	if cost is None:
		cost = "Unknown"
	data = data.replace("#F_COST#", ("%8s" % (cost.split(" ")[0]))[-8:])
	f.seek(0)
	f.write(data)
	f.close()

def replaceGCodeTagsFromSlicer(filename, slicerInt):
	f = open(filename, "r+")
	data = f.read(2048)
	data = data.replace("#P_TIME#", slicerInt.getPrintTime())
	data = data.replace("#F_AMNT#", slicerInt.getFilamentAmount())
	data = data.replace("#F_WGHT#", ("%8.2f" % (float(slicerInt.getFilamentWeight()) * 1000))[-8:])
	cost = slicerInt.getFilamentCost()
	if cost is None:
		cost = "Unknown"
	data = data.replace("#F_COST#", ("%8s" % (cost.split(" ")[0]))[-8:])
	f.seek(0)
	f.write(data)
	f.close()

### Get aleration raw contents. (Used internally in Cura)
def getAlterationFile(filename):
	if filename in tempOverride:
		return tempOverride[filename]
	global settingsDictionary
	if filename in settingsDictionary and settingsDictionary[filename].isAlteration():
		return settingsDictionary[filename].getValue()
	traceback.print_stack()
	sys.stderr.write("Error: "%s" not found in alteration settings\n" % (filename))
	return ""

def setAlterationFile(name, value):
	#Check if we have a configuration file loaded, else load the default.
	global settingsDictionary
	if name in settingsDictionary and settingsDictionary[name].isAlteration():
		settingsDictionary[name].setValue(value)
	saveProfile(getDefaultProfilePath())

def isTagIn(tag, contents):
	contents = re.sub(";[^\n]*\n", "", contents)
	return tag in contents
*/
//### Get the alteration file for output. (Used by Skeinforge)
char* getAlterationFileContents(char* altername, int extruderCount)
{
/*
	prefix = ""
	postfix = ""
	alterationContents = getAlterationFile(filename)
	if getMachineSetting("gcode_flavor") == "UltiGCode":
		if filename == "end.gcode":
			return "M25 ;Stop reading from this point on.\n;CURA_PROFILE_STRING:%s\n" % (getProfileString())
		return ""
	if filename == "start.gcode":
		if extruderCount > 1:
			alterationContents = getAlterationFile("start%d.gcode" % (extruderCount))
		#For the start code, hack the temperature and the steps per E value into it. So the temperature is reached before the start code extrusion.
		#We also set our steps per E here, if configured.
		eSteps = getMachineSettingFloat("steps_per_e")
		if eSteps > 0:
			prefix += "M92 E%f\n" % (eSteps)
		temp = getProfileSettingFloat("print_temperature")
		bedTemp = 0
		if getMachineSetting("has_heated_bed") == "True":
			bedTemp = getProfileSettingFloat("print_bed_temperature")

		if bedTemp > 0 and not isTagIn("{print_bed_temperature}", alterationContents):
			prefix += "M140 S%f\n" % (bedTemp)
		if temp > 0 and not isTagIn("{print_temperature}", alterationContents):
			if extruderCount > 0:
				for n in xrange(1, extruderCount):
					t = temp
					if n > 0 and getProfileSettingFloat("print_temperature%d" % (n+1)) > 0:
						t = getProfileSettingFloat("print_temperature%d" % (n+1))
					prefix += "M104 T%d S%f\n" % (n, t)
				for n in xrange(0, extruderCount):
					t = temp
					if n > 0 and getProfileSettingFloat("print_temperature%d" % (n+1)) > 0:
						t = getProfileSettingFloat("print_temperature%d" % (n+1))
					prefix += "M109 T%d S%f\n" % (n, t)
				prefix += "T0\n"
			else:
				prefix += "M109 S%f\n" % (temp)
		if bedTemp > 0 and not isTagIn("{print_bed_temperature}", alterationContents):
			prefix += "M190 S%f\n" % (bedTemp)
	elif filename == "end.gcode":
		if extruderCount > 1:
			alterationContents = getAlterationFile("end%d.gcode" % (extruderCount))
		#Append the profile string to the end of the GCode, so we can load it from the GCode file later.
		postfix = ";CURA_PROFILE_STRING:%s\n" % (getProfileString())
	return unicode(prefix + re.sub("(.)\{([^\}]*)\}", replaceTagMatch, alterationContents).rstrip() + "\n" + postfix).strip().encode("utf-8") + "\n"
*/
	return "";
}
/*
###### PLUGIN #####

def getPluginConfig():
	try:
		return pickle.loads(str(getProfileSetting("plugin_config")))
	except:
		return []

def setPluginConfig(config):
	putProfileSetting("plugin_config", pickle.dumps(config))

def getPluginBasePaths():
	ret = []
	if platform.system() != "Windows":
		ret.append(os.path.expanduser("~/.cura/plugins/"))
	if platform.system() == "Darwin" and hasattr(sys, "frozen"):
		ret.append(os.path.normpath(os.path.join(resources.resourceBasePath, "Cura/plugins")))
	else:
		ret.append(os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "plugins")))
	return ret

def getPluginList():
	ret = []
	for basePath in getPluginBasePaths():
		for filename in glob.glob(os.path.join(basePath, "*.py")):
			filename = os.path.basename(filename)
			if filename.startswith("_"):
				continue
			with open(os.path.join(basePath, filename), "r") as f:
				item = {"filename": filename, "name": None, "info": None, "type": None, "params": []}
				for line in f:
					line = line.strip()
					if not line.startswith("#"):
						break
					line = line[1:].split(":", 1)
					if len(line) != 2:
						continue
					if line[0].upper() == "NAME":
						item["name"] = line[1].strip()
					elif line[0].upper() == "INFO":
						item["info"] = line[1].strip()
					elif line[0].upper() == "TYPE":
						item["type"] = line[1].strip()
					elif line[0].upper() == "DEPEND":
						pass
					elif line[0].upper() == "PARAM":
						m = re.match("([a-zA-Z][a-zA-Z0-9_]*)\(([a-zA-Z_]*)(?::([^\)]*))?\) +(.*)", line[1].strip())
						if m is not None:
							item["params"].append({"name": m.group(1), "type": m.group(2), "default": m.group(3), "description": m.group(4)})
					else:
						print "Unknown item in effect meta data: %s %s" % (line[0], line[1])
				if item["name"] is not None and item["type"] == "postprocess":
					ret.append(item)
	return ret

def runPostProcessingPlugins(gcodefilename):
	pluginConfigList = getPluginConfig()
	pluginList = getPluginList()

	for pluginConfig in pluginConfigList:
		plugin = None
		for pluginTest in pluginList:
			if pluginTest["filename"] == pluginConfig["filename"]:
				plugin = pluginTest
		if plugin is None:
			continue

		pythonFile = None
		for basePath in getPluginBasePaths():
			testFilename = os.path.join(basePath, pluginConfig["filename"])
			if os.path.isfile(testFilename):
				pythonFile = testFilename
		if pythonFile is None:
			continue

		locals = {"filename": gcodefilename}
		for param in plugin["params"]:
			value = param["default"]
			if param["name"] in pluginConfig["params"]:
				value = pluginConfig["params"][param["name"]]

			if param["type"] == "float":
				try:
					value = float(value)
				except:
					value = float(param["default"])

			locals[param["name"]] = value
		try:
			execfile(pythonFile, locals)
		except:
			locationInfo = traceback.extract_tb(sys.exc_info()[2])[-1]
			return "%s: "%s" @ %s:%s:%d" % (str(sys.exc_info()[0].__name__), str(sys.exc_info()[1]), os.path.basename(locationInfo[0]), locationInfo[2], locationInfo[1])
	return None
*/

fpxyz GetMachineDimensions()
{
	fpxyz dim = fpxyz(profile.getMachineSettingFloat("machine_width"), profile.getMachineSettingFloat("machine_depth"), profile.getMachineSettingFloat("machine_height"));
	if(dim.data[0] == 0)
		dim.data[0] = DEFAULT_WIDTH;
	if(dim.data[1] == 0)
		dim.data[1] = DEFAULT_LENGTH;
	if(dim.data[2] == 0)
		dim.data[2] = DEFAULT_HEIGHT;

	return dim;
}

