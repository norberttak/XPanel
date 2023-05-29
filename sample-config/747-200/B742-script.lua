
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

function get_vs_selected()
    vs = get_dataref("B742/AP_panel/VS_rotary")
    --return math.abs(1000*vs)
    return 1000*vs
end

function landing_light_green()
    ld_lamp_lg_down = get_dataref("B742/front_lamps/LG_gear_down_lit")
    battery_sw = get_dataref("B742/ELEC/battery_sw")

    if (ld_lamp_lg_down > 0.8 and battery_sw == 1) then
        return 1
    else
        return 0
    end
end

function landing_light_red()
    ld_lamp_lg_down = get_dataref("B742/front_lamps/LG_gear_down_lit")
    battery_sw = get_dataref("B742/ELEC/battery_sw")

    if (ld_lamp_lg_down < 0.8 and battery_sw == 1) then
        return 1
    else
        return 0
    end
end

function hsi_course_rotation()
    course_set = get_dataref("B742/HSI/flight_course_deg")
    heading = get_dataref("sim/cockpit2/gauges/indicators/compass_heading_deg_mag")
    return course_set - heading
end

function hsi_course_dev_translation_x()
    dots = get_dataref("B742/HSI/LOC_ratio")
    max_dev_dot = 1.25

    if (dots > max_dev_dot) then
        dots = max_dev_dot
    elseif (dots < -1*max_dev_dot) then
        dots = -1*max_dev_dot
    end

    scale_half_width = 44
    return 200 + math.cos((-1*hsi_course_rotation()*3.14)/180) * (scale_half_width * (dots/max_dev_dot))    
end

function hsi_course_dev_translation_y()
    dots = get_dataref("B742/HSI/LOC_ratio")
    max_dev_dot = 1.25

    if (dots > max_dev_dot) then
        dots = max_dev_dot
    elseif (dots < -1*max_dev_dot) then
        dots = -1*max_dev_dot
    end

    scale_half_width = 44
    return 120 + math.sin((-1*hsi_course_rotation()*3.14)/180) * (scale_half_width * (dots/max_dev_dot))    
end

function get_current_baro_hpa()
    baro_hgin = get_dataref("sim/cockpit/misc/barometer_setting")
    return math.floor(33.86389*baro_hgin+0.5)
end

baro_set_to_std = false
function S1_button_set_altimeter()
    if (baro_set_to_std) then
        local_qnh = get_dataref("sim/weather/barometer_sealevel_inhg")
        set_dataref("sim/cockpit/misc/barometer_setting",local_qnh)
        baro_set_to_std = false
    else
        command_once("sim/instruments/barometer_2992")
        baro_set_to_std = true
    end
end

function S1_button_lit()
    if (baro_set_to_std) then
        return 1
    else
        return 0
    end
end

function get_mach_speed()
    return math.floor(get_dataref("sim/cockpit2/gauges/indicators/mach_pilot")*1000)
end

function get_gs_needle_pos()
 return 120 + get_dataref("B742/HSI/GS_ratio",0)*36
end
