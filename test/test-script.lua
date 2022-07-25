set_dataref("/sim/test/lua_script_test1",567)

log_msg("TRACE","test-script loaded ")

function flight_loop(param)
    log_msg("TRACE","flight loop called "..param)
    set_dataref("/sim/test/lua_flight_loop",12345)
end

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

function get_display_value()
    log_msg("TRACE","get_display_value() called")
    return 67890
end
