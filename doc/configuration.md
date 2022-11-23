# XPanel v1.2 Configiration file format
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
Set the log level of the plugin. The log lines are written to X-Plane's default log file. In normal use-cases set the log_level to ERROR.
```ini
log_level="ERROR"
```
### Aircraft ACF file

The ACF file is the main file for an X-Plane aircraft. The plugin gets the ACF file name parameter from the X-Plane system. The reason to put the ACF file name into the configuration is only for safety. During parse, we can check if accidentally a wrong configuration file (created for another aircraft) has been loaded.
```ini
aircraft_acf="tu154.acf"
```
### Script file

You can write action handlers (see later) in LUA script as well. The plugin will load and interpret the script file you specify here. The script file shall be in the current aircraft's folder (the same directory as the xpanel.ini config file)
```ini
script_file="TU154-arduino-home-cockpit.lua"
```
The details of the LUA script files are in the [lua config description](lua.md) document.
## Devices
Devices can be defined by a new section in the configuration file. Currently, it supports only two types of devices: XSaintek's Multi Panel and a homemade Arduino-based IO board.
```ini
[device:id="saitek_multi"]
[device:id="aurduino_homecockpit"]
```
The devices have a few config options which can be set by the configuration file.
* USB VID (Vendor ID),
* USB PID (Product ID),
* the aircraft .acf file (this is the main file of an X-Plane aircraft. This uniquely identifies the loaded aircraft)
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
| ID | Recommended Function  |
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
|SW_ALT | Selector Switch, ALT posiiton
|SW_VS | Selector Switch, VS posiiton
|SW_IAS | Selector Switch, IAS posiiton
|SW_HDG | Selector Switch, HDG posiiton
|SW_CRS | Selector Switch, CRS posiiton
|MULTI_DISPLAY_UP | Upper Display (7 segments, 5 digits)
|MULTI_DISPLAY_DOWN | Down Display (7 segments, 5 digits)

### Saitek Radio Panel's ids:
| ID | Recommended Function  |
|---|---|
|KNOB_UP_BIG_PLUS | Upper Rotation Knob: Outer (big) Ring, Plus Direction
|KNOB_UP_BIG_MINUS | Upper Rotation Knob: Outer (big) Ring, Minus Direction
|KNOB_UP_SMALL_PLUS | Upper Rotation Knob: Iner (small) Button, Plus Direction
|KNOB_UP_SMALL_MINUS | Upper Rotation Knob: Iner (small) Button, Minus Direction
|KNOB_DOWN_BIG_PLUS | Down Rotation Knob: Outer (big) Ring, Plus Direction
|KNOB_DOWN_BIG_MINUS | Down Rotation Knob: Outer (big) Ring, Minus Direction
|KNOB_DOWN_SMALL_PLUS | Down Rotation Knob: Iner (small) Button, Plus Direction
|KNOB_DOWN_SMALL_MINUS | Upper Rotation Knob: Iner (small) Button, Minus Direction
|ACT_STBY_UP | Upper Xchange button (Active <--> Standby)
|ACT_STBY_DOWN | Down Xchange button (Active <--> Standby)
|SW_UP_COM1 | Upper Selector Switch, COM1 position
|SW_UP_COM2 | Upper Selector Switch, COM2 position
|SW_UP_NAV1 | Upper Selector Switch, NAV1 position
|SW_UP_NAV2 | Upper Selector Switch, NAV2 position
|SW_UP_ADF | Upper Selector Switch, ADF position
|SW_UP_DME | Upper Selector Switch, DME position
|SW_UP_IDT | Upper Selector Switch, IDT position
|SW_DOWN_COM1 | Down Selector Switch, COM1 position
|SW_DOWN_COM2 | Down Selector Switch, COM2 position
|SW_DOWN_NAV1 | Down Selector Switch, NAV1 position
|SW_DOWN_NAV2 | Down Selector Switch, NAV2 position
|SW_DOWN_ADF | Down Selector Switch, ADF position
|SW_DOWN_DME | Down Selector Switch, DME position
|SW_DOWN_IDT | Down Selector Switch, IDT position
|RADIO_DISPLAY_STBY_UP | Upper Standby Display (7 segments, 5 digits)
|RADIO_DISPLAY_ACTIVE_UP | Upper Active Display (7 segments, 5 digits)
|RADIO_DISPLAY_STBY_DOWN | Down Standby Display (7 segments, 5 digits)
|RADIO_DISPLAY_ACTIVE_DOWN | Down Active Display (7 segments, 5 digits)


### Arduino based IO board
If you have a USB HID capable IO board (like I have an Arduino Lenoardo) you
can define the logical (or symbolic) names for each register bits in a configuration file. A USB HID report
is based on bytes so the config file follows this order as well.

To define a symbolic name you need to select the register (1 byte long registers with  0 based index)
and then the bit index. In the example bellow we define two symbolic names (STROBE and DOME)
which are in the register 1 and assign to the 0 and the 1 bit respectively.

```ini
[register:adr=1]
button:id="STROBE",bit=0
button:id="DOME",bit=1

[register:adr=5]
display:id="ALTIMETER_GAUGE",width=2
```
In the aircraft specific configuration files you can use these symbolic names.

The board sepcific config file is not aircraft specific therefore you should have only one instance
of this config and put it to the same folder where the plugin binary installed (for example:
c:\xplane11\resources\plugins\XPanel\64\board-config.ini)

The release package contains my board-config.ini but for sure you have to modify it
according to your HW design.

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

