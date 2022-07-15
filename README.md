# XPanel
## What is this?
This is a plugin for [XPlane 11](https://www.x-plane.com/) flight simulator. If you have or plan to build a home cockpit then it is good to check this. It can connect your USB HID hardware devices to the XPlane11 system.

It has a configuration file where you can define the logical connections between hardware elements (buttons, switches, displays, etc) and the internal [dataref](https://developer.x-plane.com/sdk/) of XPlane.

Currently, it supports two types of USB devices:

1. [Saitek Multi Panel](https://www.saitek.com/uk/prod-bak/multi.html) This is a device that mainly contains the buttons associated with the autopilot functions.
2. [Saitek Radio Panel](https://www.saitek.com/uk/prod-bak/radio.html) Device to conrol radio functions of your airplane
3. Arduino based USB HID device which can simulate switches and displays customized on your own.

## Configuration syntax
For configuration file syntax please see the document [here](doc/configuration.md)

## Install
Get the latest release from [github](https://github.com/norberttak/XPanel/releases)

Copy the xpanel folder into your XPlane plugin directory (in my case it is c:\XPlane11\resources\plugins).

Please don't forget to remove any other plugins that want to connect to your USB Hid devices in the home cockpit.

If you have any errors during the plugin load or run please check the main xplane log file. If you want more detailed logs from the plugin, please set the log level to DEBUG or even TRACE. See the details at [config syntax](doc/configuration.md)

## Build
The project can be built by Visual Studio 2022 on windows machines. It can be ported to Linux systems as well but that will be in a later phase only.

Check out the latest source file from [github](https://github.com/norberttak/XPanel)

### Build the plugin and unit tests
Open the solution file (XPanel.sln) with Visual Studio. Select either Release or Debug build configuration. The solution file contains two projects. One for the xpanel plugin and one for the unit tests.

The build artifact of the plugin is generated in the Release|Debug/plugin/xpanel folder (win.xpl file)

### Run unit tests
To run the unit tests, open the Visual Studio test menu and select Test Explorer. Push the run-all button and check the test results. 

## Report bugs
If you find any bug in the plugin or need help please open a new bug report at [github](https://github.com/norberttak/XPanel/issues) Please always attach the Xplane log file and make sure you set the highest log level (TRACE) in the plugin config file. Also attach your plugin configuration file as well. 
