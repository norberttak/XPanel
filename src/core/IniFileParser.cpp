// 
// cfg - A simple configuration file library.
// 
// Copyright (c) 2015-2017 Sean Farrell <sean.farrell@rioki.org>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 

// The original code is from: https://github.com/rioki/cfg 
// I modified it a littlebit for the XPanel plugin's syntax 

#include "IniFileParser.h"
#include "Logger.h"
#include <sstream>

IniFileParser::IniFileParser()
	: in(nullptr), line(0), error_count(0), next_token(NO_TOKEN)
{

}

IniFileParser::~IniFileParser()
{
	clean();
}

IniFile& IniFileParser::get_parsed_ini_file()
{
	return parsed_ini_file;
}

int IniFileParser::get_number_of_errors()
{
	return error_count;
}

void IniFileParser::clean()
{
	parsed_ini_file.sections.clear();
	parsed_ini_file.file_name = "";
	error_count = 0;
	line = 0;
}

void IniFileParser::parse(std::istream& _in, const std::string& _file, unsigned int _line)
{
	(void)_line;
	in = &_in;
	file = _file;
	line = 1;

	clean();

	IniFileSection root_section;
	root_section.header.id = "";
	root_section.header.name = "";
	parsed_ini_file.file_name = _file;
	parsed_ini_file.sections.emplace_back(root_section);

	// prep the look ahead
	std::string dummy;
	get_next_token(dummy);

	while (next_token != CONFIG_FILE_END)
	{
		if (next_token == NEWLINE)
		{
			// skip empty lines
			get_next_token(dummy);
		}
		else if (next_token == IDENTIFIER)
		{
			parse_value_pair();
		}
		else
		{
			parse_section();
		}

	}
}

IniFileParser::Token IniFileParser::get_next_token(std::string& value)
{
	Token token = next_token;
	value = next_value;

	next_token = lex_token(next_value);
	while (next_token == WHITESPACE || next_token == COMMENT)
	{
		next_token = lex_token(next_value);
	}

	return token;
}

IniFileParser::Token IniFileParser::lex_token(std::string& value)
{
	value.clear();
	int c = in->get();
	switch (c)
	{
	case ' ': case '\t': case '\v':
		value.push_back(c);
		return lex_whitespace(value);
	case '\n': case '\r':
		value.push_back(c);
		return lex_newline(value);
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	case '-': case '+':
		value.push_back(c);
		return lex_number(value);
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
	case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
	case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
	case 's': case 't': case 'u': case 'v': case 'w': case 'x':
	case 'y': case 'z':
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
	case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
	case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
	case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
	case 'Y': case 'Z':
	case '_': case '.':
		value.push_back(c);
		return lex_identifier(value);
	case '"':
		return lex_string(value);
	case '[':
		value = "[";
		return OPEN_BRACE;
	case ']':
		value = "]";
		return CLOSE_BRACE;
	case ':':
	case ',':
		return SEPARATOR;
	case '=':
		value = "=";
		return EQUALS;
	case ';':
		return lex_comment(value);
	case EOF:
		return CONFIG_FILE_END;
	default:
		value.push_back(c);
		error("Unexpected " + value);
		return PARSER_ERROR;
	}
}

IniFileParser::Token IniFileParser::lex_whitespace(std::string& value)
{
	int c = in->get();
	while (true)
	{
		switch (c)
		{
		case ' ': case '\t': case '\v':
			value.push_back(c);
			break;
		default:
			in->unget();
			return WHITESPACE;
		}
		c = in->get();
	}
}

IniFileParser::Token IniFileParser::lex_newline(std::string& value)
{
	int c = in->get();
	switch (c)
	{
	case '\n': case '\r':
		if (c != value[0])
		{
			// \r\n or \n\r
			value.push_back(c);
		}
		else
		{
			// treat \n \n as two newline, obviously
			in->unget();
		}
		line++;
		return NEWLINE;
	default:
		in->unget();
		line++;
		return NEWLINE;
	}
}

IniFileParser::Token IniFileParser::lex_number(std::string& value)
{
	int c = in->get();
	while (true)
	{
		// NOTE: not validating the actual format
		switch (c)
		{
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
		case '.':
			value.push_back(c);
			break;
		default:
			in->unget();
			return NUMBER;
		}
		c = in->get();
	}
}

