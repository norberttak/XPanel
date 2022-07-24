
function radio_transfer(radio_name)
    log_msg("TRACE","button "..radio_name.." transfer handler ")
    value = get_dataref('B742/'..radio_name..'/TFR_sw')

    if (value == 1) then
        set_dataref('B742/'..radio_name..'/TFR_sw',0)
    else
        set_dataref('B742/'..radio_name..'/TFR_sw',1)
    end    
end

function autopilot_state_toggle()
    log_msg("TRACE","auto pilot state toggle")
    value = get_dataref("B742/AP_panel/AP_engage_A")

    if (value == 2) then
        set_dataref("B742/AP_panel/AP_engage_A",0)
    else
        set_dataref("B742/AP_panel/AP_engage_A",2)
    end
end

function altitude_select_mode_toggle()
    value=get_dataref("B742/AP_panel/altitude_mode_sw")

    if (value == -1) then
        set_dataref("B742/AP_panel/altitude_mode_sw",0)
    else
        set_dataref("B742/AP_panel/altitude_mode_sw",-1)
    end
end
