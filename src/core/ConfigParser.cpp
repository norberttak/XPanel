/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <regex>
#include "core/ConfigParser.h"
#include "core/IniFileParser.h"
#include "core/Trigger.h"
#include "core/MultiPurposeDisplay.h"
#include "fip/FIPScreen.h"
#include "core/Logger.h"

ConfigParser::ConfigParser()
{
	process_functions[TOKEN_ON_PUSH] = &ConfigParser::handle_on_push_or_release;
	process_functions[TOKEN_ON_RELEASE] = &ConfigParser::handle_on_push_or_release;
	process_functions[TOKEN_VID] = &ConfigParser::handle_on_vid;
	process_functions[TOKEN_PID] = &ConfigParser::handle_on_pid;
	process_functions[TOKEN_LOG_LEVEL] = &ConfigParser::handle_on_log_level;
	process_functions[TOKEN_ACF] = &ConfigParser::handle_on_acf_file;
	process_functions[TOKEN_SCRIPT] = &ConfigParser::handle_on_script_file;
	process_functions[TOKEN_LIT] = &ConfigParser::handle_on_lit_or_unlit_or_blink;
	process_functions[TOKEN_UNLIT] = &ConfigParser::handle_on_lit_or_unlit_or_blink;
	process_functions[TOKEN_BLINK] = &ConfigParser::handle_on_lit_or_unlit_or_blink;
	process_functions[TOKEN_DYN_SPEED_MID] = &ConfigParser::handle_on_dynamic_speed;
	process_functions[TOKEN_DYN_SPEED_HIGH] = &ConfigParser::handle_on_dynamic_speed;
	process_functions[TOKEN_DISPLAY_LINE] = &ConfigParser::handle_on_line_add;
	process_functions[TOKEN_BCD] = &ConfigParser::handle_on_set_bcd;
	process_functions[TOKEN_ENCODER_INC] = &ConfigParser::handle_on_encoder_inc_or_dec;
	process_functions[TOKEN_ENCODER_DEC] = &ConfigParser::handle_on_encoder_inc_or_dec;
	process_functions[TOKEN_SERIAL] = &ConfigParser::handle_on_fip_serial;
	process_functions[TOKEN_FIP_OFFSET_X] = &ConfigParser::handle_on_fip_offset;
	process_functions[TOKEN_FIP_OFFSET_Y] = &ConfigParser::handle_on_fip_offset;
	process_functions[TOKEN_FIP_ROTATION] = &ConfigParser::handle_on_fip_rotation;
	process_functions[TOKEN_FIP_MASK] = &ConfigParser::handle_on_fip_mask;
	process_functions[TOKEN_FIP_TEXT] = &ConfigParser::handle_on_fip_text;
}

ConfigParser::~ConfigParser()
{
	process_functions.clear();
}

std::vector<std::string> ConfigParser::tokenize(std::string line)
{
	std::regex regex("[:|,]+");
	std::sregex_token_iterator iter(line.begin(), line.end(), regex, -1);
	std::sregex_token_iterator end;

	std::vector<std::string> tokens(iter, end);

	return tokens;
}

bool ConfigParser::get_and_remove_token_pair(std::vector<std::string>& tokens, std::string name, std::string& out_value)
{
	for (size_t i = 0; i < tokens.size(); i += 2)
	{
		if ((i+1) < tokens.size() && tokens[i] == name)
		{
			out_value = tokens[i + 1];
			tokens.erase(tokens.begin() + i, tokens.begin() + i + 2);
			return true;
		}
	}
	return false;
}

