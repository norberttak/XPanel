-- Global function
local function set_pos_by_cmd(init_pos,end_pos,cmd_dec,cmd_inc)
    cur_pos = init_pos

    while (cur_pos ~= end_pos) do
        if (cur_pos > end_pos) then
            command_once(cmd_dec)
            cur_pos = cur_pos - 1  
        else
            command_once(cmd_inc)
            cur_pos = cur_pos + 1
        end

    end    
end
------------------

function power(action)
    log_msg("TRACE","button POWER handler "..action)
    
    FF777_GPU_1_AVAILABLE = get_dataref("1-sim/ckpt/lampsGlow/primExtPwrAVAIL")
    FF777_GPU_2_AVAILABLE = get_dataref("1-sim/ckpt/lampsGlow/secExtPwrAVAIL")
    FF777_GPU_1_ON = get_dataref("1-sim/ckpt/lampsGlow/primExtPwrON")
    FF777_GPU_2_ON = get_dataref("1-sim/ckpt/lampsGlow/secExtPwrON")
    FF777_ENG_1_AVAILABLE = get_dataref("1-sim/ckpt/lampsGlow/leftGenOFF")
    FF777_ENG_2_AVAILABLE = get_dataref("1-sim/ckpt/lampsGlow/rightGenOFF")
    FF777_ENG_1_ON = get_dataref("1-sim/ckpt/bLamps/leftGenButton")
    FF777_ENG_2_ON = get_dataref("1-sim/ckpt/bLamps/rightGenButton")    
    FF777_APU_AVAILABLE = get_dataref("1-sim/ckpt/lampsGlow/apuGenOFF")
    FF777_APU_OFF = get_dataref("1-sim/ckpt/lamps/apuGenOFF")
    FF777_APU_ON = get_dataref("1-sim/ckpt/bLamps/apuGenButton")   

    if (action == "on") then
        if (FF777_GPU_1_AVAILABLE == 1 or FF777_GPU_2_AVAILABLE == 1) then
        -- Collego le GPU
            if (FF777_GPU_1_AVAILABLE == 1) then
                command_once("1-sim/command/primExtPwrButton_button")
            end
            if (FF777_GPU_2_AVAILABLE == 1) then
                command_once("1-sim/command/seconExtPwrButton_button")
            end
        else
        -- Collego i Motori
            if (FF777_ENG_1_AVAILABLE == 1) then
                command_once("1-sim/command/leftGenButton_button")
            end
            if (FF777_ENG_2_AVAILABLE == 1) then
                command_once("1-sim/command/rightGenButton_button")
            end
        end

    -- Scollego l'APU
        if (FF777_APU_ON == 1) then
            command_once("1-sim/command/apuGenButton_button")
        end
    else
        if (FF777_APU_AVAILABLE == 1) then
        -- Collego APU
            command_once("1-sim/command/apuGenButton_button")
        end        
    -- Scollego i Motori
        if (FF777_ENG_1_ON == 1) then
            command_once("1-sim/command/leftGenButton_button")
        end
        if (FF777_ENG_2_ON == 1) then
            command_once("1-sim/command/rightGenButton_button")
        end
    -- Scollego le GPU
        if (FF777_GPU_1_ON == 1) then
            command_once("1-sim/command/primExtPwrButton_button")
        end
        if (FF777_GPU_2_ON == 1) then
            command_once("1-sim/command/seconExtPwrButton_button")
        end
    end
end

function landing_light_green(gear)
    FF777_GEAR_POSITION = get_dataref("sim/aircraft/parts/acf_gear_deploy",gear)
    
    if (FF777_GEAR_POSITION == 1) then
        return 1
    else
        return 0
    end
end

function landing_light_red(gear)
    FF777_GEAR_POSITION = get_dataref("sim/aircraft/parts/acf_gear_deploy",gear)
    
    if (FF777_GEAR_POSITION > 0 and FF777_GEAR_POSITION < 1) then
        return 1
    else
        return 0
    end
end

function landing_light_off()
    FF777_GEAR_RETRACT = get_dataref("sim/aircraft/gear/acf_gear_retract")
    
    if (FF777_GEAR_RETRACT == 0) then
        return 1
    else
        return 0
    end
end

function get_adf(dataref)
    return get_dataref(dataref) * 10
end