IniFileParser::Token IniFileParser::lex_identifier(std::string& value)
{
	int c = in->get();
	while (true)
	{
		switch (c)
		{
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
		case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
		case 's': case 't': case 'u': case 'v': case 'w': case 'x':
		case 'y': case 'z':
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
		case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
		case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
		case 'Y': case 'Z':
		case '_': case '-':
			value.push_back(c);
			break;
		default:
			in->unget();
			return IDENTIFIER;
		}
		c = in->get();
	}
}

IniFileParser::Token IniFileParser::lex_string(std::string& value)
{
	int c = in->get();
	while (true)
	{
		switch (c)
		{
		case '"':
			return STRING;
		case '\n': case '\r': case EOF:
			error("Unexpected newline in string");
			return PARSER_ERROR;
		case ' ': //skip every white spaces inside a string
		case '\t':
			break;
		case '\\':
			c = in->get();
			switch (c)
			{
			case '\'':
				value.push_back('\'');
				break;
			case '"':
				value.push_back('"');
				break;
			case '\\':
				value.push_back('\\');
				break;
			case 'a':
				value.push_back('\a');
				break;
			case 'b':
				value.push_back('\b');
				break;
			case 'f':
				value.push_back('\f');
				break;
			case 'n':
				value.push_back('\n');
				break;
			case 'r':
				value.push_back('\r');
				break;
			case 't':
				value.push_back('\t');
				break;
			case 'v':
				value.push_back('\v');
				break;
			default:
				error("Unknown escape sequence");
				break;
			}
			break;
		default:
			value.push_back(c);
			break;
		}
		c = in->get();
	}
}

IniFileParser::Token IniFileParser::lex_comment(std::string& value)
{
	int c = in->get();
	while (true)
	{
		switch (c)
		{
		case '\n': case '\r': case EOF:
			line++;
			return COMMENT;
		default:
			value.push_back(c);
			break;
		}
		c = in->get();
	}
}

void IniFileParser::parse_section()
{
	parse_section_header();

	while (next_token == IDENTIFIER || next_token == NEWLINE)
	{
		parse_value_pair();
	}
}

void IniFileParser::parse_section_header()
{
	std::string value;
	Token t = get_next_token(value);

	if (t != OPEN_BRACE)
	{
		error("Expected open breace");
	}

	t = get_next_token(value);
	if (t != IDENTIFIER)
	{
		error("Expected identifier");
	}
	section = value;

	IniFileSection new_section;

	while ((t = get_next_token(value)) == SEPARATOR)
	{
		t = get_next_token(value);
		if (t != IDENTIFIER)
		{
			error("Expected identifier");
		}
		std::string property_name = value;

		t = get_next_token(value);
		if (t != EQUALS)
		{
			error("Expected equals");
		}

		t = get_next_token(value);
		if (t != IDENTIFIER && t != STRING && t != NUMBER)
		{
			error("Expected identifier or string");
		}

		new_section.header.properties[property_name] = value;
	}

	if (t != CLOSE_BRACE)
	{
		error("Expected close brace");
	}

	t = get_next_token(value);
	if (t != NEWLINE && t != CONFIG_FILE_END)
	{
		error("Expected newline");
	}

	//id is the most frequent property in xpanel's config so save it on a dedicated variable as well
	if (new_section.header.properties.count("id") > 0)
		new_section.header.id = new_section.header.properties["id"];
	else
		new_section.header.id = "";

	new_section.header.name = section;
	new_section.header.line = line;
	parsed_ini_file.sections.emplace_back(new_section);
}

void IniFileParser::parse_value_pair()
{
	std::string value;
	Token t = get_next_token(value);

	if (t == NEWLINE || t == COMMENT)
	{
		// accept empty line
		return;
	}

	if (t != IDENTIFIER)
	{
		error("Expected identifier");
	}
	std::string key = value;

	t = get_next_token(value);
	if (t != EQUALS)
	{
		error("Expected equals");
	}

	t = get_next_token(value);
	if (t == STRING || t == NUMBER)
	{
		parsed_ini_file.sections.back().key_value_pairs.emplace_back(key, value);
	}
}

void IniFileParser::error(const std::string& msg)
{
	error_count++;
	Logger(TLogLevel::logERROR) << "config error: " << file << "[" << line+1 << "]: " << msg << std::endl;
}