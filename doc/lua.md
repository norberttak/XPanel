# Lua scripts in configuration

XPanel plugin is shipped with a [Lua 5.4](http://www.lua.org/) interpreter. You can call Lua expressions from the config file.
The more convenient way is to put all you Lua codes int a .lua file and refer this in your config file.

This plugin is independent of the [FlyWithLua plugin](https://github.com/X-Friese/FlyWithLua)! You can't use the functions defined by that plugin.

The following Lua command are defined by the plugin:

## Do an XPlane command
```lua 
command_once('/xplane/command')
command_begin('/xplane/command')
command_end('/xplane/command')
```
The parameters are the xplane command names as a string.

You can trigger an xplane command on three different ways: The "begin" means like you keep push a button.
The "end" means you release the button. If you don't care about the length of button press you can issue a single command as "once"
This contains a begin and immediately an end.

## Set or get XPlane datarefs
```lua
value = get_dataref('/xplane/dataref')
set_dataref('/xplane/dataref',value)
```
The Xplane dataref values can set or get by these functions. The first parameter is the dataref name as a string.
The get_dataref will return with the current value of the dataref.

The set_dataref second parameter is the value what you want to set. Please be careful to pass the right type of value here.
XPlane checks the value type and reject it if it doesn't match with the required type of the dataref.

The plugin tries to convert it to the required type but it could make tricky issues if you don't care about the type of values.
The dataref types are listed [here](https://developer.x-plane.com/sdk/XPLMDataAccess/#XPLMDataTypeID)

You may know that it's a high cost call into the XPlane system to find an internal dataref by the name (more details [here](https://developer.x-plane.com/sdk/XPLMDataAccess/)) Therefore the plugin will 
store the dataref pointer so the costly XPLMFindDataRef will be called only once.

## Logger command
To put a log line into the xplane's log file you can use this lua command. 
The first parameter determines the log level. If the actual log level is higher than your message here (for example you call log_msg with first parameter as 'TRACE' and the log level is set to INFO by the [config file](configuration.md)) 
your log message will be ignored.

```
log_msg('ERROR|WARNING|INFO|DEBUG|TRACE','log message')
```

# How to use the Lua commands in the config file?
An example lua script file can be found [here](../test/test-script.lua). This script defines a function **button_AP** with one parameter named **action** which can be
* 'push'
* 'release'
* 'once'

```lua
function button_AP(action)
    log_msg("TRACE","button AP handler "..action)
    if (action == "push") then
        command_begin("/sim/test/lua/button_AP")
    elseif (action == "release") then
        command_end("/sim/test/lua/button_AP")
    elseif (action == "once") then
        command_once("/sim/test/lua/button_AP")
    else
        log_msg("ERROR","invalid action parameter "..action) 
    end
end
```
You can call the button_AP function from the config file like this:
```ini
script_file="test-script.lua"

[button:id="AP"] 
on_push="lua:button_AP('push')"
on_release="lua:button_AP('release')"
```
Please note that in the config file we use single quote (') instead of double (") in the lua function parameter.

## Flight loop function
If you define a LUA function named flight_loop(param) in your script, that function will be periodically called by the plugin in every flight loop.
The param is the elapsed time (in seconds) since the previous call of the function.
You can use the dataref set/get and command handler functions as well from this flight loop function.

```lua
function flight_loop(param)
    log_msg("TRACE","flight loop called. Time elapsed "..param)
    set_dataref("/sim/test/lua_flight_loop",12345)
end
```
