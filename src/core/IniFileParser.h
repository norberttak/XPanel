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

#ifndef _CFG_PARSER_H_
#define _CFG_PARSER_H_

#include <string>
#include <iostream>
#include <map>
#include <vector>

struct IniFileSectionHeader
{
	std::string name = "";
	std::string id = "";
	std::map<std::string, std::string> properties;
	int line = 0;
};

struct IniFileSection
{
	IniFileSectionHeader header;
	std::vector<std::pair<std::string, std::string>> key_value_pairs;
};

struct IniFile
{
	std::string file_name = "";
	std::vector<IniFileSection> sections;
};


class IniFileParser
{
public:

	IniFileParser();

	~IniFileParser();

	/**
	 * Parse a stream.
	 *
	 * @param in   the stream to parse
	 * @param file the file that is parsed, for error purposes
	 * @param line the initial line
	 **/
	void parse(std::istream& in, const std::string& file = "", unsigned int line = 1);
	IniFile& get_parsed_ini_file();
	int get_number_of_errors();
private:
	enum Token
	{
		NO_TOKEN,
		WHITESPACE,
		OPEN_BRACE,  // [
		CLOSE_BRACE, // ]
		EQUALS,      // =
		SEPARATOR, //:,
		IDENTIFIER,  // [a-zA-Z_][a-zA-Z0-9_-]*
		STRING,      // "([^"]*)"
		NUMBER,		 // (+|-)[0-9]+(\.[0-9]+)
		NEWLINE,     // \n|(\r\n)|\r
		COMMENT,	 // #[^\n\r]*
		CONFIG_FILE_END,
		PARSER_ERROR		 // this should never be returned
	};

	IniFile parsed_ini_file;

	std::istream* in;
	std::string   file;
	unsigned int  line;
	int error_count;

	Token next_token;
	std::string next_value;

	std::string section;

	void clean();

	Token get_next_token(std::string& value);

	Token lex_token(std::string& value);
	Token lex_whitespace(std::string& value);
	Token lex_newline(std::string& value);
	Token lex_number(std::string& value);
	Token lex_identifier(std::string& value);
	Token lex_string(std::string& value);
	Token lex_comment(std::string& value);

	void parse_section();

	void parse_section_header();

	void parse_value_pair();

	void error(const std::string& msg);

	IniFileParser(const IniFileParser&) = delete;
	const IniFileParser& operator = (const IniFileParser&) = delete;
};

#endif