int ConfigParser::parse_file(std::string file_name, Configuration& config)
{
	last_error_message = "";
	std::ifstream input_file(file_name);
	Logger(TLogLevel::logINFO) << "parse config file: " << file_name << std::endl;
	if (!input_file.is_open()) {
		Logger(TLogLevel::logERROR) << "parser: error open config file: " << file_name << std::endl;
		return EXIT_FAILURE;
	}

	IniFileParser ini_parser;
	ini_parser.parse(input_file, file_name);
	input_file.close();

	if (ini_parser.get_number_of_errors() > 0)
		return EXIT_FAILURE;

	IniFile ini_file = ini_parser.get_parsed_ini_file();

	// Expand FIP macros (@use / [@page] / @parameter) into concrete [page]/[layer]
	// sections before the normal section processing below runs on them.
	if (expand_macros(ini_file, config) != EXIT_SUCCESS)
	{
		Logger(TLogLevel::logERROR) << "parser: error while expanding macros in config file: " << file_name << std::endl;
		return EXIT_FAILURE;
	}

	int error_count = 0;
	for (auto& ini_section : ini_file.sections)
	{
		try {
			if (process_ini_section(ini_section, config) != EXIT_SUCCESS)
				error_count++;
		}
		catch (std::exception& e)
		{
			error_count++;
			Logger(TLogLevel::logERROR) << "parser: exception happend. section starts at " << ini_section.header.line << " line, e=" << e.what() << std::endl;
		}
	}

	return (error_count == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}

int ConfigParser::process_ini_section(IniFileSection& section, Configuration& config)
{
	if (section.header.name == TOKEN_SECTION_DEVICE)
	{
		ClassConfiguration* c = new ClassConfiguration();
		config.class_configs.push_back(*c);

		if (section.header.id == DEVICE_TYPE_SAITEK_MULTI)
			config.class_configs.back().device_class_type = SAITEK_MULTI;
		else if (section.header.id == DEVICE_TYPE_SAITEK_RADIO)
			config.class_configs.back().device_class_type = SAITEK_RADIO;
		else if (section.header.id == DEVICE_TYPE_SAITEK_SWITCH)
			config.class_configs.back().device_class_type = SAITEK_SWITCH;
		else if (section.header.id == DEVICE_TYPE_ARDUINO_HOME_COCKPIT)
			config.class_configs.back().device_class_type = HOME_COCKPIT;
		else if (section.header.id == DEVICE_TYPE_SAITEK_FIP_SCREEN)
			config.class_configs.back().device_class_type = LOGITECH_FIP;
		else if (section.header.id == DEVICE_TYPE_TRC1000PFD)
			config.class_configs.back().device_class_type = TRC1000_PFD;
		else if (section.header.id == DEVICE_TYPE_TRC1000AUDIO)
			config.class_configs.back().device_class_type = TRC1000_AUDIO;
		else
		{
			last_device_id = "";
			Logger(TLogLevel::logERROR) << "parser: unknown device type: " << section.header.name << std::endl;
			return EXIT_FAILURE;
		}
		last_device_id = section.header.id;
	}
	else if (section.header.name == TOKEN_SECTION_BUTTON)
	{
		multi_mid = 1; multi_high = 1;
		speed_mid = 0; speed_high = 0;
		Logger(TLogLevel::logDEBUG) << "parser: button detected " << section.header.id << std::endl;
	}
	else if (section.header.name == TOKEN_SECTION_LIGHT)
	{
		Logger(TLogLevel::logDEBUG) << "parser: light detected " << section.header.id << std::endl;
	}
	else if (section.header.name == TOKEN_SECTION_ROT_ENCODER)
	{
		multi_mid = 1; multi_high = 1;
		speed_mid = 0; speed_high = 0;
		Logger(TLogLevel::logDEBUG) << "parser: rotation encoder detected " << section.header.id << std::endl;
	}
	else if (section.header.name == TOKEN_SECTION_DISPLAY)
	{
		config.class_configs.back().generic_displays[section.header.id] = new GenericDisplay(false);
		if (section.header.properties.count(TOKEN_BCD) > 0)
			config.class_configs.back().generic_displays[section.header.id]->set_bcd(section.header.properties[TOKEN_BCD] == "yes" ? true : false);

		if (section.header.properties.count(TOKEN_BLANK_LEADING_ZEROS) > 0)
			config.class_configs.back().generic_displays[section.header.id]->set_blank_leading_zeros(section.header.properties[TOKEN_BLANK_LEADING_ZEROS] == "yes" ? true : false);

		Logger(TLogLevel::logDEBUG) << "parser: display detected " << section.header.id << std::endl;
	}
	else if (section.header.name == TOKEN_SECTION_MULTI_DISPLAY)
	{
		config.class_configs.back().multi_displays[section.header.id] = new MultiPurposeDisplay();
		if (section.header.properties.count(TOKEN_BLANK_LEADING_ZEROS) > 0)
			config.class_configs.back().multi_displays[section.header.id]->set_blank_leading_zeros(section.header.properties[TOKEN_BLANK_LEADING_ZEROS] == "yes" ? true : false);

		Logger(TLogLevel::logDEBUG) << "parser: multi display detected " << section.header.id << std::endl;
	}
	else if (section.header.name == TOKEN_SECTION_FIP_SCREEN)
	{
		FIPScreen* screen = new FIPScreen();
		config.class_configs.back().fip_screens[last_device_id] = screen;
		Logger(TLogLevel::logDEBUG) << "parser: fip screen added: " << section.header.id << std::endl;
	}
	else if (section.header.name == TOKEN_FIP_PAGE)
	{
		config.class_configs.back().fip_screens[last_device_id]->add_page(section.header.id, false);
		Logger(TLogLevel::logDEBUG) << "parser: FIP page added " << section.header.id << std::endl;
	}
	else if (section.header.name == TOKEN_FIP_LAYER)
	{
		if (process_fip_layer_section(section, config) == EXIT_FAILURE)
			return EXIT_FAILURE;
	}
	else if (section.header.name == "")
	{
		// This is the root section of the ini tree for the plugin level options (log_level, ...)
		Logger(TLogLevel::logDEBUG) << "parser: common options detected" << std::endl;
	}
	else
	{
		Logger(TLogLevel::logERROR) << "parser: unknown section name: " << section.header.name << std::endl;
		return EXIT_FAILURE;
	}


	for (auto& key_value : section.key_value_pairs)
	{
		if (process_functions.count(key_value.first) == 0)
		{
			Logger(logERROR) << "parser: unknown key value: " << key_value.first << std::endl;
			return EXIT_FAILURE;
		}

		// call process function for the 'key':
		if ((*this.*process_functions[key_value.first])(section.header, key_value.first, key_value.second, config) != EXIT_SUCCESS)
		{
			Logger(logERROR) << "parser: error while processing key: " << key_value.first << std::endl;
			return EXIT_FAILURE;
		}

		Logger(TLogLevel::logDEBUG) << "parser: key/value processed: " << key_value.first << "-" << key_value.second << std::endl;
	}
	return EXIT_SUCCESS;
}

int ConfigParser::process_fip_layer_section(IniFileSection& section, Configuration& config)
{
	if (section.header.properties.count(TOKEN_IMAGE) > 0)
	{
		int page_index = config.class_configs.back().fip_screens[last_device_id]->get_last_page_index();
		if (page_index < 0)
		{
			Logger(logERROR) << "Parser: invalid FIP page index. section starts at " << section.header.line << std::endl;
			return EXIT_FAILURE;
		}

		std::vector<std::string> m = tokenize(section.header.properties[TOKEN_IMAGE]);

		//fip-images/Adf_Kompass_Ring.bmp,ref_x:0,ref_y:0,base_rot:0
		if (m.size() >= 7)
		{
			// Macro-expanded layers carry an explicit base dir (the macro folder) so their
			// bmp assets resolve there instead of the aircraft folder. The base dir travels
			// as a header property (not through tokenize) so a Windows path's drive-letter
			// colon can't be mistaken for a token separator.
			std::filesystem::path bmp_base_dir = section.header.properties.count(MACRO_BASE_DIR_PROPERTY) > 0
				? std::filesystem::path(section.header.properties[MACRO_BASE_DIR_PROPERTY])
				: std::filesystem::path(config.aircraft_path);
			std::filesystem::path bmp_file_absolute_path = bmp_base_dir;
			bmp_file_absolute_path /= std::string(m[0]);

			int ref_x = 0; 
			int ref_y = 0;
			int base_rot = 0;

			for (int i = 1; i <= 6; i += 2)
			{
				if (m[i] == "ref_x")
					ref_x = stoi(m[i + 1]);
				else if (m[i] == "ref_y")
					ref_y = stoi(m[i + 1]);
				else if (m[i] == "base_rot")
					base_rot = stoi(m[i + 1]);
				else
				{
					Logger(TLogLevel::logERROR) << "parse: invalid fip layer property name: " << m[i] << std::endl;
					return EXIT_FAILURE;
				}
			}

			if (config.class_configs.back().fip_screens[last_device_id]->add_layer_to_page(page_index, bmp_file_absolute_path.string(), ref_x, ref_y, base_rot) < 0)
				return EXIT_FAILURE;
		}
		else
		{
			Logger(logERROR) << "Parser: invalid image layer syntax. section starts at " << section.header.line << " " << section.header.properties[TOKEN_IMAGE] << std::endl;
			return EXIT_FAILURE;
		}
		Logger(TLogLevel::logDEBUG) << "parser: FIP image layer added " << section.header.id << std::endl;
	}
	else if (section.header.properties.count(TOKEN_TYPE) > 0 && section.header.properties[TOKEN_TYPE] == TOKEN_TEXT)
	{
		int page_index = config.class_configs.back().fip_screens[last_device_id]->get_last_page_index();
		if (page_index >= 0)
		{
			std::filesystem::path fip_fonts_bmp = config.plugin_path;
			fip_fonts_bmp /= "fip-fonts.bmp";

			if (config.class_configs.back().fip_screens[last_device_id]->add_text_layer_to_page(page_index, fip_fonts_bmp.string(), 0) < 0)
				return EXIT_FAILURE;
		}
		else
		{
			Logger(logERROR) << "Parser: invalid FIP page index. section starts at " << section.header.line << std::endl;
			return EXIT_FAILURE;
		}
	}
	else
	{
		Logger(logERROR) << "Parser: invalid layer syntax. section starts at " << section.header.line << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

// Replace every @{name} occurrence in s with its bound value. Returns false and sets
// 'missing' if a placeholder has no binding (or is malformed), leaving s partially expanded.
static bool substitute_in_string(std::string& s, const std::map<std::string, std::string>& bindings, std::string& missing)
{
	size_t pos = 0;
	while ((pos = s.find("@{", pos)) != std::string::npos)
	{
		size_t end = s.find('}', pos + 2);
		if (end == std::string::npos)
		{
			missing = s.substr(pos);
			return false;
		}
		std::string name = s.substr(pos + 2, end - (pos + 2));
		auto it = bindings.find(name);
		if (it == bindings.end())
		{
			missing = name;
			return false;
		}
		s.replace(pos, end - pos + 1, it->second);
		pos += it->second.size();
	}
	return true;
}

// Parse an @parameter value of the form "name:<n>,value:<v>" into its name and value parts.
// The value part is taken verbatim (everything after ",value:") so its own ':'/',' survive.
bool ConfigParser::parse_macro_parameter(const std::string& raw, std::string& out_name, std::string& out_value)
{
	const std::string name_prefix = "name:";
	const std::string value_delim = ",value:";

	if (raw.rfind(name_prefix, 0) != 0)
		return false;

	size_t vpos = raw.find(value_delim);
	if (vpos == std::string::npos || vpos < name_prefix.size())
		return false;

	out_name = raw.substr(name_prefix.size(), vpos - name_prefix.size());
	out_value = raw.substr(vpos + value_delim.size());

	return !out_name.empty() && !out_value.empty();
}

int ConfigParser::substitute_placeholders_in_section(IniFileSection& section,
	const std::map<std::string, std::string>& bindings,
	const std::string& macro_name, const std::string& page_id)
{
	std::string missing;
	for (auto& key_value : section.key_value_pairs)
	{
		if (!substitute_in_string(key_value.second, bindings, missing))
		{
			Logger(logERROR) << "parser: macro '" << macro_name << "' page '" << page_id << "': unbound parameter '" << missing << "'" << std::endl;
			return EXIT_FAILURE;
		}
	}
	for (auto& prop : section.header.properties)
	{
		if (!substitute_in_string(prop.second, bindings, missing))
		{
			Logger(logERROR) << "parser: macro '" << macro_name << "' page '" << page_id << "': unbound parameter '" << missing << "'" << std::endl;
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}

// Load <plugin>/macros/<macro_name>.macro, and for each [page] it defines emit a cloned
// [page] section followed by its [layer] sections with @{...} placeholders substituted from
// the matching [@page] binding block. The concrete sections are appended to 'output'.
int ConfigParser::load_and_expand_macro(const std::string& macro_name,
	const std::map<std::string, std::map<std::string, std::string>>& page_bindings,
	Configuration& config, std::vector<IniFileSection>& output)
{
	std::filesystem::path macro_dir = std::filesystem::path(config.plugin_path) / "macros";
	std::filesystem::path macro_file = macro_dir / (macro_name + ".macro");

	std::ifstream input_file(macro_file);
	if (!input_file.is_open())
	{
		Logger(logERROR) << "parser: cannot open macro file: " << macro_file.string() << std::endl;
		return EXIT_FAILURE;
	}

	IniFileParser macro_parser;
	macro_parser.parse(input_file, macro_file.string());
	input_file.close();

	if (macro_parser.get_number_of_errors() > 0)
	{
		Logger(logERROR) << "parser: error parsing macro file: " << macro_file.string() << std::endl;
		return EXIT_FAILURE;
	}

	IniFile macro_ini = macro_parser.get_parsed_ini_file();

	std::string current_page_id = "";
	bool have_page = false;
	const std::map<std::string, std::string> empty_bindings;
	const std::map<std::string, std::string>* active_bindings = &empty_bindings;

	for (auto& section : macro_ini.sections)
	{
		if (section.header.name == TOKEN_SECTION_MACRO_INFO)
		{
			for (auto& key_value : section.key_value_pairs)
			{
				if (key_value.first == TOKEN_MACRO_DEVICE && key_value.second != "fip")
				{
					Logger(logERROR) << "parser: macro '" << macro_name << "' targets device '" << key_value.second << "'. Only 'fip' macros are supported" << std::endl;
					return EXIT_FAILURE;
				}
			}
		}
		else if (section.header.name == TOKEN_FIP_PAGE)
		{
			current_page_id = section.header.id;
			have_page = true;
			auto it = page_bindings.find(current_page_id);
			active_bindings = (it != page_bindings.end()) ? &it->second : &empty_bindings;

			IniFileSection page_section = section;
			if (substitute_placeholders_in_section(page_section, *active_bindings, macro_name, current_page_id) != EXIT_SUCCESS)
				return EXIT_FAILURE;
			output.push_back(page_section);
		}
		else if (section.header.name == TOKEN_FIP_LAYER)
		{
			if (!have_page)
			{
				Logger(logERROR) << "parser: macro '" << macro_name << "' has a [layer] before any [page]" << std::endl;
				return EXIT_FAILURE;
			}

			IniFileSection layer_section = section;
			if (substitute_placeholders_in_section(layer_section, *active_bindings, macro_name, current_page_id) != EXIT_SUCCESS)
				return EXIT_FAILURE;

			// tell process_fip_layer_section to resolve this layer's bmp against the macro folder
			layer_section.header.properties[MACRO_BASE_DIR_PROPERTY] = macro_dir.string();
			output.push_back(layer_section);
		}
		else if (section.header.name == "")
		{
			// synthetic root section of the macro file - nothing to emit
			continue;
		}
		else
		{
			Logger(logERROR) << "parser: unsupported section '" << section.header.name << "' in macro '" << macro_name << "'" << std::endl;
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

// Rewrite ini_file.sections in place: expand any '@use' referenced macro inside a [screen]
// section (bound by the following [@page]/@parameter blocks) into concrete [page]/[layer]
// sections. Non-macro sections pass through unchanged.
int ConfigParser::expand_macros(IniFile& ini_file, Configuration& config)
{
	std::vector<IniFileSection> output;
	std::vector<IniFileSection>& sections = ini_file.sections;

	for (size_t i = 0; i < sections.size(); )
	{
		IniFileSection& section = sections[i];

		if (section.header.name == TOKEN_SECTION_MACRO_PAGE)
		{
			Logger(logERROR) << "parser: [@page:...] binding block without a preceding [screen] that uses a macro. section at line " << section.header.line << std::endl;
			return EXIT_FAILURE;
		}

		if (section.header.name == TOKEN_SECTION_FIP_SCREEN)
		{
			// pull the @use macro names out of the screen section, keep the rest
			std::vector<std::string> macro_names;
			std::vector<std::pair<std::string, std::string>> kept_key_values;
			for (auto& key_value : section.key_value_pairs)
			{
				if (key_value.first == TOKEN_MACRO_USE)
					macro_names.push_back(key_value.second);
				else
					kept_key_values.push_back(key_value);
			}
			section.key_value_pairs = kept_key_values;
			output.push_back(section);
			i++;

			if (macro_names.empty())
				continue;

			// gather the following [@page:...] binding blocks: page id -> (param name -> value)
			std::map<std::string, std::map<std::string, std::string>> page_bindings;
			while (i < sections.size() && sections[i].header.name == TOKEN_SECTION_MACRO_PAGE)
			{
				IniFileSection& binding = sections[i];
				std::map<std::string, std::string> params;
				for (auto& key_value : binding.key_value_pairs)
				{
					if (key_value.first != TOKEN_MACRO_PARAMETER)
					{
						Logger(logERROR) << "parser: unexpected key '" << key_value.first << "' in [@page] binding block at line " << binding.header.line << std::endl;
						return EXIT_FAILURE;
					}
					std::string param_name, param_value;
					if (!parse_macro_parameter(key_value.second, param_name, param_value))
					{
						Logger(logERROR) << "parser: invalid @parameter syntax at line " << binding.header.line << ": " << key_value.second << std::endl;
						return EXIT_FAILURE;
					}
					params[param_name] = param_value;
				}
				page_bindings[binding.header.id] = params;
				i++;
			}

			for (auto& macro_name : macro_names)
			{
				if (load_and_expand_macro(macro_name, page_bindings, config, output) != EXIT_SUCCESS)
					return EXIT_FAILURE;
			}
			continue;
		}

		output.push_back(section);
		i++;
	}

	sections = output;
	return EXIT_SUCCESS;
}

int ConfigParser::handle_on_vid(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config)
{
	(void)section_header;
	(void)key;
	std::stringstream ss;
	ss << std::hex << value;
	ss >> config.class_configs.back().vid;
	Logger(TLogLevel::logDEBUG) << "parser: vid=" << config.class_configs.back().vid << std::endl;

	return EXIT_SUCCESS;
}

int ConfigParser::handle_on_pid(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config)
{
	(void)section_header;
	(void)key;
	std::stringstream ss;
	ss << std::hex << value;
	ss >> config.class_configs.back().pid;
	Logger(TLogLevel::logDEBUG) << "parser: pid=" << config.class_configs.back().pid << std::endl;

	return EXIT_SUCCESS;
}

int ConfigParser::handle_on_log_level(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config)
{
	(void)key;
	(void)config;
	if (section_header.id != "")
		Logger(TLogLevel::logWARNING) << "Log level shall be defined in the common part of the config file!" << std::endl;

	TLogLevel level;
	if (value == "DEBUG")
		level = TLogLevel::logDEBUG;
	else if (value == "INFO")
		level = TLogLevel::logINFO;
	else if (value == "WARNING")
		level = TLogLevel::logWARNING;
	else if (value == "ERROR")
		level = TLogLevel::logERROR;
	else if (value == "TRACE")
		level = TLogLevel::logTRACE;
	else {
		Logger(TLogLevel::logERROR) << "parser: unknown log level: " << value << std::endl;
		return EXIT_FAILURE;
	}

	Logger::set_log_level(level);
	Logger(TLogLevel::logDEBUG) << "parser: log level set to " << level << std::endl;

	return EXIT_SUCCESS;
}

int ConfigParser::handle_on_acf_file(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config)
{
	(void)key;
	if (section_header.id != "")
		Logger(TLogLevel::logWARNING) << "ACF file shall be defined in the common part of the config file!" << std::endl;

	config.aircraft_acf = value;
	Logger(TLogLevel::logDEBUG) << "parser: ACF file=" << config.aircraft_acf << std::endl;

	return EXIT_SUCCESS;
}

int ConfigParser::handle_on_script_file(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config)
{
	(void)key;
	if (section_header.id != "")
		Logger(TLogLevel::logWARNING) << "Script shall be defined in the common part of the config file!" << std::endl;

	config.script_file = value;
	Logger(TLogLevel::logDEBUG) << "parser: script file=" << config.script_file << std::endl;
	return EXIT_SUCCESS;
}

/* Find the array index [] in the string. If index found the index part will be removed
   from teh string and the parsed index is set in the index variable.
   If no index found index will be set to -1
*/
void ConfigParser::check_and_get_array_index(std::string& dataref, int& index)
{
	std::cmatch m;
	if (std::regex_match(dataref.c_str(), m, std::regex("(.+)\\[([0-9]+)\\]")))
	{
		dataref = m[1];
		index = atoi(m[2].str().c_str());
	}
	else
	{
		index = -1;
	}
}

XPLMDataTypeID ConfigParser::normalize_dataref_type(XPLMDataTypeID data_ref_type)
{
	/* from xplane documenation:
	   Data types each take a bit field; it is legal to have a single dataref 
	   be more than one type of data. When this happens, you can pick 
	   any matching get/set API.
	*/
	if (data_ref_type & xplmType_Double)
		return xplmType_Double;
	else if (data_ref_type & xplmType_Float)
		return xplmType_Float;
	else if (data_ref_type & xplmType_FloatArray)
		return xplmType_FloatArray;
	else if (data_ref_type & xplmType_IntArray)
		return xplmType_IntArray;
	else
		return data_ref_type;
}

int ConfigParser::handle_on_push_or_release(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config)
{
	Action* action;

	std::vector<std::string> m = tokenize(value);

	//on_push="on_select:SW_DOWN_COM1,dataref:sim/cockpit2/radios/actuators/com1_frequency_hz_833:-5:0:160000"
	//on_push="on_select:SW_UP_ADF,lua:radio_transfer('ADF1')"
	//on_push="commandref:sim/flight_controls/flaps_up:once"
	//on_release="dataref:B742/AP_panel/AT_on_sw:0"
	//on_push = "lua:button_AP('push')"
	//on_push="dataref:/sim/data/data_array[0]:1"

	if (m.size() < 2)
	{
		Logger(TLogLevel::logERROR) << "parser: invalid syntax (section starts at line: " << section_header.line << "): " << value << std::endl;
		return EXIT_FAILURE;
	}

	//save condition (on_select) for further use
	std::string condition = "";
	if (m[0] == TOKEN_ON_SELECT)
	{
		condition = m[1];
		m.erase(m.begin(), m.begin() + 2);
	}

	if (m.size() < 2)
	{
		Logger(TLogLevel::logERROR) << "parser: invalid syntax (section starts at line: " << section_header.line << "): " << value << std::endl;
		return EXIT_FAILURE;
	}

	if (m[0] == TOKEN_DATAREF)
	{
		if (m.size() < 3)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid syntax (section starts at line: " << section_header.line << "): " << value << std::endl;
			return EXIT_FAILURE;
		}

		int index;
		check_and_get_array_index(m[1], index);
		XPLMDataRef dataRef = XPLMFindDataRef(m[1].c_str());

		if (dataRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid data ref (section starts at line: " << section_header.line << "): " << value << std::endl;
			return EXIT_FAILURE;
		}

		if (m.size() >= 5)
		{
			// dataref change with (delta,min,max):
			action = new Action(dataRef, stof(m[2]), stof(m[4]), stof(m[3]));
		}
		else
		{
			switch (normalize_dataref_type(XPLMGetDataRefTypes(dataRef))) {
			case xplmType_IntArray:
				if (index < 0)
				{
					Logger(TLogLevel::logERROR) << "parser: invalid data ref array index (section starts at line: " << section_header.line << "): " << index << std::endl;
					return EXIT_FAILURE;
				}
				action = new Action(dataRef, index, stoi(m[2]));
				break;
			case xplmType_FloatArray:
				if (index < 0)
				{
					Logger(TLogLevel::logERROR) << "parser: invalid data ref array index (section starts at line: " << section_header.line << "): " << index << std::endl;
					return EXIT_FAILURE;
				}
				action = new Action(dataRef, index, stof(m[2]));
				break;
			case xplmType_Int:
				action = new Action(dataRef, stoi(m[2]));
				break;
			case xplmType_Float:
				action = new Action(dataRef, stof(m[2]));
				break;
			case xplmType_Double:
				action = new Action(dataRef, stod(m[2]));
				break;
			default:
				Logger(TLogLevel::logERROR) << "parser: invalid data ref type (section starts at line: " << section_header.line << "): " << value << std::endl;
				return EXIT_FAILURE;
			}
		}

		Logger(TLogLevel::logDEBUG) << "parser: button push/release dataref " << m[1] << std::endl;
	}
	else if (m[0] == TOKEN_COMMANDREF)
	{
		XPLMCommandRef commandRef = XPLMFindCommand(m[1].c_str());
		if (commandRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid command ref (section starts at " << section_header.line << "): " << value << std::endl;
			return EXIT_FAILURE;
		}
		CommandType command_type = CommandType::ONCE;

		if (m.size() >= 3)
		{
			if (m[2] == TOKEN_BEGIN)
				command_type = CommandType::BEGIN;
			else if (m[2] == TOKEN_END)
				command_type = CommandType::END;
			else
				command_type = CommandType::ONCE;
		}

		action = new Action(commandRef, command_type);
	}
	else if (m[0] == TOKEN_LUA)
	{
		action = new Action(m[1]);
		Logger(TLogLevel::logDEBUG) << "parser: button push/release lua string " << m[1] << std::endl;
	}
	else
	{
		Logger(TLogLevel::logERROR) << "parser: invalid action ref (section starts at " << section_header.line << "): " << value << std::endl;
		return EXIT_FAILURE;
	}

	//if saved any condition before, we add it to the action
	if (condition != "")
		action->add_condition(condition);

	action->set_dynamic_speed_params(speed_mid, multi_mid, speed_high, multi_high);

	if (key == TOKEN_ON_PUSH)
		config.class_configs.back().push_actions[section_header.id].push_back(action);
	else if (key == TOKEN_ON_RELEASE)
		config.class_configs.back().release_actions[section_header.id].push_back(action);
	else
	{
		Logger(TLogLevel::logERROR) << "parser: invalid key (section starts at line: " << section_header.line << "): " << key << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int ConfigParser::handle_on_dynamic_speed(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config)
{
	(void)section_header;
	(void)config;
	//dynamic_speed_mid="2tick/sec:2x"
	//dynamic_speed_high = "6tick/sec:4x"

	std::cmatch m;
	if (std::regex_match(value.c_str(), m, std::regex("([+-]*[0-9\\.]+)tick\\/sec:([+-]*[0-9]+)x")))
	{

		if (key == TOKEN_DYN_SPEED_MID) {
			speed_mid = stof(m[1]);
			multi_mid = stoi(m[2]);
		}
		else
		{
			speed_high = stof(m[1]);
			multi_high = stoi(m[2]);
		}

		Logger(TLogLevel::logDEBUG) << "parser: dynamic speed config: " << key << ":" << m[1] << "x" << m[2] << std::endl;
		return EXIT_SUCCESS;
	}
	else
	{
		Logger(TLogLevel::logERROR) << "parser: invalid dynamic speed syntax: " << value << std::endl;
		return EXIT_FAILURE;
	}
}

int ConfigParser::handle_on_lit_or_unlit_or_blink(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config)
{
	//trigger_unlit="dataref:sim/custom/lights/button/absu_zk:0"
	//trigger_lit="lua:get_led_status():1"

	TriggerType trigger_type;
	if (key == TOKEN_LIT)
		trigger_type = TriggerType::LIT;
	else if (key == TOKEN_UNLIT)
		trigger_type = TriggerType::UNLIT;
	else if (key == TOKEN_BLINK)
		trigger_type = TriggerType::BLINK;
	else
	{
		Logger(TLogLevel::logERROR) << "parser: invalid key value: " << key << std::endl;
		return EXIT_FAILURE;
	}

	Trigger* trigger;

	std::vector<std::string> m = tokenize(value);
	if (m.size() < 3)
	{
		Logger(TLogLevel::logERROR) << "parser: invalid syntax (section starts at line: " << section_header.line << "): " << value << std::endl;
		return EXIT_FAILURE;
	}

	if (m[0] == TOKEN_DATAREF)
	{
		XPLMDataRef dataRef = XPLMFindDataRef(m[1].c_str());
		if (dataRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid data ref (section starts at line: " << section_header.line << "): " << value << std::endl;
			return EXIT_FAILURE;
		}

		trigger = new Trigger(dataRef, stod(m[2]), trigger_type);
		Logger(TLogLevel::logDEBUG) << "parser: light lit/unlit/blink dataref " << m[1] << std::endl;
	}
	else if (m[0] == TOKEN_LUA)
	{
		trigger = new Trigger(m[1], stod(m[2]), trigger_type);
		Logger(TLogLevel::logDEBUG) << "parser: light lit/unlit/blink lua " << m[1] << std::endl;
	}
	else
	{
		Logger(TLogLevel::logERROR) << "parser: invalid action ref (section starts at line: " << section_header.line << "): " << value << std::endl;
		return EXIT_FAILURE;
	}

	config.class_configs.back().light_triggers[section_header.id].push_back(trigger);
	return EXIT_SUCCESS;
}

int ConfigParser::handle_on_line_add(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config)
{
	(void)key;
	//line="on_select:SW_ALT,dataref:sim/custom/gauges/compas/pkp_helper_course_L"
	//line="on_select:SW_ALT,dataref:sim/custom/gauges/compas/pkp_helper_course_L, dot_position: 2" 
	//line="on_select:SW_ALT,dataref:sim/custom/gauges/compas/pkp_helper_course_L, minimum_digit_number: 3"
	//line="dataref:sim/custom/gauges/compas/pkp_helper_course_L"
	//line="dataref:sim/custom/gauges/compas/test[0] 
	//line="const:1.5" 
	//line="lua:test()"

	std::vector<std::string> m = tokenize(value);

	if (m.size() < 2)
	{
		Logger(TLogLevel::logERROR) << "parser: invalid syntax (section starts at line: " << section_header.line << "): " << value << std::endl;
		return EXIT_FAILURE;
	}

	std::string condition = "";
	get_and_remove_token_pair(m, TOKEN_ON_SELECT, condition);

	std::string min_digit_number = "";
	get_and_remove_token_pair(m, TOKEN_MIN_DIGIT_NUMBER, min_digit_number);
	if (min_digit_number != "")
	{
		if (section_header.name == TOKEN_SECTION_MULTI_DISPLAY)
			config.class_configs.back().multi_displays[section_header.id]->set_minimum_number_of_digits(condition, stoi(min_digit_number));
		else if(section_header.name == TOKEN_SECTION_DISPLAY)
			config.class_configs.back().generic_displays[section_header.id]->set_minimum_number_of_digits(stoi(min_digit_number));
		else
		{
			Logger(TLogLevel::logERROR) << "parser: invalid line for device type: " << section_header.name << std::endl;
			return EXIT_FAILURE;
		}
	}

	std::string dot_position = "";
	get_and_remove_token_pair(m, TOKEN_DOT_POSITION, dot_position);
	if (dot_position != "")
	{
		if (section_header.name == TOKEN_SECTION_MULTI_DISPLAY)
			config.class_configs.back().multi_displays[section_header.id]->set_dot_position(condition, stoi(dot_position));
		else if (section_header.name == TOKEN_SECTION_DISPLAY)
			config.class_configs.back().generic_displays[section_header.id]->set_dot_position(stoi(dot_position));
		else
		{
			Logger(TLogLevel::logERROR) << "parser: invalid line for device type: " << section_header.name << std::endl;
			return EXIT_FAILURE;
		}
	}

	// at this point only the dataref/lua/const key-value pair shall remain
	if (m.size() != 2)
	{
		Logger(TLogLevel::logERROR) << "parser: invalid syntax (section starts at line: " << section_header.line << "): " << value << std::endl;
		return EXIT_FAILURE;
	}

	if (m[0] == TOKEN_DATAREF)
	{
		int index;
		check_and_get_array_index(m[1], index);

		XPLMDataRef dataRef = XPLMFindDataRef(m[1].c_str());
		if (dataRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid data ref (section starts at line: " << section_header.line << "): " << value << std::endl;
			return EXIT_FAILURE;
		}
		if (section_header.name == TOKEN_SECTION_DISPLAY)
		{
			if (index >= 0)
				config.class_configs.back().generic_displays[section_header.id]->add_dataref(dataRef, index);
			else
				config.class_configs.back().generic_displays[section_header.id]->add_dataref(dataRef);
		}
		else if (section_header.name == TOKEN_SECTION_MULTI_DISPLAY)
		{
			if (condition != "")
				config.class_configs.back().multi_displays[section_header.id]->add_condition(condition, dataRef);
			else
				config.class_configs.back().multi_displays[section_header.id]->add_dataref(dataRef);
		}
		else
		{
			Logger(TLogLevel::logERROR) << "parser: invalid line for device type: " << section_header.name << std::endl;
			return EXIT_FAILURE;
		}

		Logger(TLogLevel::logDEBUG) << "parser: generic display add line " << section_header.id << " " << m[1] << std::endl;
		return EXIT_SUCCESS;
	}
	else if (m[0] == TOKEN_LUA)
	{
		if (section_header.name == TOKEN_SECTION_DISPLAY)
		{
			config.class_configs.back().generic_displays[section_header.id]->add_lua(m[1]);
		}
		else if (section_header.name == TOKEN_SECTION_MULTI_DISPLAY)
		{
			if (condition != "")
				config.class_configs.back().multi_displays[section_header.id]->add_condition(condition, m[1]);
			else
				config.class_configs.back().multi_displays[section_header.id]->add_lua(m[1]);
		}
		else
		{
			Logger(TLogLevel::logERROR) << "parser: invalid line for device type: " << section_header.name << std::endl;
			return EXIT_FAILURE;
		}
		Logger(TLogLevel::logDEBUG) << "parser: generic display add line " << section_header.id << " " << m[1] << std::endl;
		return EXIT_SUCCESS;
	}
	else if (m[0] == TOKEN_CONST)
	{
		if (section_header.name == TOKEN_SECTION_DISPLAY)
		{
			config.class_configs.back().generic_displays[section_header.id]->add_const(stod(m[1]));
		}
		else if (section_header.name == TOKEN_SECTION_MULTI_DISPLAY)
		{
			if (condition != "")
				config.class_configs.back().multi_displays[section_header.id]->add_condition(condition, stod(m[1]));
			else
				config.class_configs.back().multi_displays[section_header.id]->add_const(stod(m[1]));
		}
		else
		{
			Logger(TLogLevel::logERROR) << "parser: invalid line for device type: " << section_header.name << std::endl;
			return EXIT_FAILURE;
		}

		Logger(TLogLevel::logDEBUG) << "parser: generic display add line " << section_header.id << " " << m[1] << std::endl;
		return EXIT_SUCCESS;
	}

	return EXIT_FAILURE;
}

int ConfigParser::handle_on_set_bcd(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config)
{
	(void)key;
	if (section_header.name == TOKEN_SECTION_MULTI_DISPLAY)
		config.class_configs.back().multi_displays[section_header.id]->set_bcd(value == "yes" ? true : false);
	else if (section_header.name == TOKEN_SECTION_DISPLAY)
		config.class_configs.back().generic_displays[section_header.id]->set_bcd(value == "yes" ? true : false);
	else
	{
		Logger(TLogLevel::logERROR) << "parser: bcd option can be set for generic- or multidisplay items";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int ConfigParser::handle_on_encoder_inc_or_dec(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config)
{
	//on_increment="commandref:sim/GPS/g1000n1_nav_inner_up:once"
	//on_decrement = "commandref:sim/GPS/g1000n1_nav_inner_down:once"

	if (section_header.name != TOKEN_SECTION_ROT_ENCODER)
	{
		Logger(TLogLevel::logERROR) << "parser: " << key << " is only valid for encoders" << std::endl;
		return EXIT_FAILURE;
	}

	std::vector<std::string> m = tokenize(value);

	if (m.size() < 2)
	{
		Logger(TLogLevel::logERROR) << "parser: invalid syntax (section starts at line: " << section_header.line << "): " << value << std::endl;
		return EXIT_FAILURE;
	}

	std::string condition = "";
	if (m[0] == TOKEN_ON_SELECT)
	{
		condition = m[1];
		m.erase(m.begin(), m.begin() + 2);
	}

	if (m.size() < 2)
	{
		Logger(TLogLevel::logERROR) << "parser: invalid syntax (section starts at line: " << section_header.line << "): " << value << std::endl;
		return EXIT_FAILURE;
	}

	Action* action;

	if (m[0] == TOKEN_COMMANDREF)
	{
		XPLMCommandRef commandRef = XPLMFindCommand(m[1].c_str());
		if (commandRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid command ref (section starts at line: " << section_header.line << "): " << value << std::endl;
			return EXIT_FAILURE;
		}
		CommandType command_type = CommandType::ONCE;

		if (m.size() >= 3)
		{
			if (m[2] == TOKEN_BEGIN)
				command_type = CommandType::BEGIN;
			else if (m[2] == TOKEN_END)
				command_type = CommandType::END;
			else
				command_type = CommandType::ONCE;
		}

		action = new Action(commandRef, command_type);
		Logger(TLogLevel::logDEBUG) << "parser: rot encoder inc/dec command " << m[1] << std::endl;
	}
	else if (m[0] == TOKEN_DATAREF)
	{
		XPLMDataRef dataRef = XPLMFindDataRef(m[1].c_str());
		if (dataRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid data ref (section starts at line: " << section_header.line << "): " << value << std::endl;
			return EXIT_FAILURE;
		}
		action = new Action(dataRef, stof(m[2]), stof(m[4]), stof(m[3]));
		Logger(TLogLevel::logDEBUG) << "parser: encoder inc/dec dataref " << m[1] << std::endl;

	}
	else if (m[0] == TOKEN_LUA)
	{
		action = new Action(m[1]);
		Logger(TLogLevel::logDEBUG) << "parser: rot encoder inc lua string " << m[1] << std::endl;
	}
	else
	{
		Logger(TLogLevel::logERROR) << "parser: invalid action ref (section starts at line: " << section_header.line << "): " << value << std::endl;
		return EXIT_FAILURE;
	}

	action->set_dynamic_speed_params(speed_mid, multi_mid, speed_high, multi_high);

	if (condition != "")
		action->add_condition(condition);

	if (key == TOKEN_ENCODER_INC)
		config.class_configs.back().encoder_inc_actions[section_header.id].push_back(action);
	else
		config.class_configs.back().encoder_dec_actions[section_header.id].push_back(action);

	return EXIT_SUCCESS;
}

int ConfigParser::handle_on_fip_serial(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config)
{
	(void)section_header;
	(void)key;
	config.class_configs.back().serial_number = value;
	Logger(TLogLevel::logDEBUG) << "parser: serial number: " << value << std::endl;
	return EXIT_SUCCESS;
}

int ConfigParser::handle_on_fip_offset(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config)
{
	//offset_x="const:80"
	//offset_y = "dataref::sim/cockpit2/radios/actuators/com1_frequency_hz"
	//offset_y = "dataref::sim/cockpit2/radios/actuators/com1_frequency_hz[0],scale:1.0"

	ScreenAction* action = new ScreenAction();
	action->page_index = config.class_configs.back().fip_screens[last_device_id]->get_last_page_index();
	action->layer_index = config.class_configs.back().fip_screens[last_device_id]->get_last_layer_index(action->page_index);

	if (key == TOKEN_FIP_OFFSET_X)
		action->type = SC_TRANSLATION_X;
	else
		action->type = SC_TRANSLATION_Y;

	std::vector<std::string> m = tokenize(value);

	if (m.size() < 2)
	{
		Logger(TLogLevel::logERROR) << "parser: invalid syntax (section starts at line: " << section_header.line << "): " << value << std::endl;
		return EXIT_FAILURE;
	}

	if (m[0] == TOKEN_CONST)
	{
		action->constant_val = stoi(m[1]);
	}
	else if (m[0] == TOKEN_DATAREF)
	{
		int index = -1;
		check_and_get_array_index(m[1], index);

		if (m.size() >= 3)
			action->scale_factor = std::stod(m[3]);

		XPLMDataRef dataRef = XPLMFindDataRef(m[1].c_str());
		if (dataRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid data ref (section starts at line: " << section_header.line << "): " << m[1] << std::endl;
			return EXIT_FAILURE;
		}

		if (index >= 0)
			action->data_ref_index = index;

		action->data_ref = dataRef;
		action->data_ref_type = normalize_dataref_type(XPLMGetDataRefTypes(dataRef));
	}
	else if (m[0] == TOKEN_LUA)
	{
		action->lua_str = m[1];
	}
	else
	{
		Logger(TLogLevel::logERROR) << "parser: invalid action ref (section starts at line: " << section_header.line << "): " << value << std::endl;
		delete action;
		return EXIT_FAILURE;
	}

	Logger(logTRACE) << "config parser: add FIP offset const: " << action->constant_val << std::endl;

	config.class_configs.back().fip_screens[last_device_id]->add_screen_action(action);
	return EXIT_SUCCESS;
}

int ConfigParser::handle_on_fip_rotation(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config)
{
	(void)key;
	ScreenAction* action = new ScreenAction();
	action->page_index = config.class_configs.back().fip_screens[last_device_id]->get_last_page_index();
	action->layer_index = config.class_configs.back().fip_screens[last_device_id]->get_last_layer_index(action->page_index);
	action->type = SC_ROTATION;

	std::vector<std::string> m = tokenize(value);

	if (m[0] == TOKEN_CONST)
	{
		action->constant_val = stoi(m[1]);
		Logger(logTRACE) << "config parser: add FIP rotation const: " << action->constant_val << std::endl;
	}
	else if (m[0] == TOKEN_DATAREF)
	{
		int index = -1;
		check_and_get_array_index(m[1], index);

		action->scale_factor = std::stod(m[3]);
		XPLMDataRef dataRef = XPLMFindDataRef(m[1].c_str());
		if (dataRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid data ref (section starts at line: " << section_header.line << "): " << value << std::endl;
			return EXIT_FAILURE;
		}

		if (index >= 0)
			action->data_ref_index = index;

		action->data_ref = dataRef;
		action->data_ref_type = normalize_dataref_type(XPLMGetDataRefTypes(dataRef));
	}
	else if (m[0] == TOKEN_LUA)
	{
		action->lua_str = m[1];
	}
	else
	{
		Logger(TLogLevel::logERROR) << "parser: invalid action ref (section starts at line: " << section_header.line << "): " << value << std::endl;
		delete action;
		return EXIT_FAILURE;
	}

	config.class_configs.back().fip_screens[last_device_id]->add_screen_action(action);
	return EXIT_SUCCESS;
}

int ConfigParser::handle_on_fip_mask(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config)
{
	(void)section_header;
	(void)key;
	int page_index = config.class_configs.back().fip_screens[last_device_id]->get_last_page_index();
	int layer_index = config.class_configs.back().fip_screens[last_device_id]->get_last_layer_index(page_index);

	std::vector<std::string> m = tokenize(value);

	if (m.size() >= 8 && page_index >= 0 && layer_index >= 0)
	{
		//"screen_x:0,screen_y:120,height:100,width:60"
		int screen_x, screen_y, height, width;
		for (int i = 0; i < 8; i += 2)
		{
			if (m[i] == "screen_x")
				screen_x = stoi(m[1]);
			else if (m[i] == "screen_y")
				screen_y = stoi(m[i + 1]);
			else if (m[i] == "height")
				height = stoi(m[i + 1]);
			else if (m[i] == "width")
				width = stoi(m[i + 1]);
			else
			{
				Logger(logERROR) << "parser: invalid key name: " << m[i] << std::endl;
				return EXIT_FAILURE;
			}
		}

		MaskWindow mask(screen_x, screen_y, width, height);
		config.class_configs.back().fip_screens[last_device_id]->set_mask(page_index, layer_index, mask);
		Logger(logTRACE) << "Parser: set mask for page " << page_index << " / layer " << layer_index << std::endl;
	}
	else
	{
		Logger(logERROR) << "Parser: invalid FIP page or layer index" << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int ConfigParser::handle_on_fip_text(IniFileSectionHeader section_header, std::string key, std::string value, Configuration& config)
{
	(void)key;
	int page_index = config.class_configs.back().fip_screens[last_device_id]->get_last_page_index();
	int layer_index = config.class_configs.back().fip_screens[last_device_id]->get_last_layer_index(page_index);

	if (page_index < 0 || layer_index < 0)
	{
		Logger(logERROR) << "Parser: invalid FIP page or layer index. section starts at " << section_header.line << std::endl;
		return EXIT_FAILURE;
	}

	std::vector<std::string> m = tokenize(value);

	if (m[0] == TOKEN_CONST)
	{
		config.class_configs.back().fip_screens[last_device_id]->set_text(page_index, layer_index, m[1]);
	}
	else if (m[0] == TOKEN_DATAREF)
	{
		ScreenAction* action = new ScreenAction();

		action->page_index = page_index;
		action->layer_index = layer_index;

		XPLMDataRef dataRef = XPLMFindDataRef(m[1].c_str());
		if (dataRef == NULL)
		{
			Logger(TLogLevel::logERROR) << "parser: invalid data ref (section starts at line: " << section_header.line << "): " << value << std::endl;
			return EXIT_FAILURE;
		}
		action->data_ref = dataRef;
		action->data_ref_type = normalize_dataref_type(XPLMGetDataRefTypes(dataRef));
		action->type = SC_SET_TEXT;

		Logger(logTRACE) << "config parser: add FIP text set dataref: " << action->data_ref << std::endl;
		config.class_configs.back().fip_screens[last_device_id]->add_screen_action(action);
	}
	else if (m[0] == TOKEN_LUA)
	{
		ScreenAction* action = new ScreenAction();

		action->page_index = page_index;
		action->layer_index = layer_index;
		action->lua_str = m[1];
		action->type = SC_SET_TEXT;

		Logger(logTRACE) << "config parser: add FIP set text lua: " << action->lua_str << std::endl;
		config.class_configs.back().fip_screens[last_device_id]->add_screen_action(action);
	}
	else
	{
		Logger(TLogLevel::logERROR) << "parser: invalid action ref (section starts at line: " << section_header.line << "): " << value << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}