You can define actions that change a dataref value by a given delta. This can be used for rotation knob handlers where you change some proportional value (like heading or course). You can also define a min and max value and the plugin won't change above or bellow the given limits.
```ini
on_push="dataref:<dataref_name>:<delta>:<min>:<max>"
```
These kind of conditional action can be used for multipurpose handlers/displays.
A good example is the Saitek Multi Panel. There is a rotation switch (left side of the display)
where you can select the function of the silver rotation knob KNOB_MINUS/KNOB_PLUS (right side of display) also
the display value on the display.

```ini
[button:id="KNOB_PLUS"]
on_push="on_select:SW_HDG,dataref:test/dynamic_speed_test:1:0:359"

[button:id="KNOB_MINUS"]
on_push="on_select:SW_HDG,dataref:test/dynamic_speed_test:-1:0:359"
```
The above example will change the dataref value if the selector switch is at SW_HDG position. The minimum is 0 and the maximum is 359.

#### Dynamic speed feature for the dataref change actions
It is very boring to rotate the knobs for a long time when you need to change the values on a wide range. To help this you can define speed factors for the action. It measures the time elapsed between the same actions (rotation in + direction for example) and based on the time interval it will apply a multiplier for the dataref change delta.
You can define two times intervals: t_low and t_high. These are for pretty fast :-) and very fast rotation speeds respectively. You can define such behavior for every button with this syntax:
```ini
[button:id="KNOB_PLUS"]
dynamic_speed="t_low=0.5x2,t_high=0.25x4"
on_push="on_select:SW_HDG,dataref:test/dynamic_speed_test:1:0:359"
```
This will apply a x4 speed factor if the time elapsed between the two actions is less than 0.25 sec and will apply a x2 factor if the elapsed time is between 0.25..0.5 sec. If the time is over 0.5 sec the dynamic speed is disabled and a multiplier x1 will be applied.

The following is show how to use X-Plane commands. When you push the button, it issues a command begin to X-Plane with the given command ref. When you release the button, we issue a command end to X-Plane.
If you don't care about the length of a button press then you can use the commands with `:once` modifier. This will issue a single command to X-Plane which contains a push and a release event as well.
```ini
[button:id="HDG"]
on_push="commandref:/sim/cmd/HDG:begin"
on_push="commandref:/sim/cmd/HDG:end"
```

## Lights

A light means an LED on the panel. This can be as a standalone LED or a background lit of a Saitek Panel's
button.

To decide about turn on/of the light you need to define triggers. A trigger means a condition
and when the condition is true the lit (turn on) or unlit (turn off) will happen.

If neither lit nor unlit condition meets it means we don't change the the sate of the light.

For sure you can define multiple lit/unlit condition for a trigger. All the triggers will be evaluated
and the last true condition will be the dominant.

You can use either dataref value or return value of a LUA function for triggers:
```ini
[light:id="NAV_L"]
trigger_lit="dataref:sim/custom/lights/button/absu_stab_h:1"
trigger_unlit="dataref:sim/custom/lights/button/absu_stab_h:0"

[light:id="AP_L"]
trigger_lit="lua:get_led_status():1"
trigger_unlit="lua:get_led_status():0"
```

The first section of the above config snippet, is for the background light of the NAV button
on a Saitek Multi panel. The LED will be turned on if the dataref value is 1 and will be turned off when
the dataref value is 0.

The secound snippet use LUA function. The plugin will call the lua function (get_led_status in this example)
and check the return value.

## Displays

A display is a charachter based 7 segment display device or an analogue gauge. It can be used for display numeric values.
The display value can be either from a dataref or from a LUA function. The display value can a conditional
display which means the value to display is depends on the position of a switch. A display taht contains conditions called
multi purpose displya (multi_display).

The 'on_select:HW input name' part defines a condition. If the HW input is in logical 1 state
the display will show you the dataref or lua script value in that line, Thi is somehow similar to a
switch-case instruction in C.

```ini
[multi_display:id="MULTI_DISPLAY_UP"]
line="on_select:SW_ALT,dataref:sim/custom/gauges/compas/pkp_helper_course_L"
line="on_select:SW_VS,lua:get_my_display_value()"
```

The firts line will display the actual value of the dataref. The secound line will
call the LUA function and displays the return value of the function.

The SW_ALT or SW_VS will determine which value will be displayed.

If you need a display device without any condition (it means the display will show the same dataeref or lua value all the time)
you can define a simple display device in the configuration like this:

```ini
[display:id="ALTIMETER"]
line="dataref:sim/test/altimeter"
```

## Flight Istrument Panel (FIP) devices
The configuration options for the FIP graphical devices can be found [here](doc/fip-screen.md)

## Example configuration file
```ini
log_level="TRACE"
script_file="tu154-saitek-multipanel.lua"
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

;AP button light
[light:id="AP_L"]
trigger_lit="lua:get_led_status():1"
trigger_unlit="lua:get_led_status():0"

;NAV button
[button:id="NAV"]
on_push="commandref:/sim/cmd/NAV:begin"
on_release="commandref:/sim/cmd/NAV:end"

[light:id="NAV_L"]
trigger_lit="dataref:sim/custom/lights/button/absu_stab_h:1"
trigger_unlit="dataref:sim/custom/lights/button/absu_stab_h:0"

[multi_display:id="MULTI_DISPLAY_UP"]
line="on_select:SW_ALT,dataref:sim/custom/gauges/compas/pkp_helper_course_L"
line="on_select:SW_VS,dataref:sim/custom/gauges/compas/pkp_helper_course_L"
line="on_select:SW_HDG,dataref:sim/custom/gauges/compas/pkp_helper_course_L"

[multi_display:id="MULTI_DISPLAY_DOWN"]
line="on_select:SW_ALT,dataref:sim/custom/gauges/compas/pkp_helper_course_L"
line="on_select:SW_VS,dataref:sim/custom/gauges/compas/pkp_helper_course_L"

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
