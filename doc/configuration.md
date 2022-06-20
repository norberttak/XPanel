# XPanel v1.0 Configiration file format
## General description
The configuration file format is _similar_ to the ini file format. It is divided into sections. A section can have properties. A section is marked by square brackets ([ ]). Every section shall have a unique id.
```ini
[button:id="NAV"]
```
The configuration file shall be named as **xpanel.ini** and need to put be into the folder of your aircraft.

### Comments
All characters after a semi-column (;) are considered as comments.
```ini
on_push="commandref:/sim/cmd/HDG:begin";this is a comment
;this is a full line comment
```
### Log level
This plugin has a very simple log facility that can be used during debug sessions. The log level can be set from the configuration files:
```ini
log_level="TRACE|DEBUG|INFO|WARNING|ERROR"
```
Set the log level of the plugin. The log lines are written to XPlane's default log file. In normal use-cases set the log_level to ERROR.
```ini
log_level="ERROR"
```
### Aircraft ACF file

The ACF file is the main file for an XPlane aircraft. The plugin gets the ACF file name parameter from the XPLane system. The reason to put the ACF file name into the configuration is only for safety. During parse, we can check if accidentally a wrong configuration file (created for another aircraft) has been loaded.
```ini
aircraft_acf="tu154.acf"
```
### Script file

You can write action handlers (see later) in LUA script as well. The plugin will load and interpret the script file you specify here. The script file shall be in the current aircraft's folder (the same directory as the xpanel.ini config file)
```ini
script_file="TU154-arduino-home-cockpit.lua"
```
## Devices
Devices can be defined by a new section in the configuration file. Currently, it supports only two types of devices: XSaintek's Multi Panel and a homemade Arduino-based IO board.
```ini
[device:id="saitek_multi"]
[device:id="aurduino_homecockpit"]
```
The devices have a few config options which can be set by the configuration file.
* USB VID (Vendor ID), 
* USB PID (Product ID), 
* the aircraft .acf file (this is the main file of an XPlane aircraft. This uniquely identifies the loaded aircraft)
* LUA script file which will be loaded in parallel to the initialization of the plugin.

```ini
[device:id="aurduino_homecockpit"]
vid="2341"
pid="8036"
```

## Buttons
An input device on the HW panel (button, switch, rotation switch) needs to be mapped as __button__ in the configuration file. Practically every bit mapped input device is handled as a button. Even if it is a rotary encoder but it is mapped to USB register a bit value.
Every button shall be defined as a new section and need to have a pre-defined id. The possible id values:
### Saitek Multi Panel's button ids:
| Button ID | Recommended Function  |
|---|---|
|AP | Autopilot |
|HDG | Heading mode
|NAV | Navigation mode
|IAS | Indicated Air Speed (IAS) hold mode
|ALT | Altitude hold mode
|VS | Vertical speed hold mode
|APR | Approach mode
|REV | Revers aproach mode
|AUTO_THROTTLE | Auto throttle arm
|FLAPS_UP | Flaps up handle
|FLAPS_DOWN | Flaps down handle
|TRIM_WHEEL_DOWN | Trimm wheel
|TRIM_WHEEL_UP | Trimm wheel
|KNOB_PLUS | Multi function rotation knob, + direction
|KNOB_MINUS | Multi function rotation knob, - direction

### Arduino based IO board
| Button ID | Register[bit] | Recommended Function  |
|---|---|---|
STROBE|1[0]|Strobe light
DOME|1[1]|Cockpit dome light
LAND|1[2]|Landing light
TAXI|1[3]|Taxi light
BEACON|1[4]|Beacon light
CAUTION|1[5]|Caution clear button
START_L|1[6]|Engine start left
WARNING|1[7]|Warning clear button
START_R|2[0]|Engine start right
PITOT|2[1]|Pitot heat
SEAT_BELT|2[2]|Seat belt sign
PROP_SYNC|2[3]|Propeller sync
AUTO_COARS|2[4]|Autocoarsen
XPDR|2[5]|Transponder idle <-> TA mode

### Define an action for a button
Every button can define multiple push and release actions. An action could be either
* set a `dataref` to a specific value (integer, float or an array)
* increase or decrease a `dataref` by a delta
* execute a `command` 
* execute a `lua` code

It is possible to define an action with a condition. This could be used for multipurpose HW elements (like the sliver rotation knob on the Saitek multi panel)

The next example is a simple action. When you push the button (or put the switch to on poisition) it sets the `sim/custom/lights/nav_lights_set` dataref to 1. When you release the button (switch set to 0) it sets the dataref to 0.
```ini
[button:id="STROBE"]
on_push="dataref:sim/custom/lights/nav_lights_set:1"
on_release="dataref:sim/custom/lights/nav_lights_set:0"
```
To set a value in an array you can use the following syntax. This will set the index 0 element of the array to value 1
```ini
on_push="dataref:/sim/data/data_array[0]:1"
```

The following is show how to use XPlane commands. When you push the button, it issues a command begin to Xplane with the given command ref. When you release the button, we issue a command end to Xplane.
If you don't care about the length of a button press then you can use the commands with `:once` modifier. This will issue a single command to Xplane which contains a push and a release event as well. 
```ini
[button:id="HDG"]
on_push="commandref:/sim/cmd/HDG:begin"
on_push="commandref:/sim/cmd/HDG:end"
```

## Example configuration file
```ini
log_level="TRACE"
script_file="tu154-saitket-multipanel.lua"
aircraft_acf="generic.acf"

;----------- Saitek Multi Panel --------------
[device:id="saitek_multi"]
vid="12AB"
pid="34CD"

;AP button
[button:id="AP"] 
on_release="dataref:/sim/hello/AP:0"; test for button press
on_release="dataref:/hello/bello:0"
on_push="dataref:/sim/hello/AP:1"
on_push="dataref:/sim/hello/AP2:1"

;NAV button
[button:id="NAV"]
on_push="commandref:/sim/cmd/NAV:begin"
on_release="commandref:/sim/cmd/NAV:end"

;---------------- Arduino based IO board ------
[device:id="aurduino_homecockpit"]
vid="2341"
pid="8036"

;STROBE light
[button:id="STROBE"]
on_push="dataref:sim/cockpit/electrical/strobe_lights_on:-1"
on_release="dataref:sim/cockpit/electrical/strobe_lights_on:0"

;BEACON light
[button:id="BEACON"]
on_push="dataref:sim/cockpit/electrical/beacon_lights_on:1"
on_release="dataref:sim/cockpit/electrical/beacon_lights_on:0"
```
