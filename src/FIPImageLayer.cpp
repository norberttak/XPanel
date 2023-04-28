/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <stdlib.h>
#include <iostream>
#include "Logger.h"
#include "FIPImageLayer.h"

FIPImageLayer::FIPImageLayer()
	:FIPLayer()
{
	
}

FIPImageLayer::~FIPImageLayer()
{
	
}

bool FIPImageLayer::load_file(std::string file_name, int _ref_x, int _ref_y)
{
	std::ifstream file(file_name, std::ios::binary);
	file.seekg(0, std::ios::beg);

	file.read((char*)&bmp_header, sizeof(bmp_header));

	if (bmp_header.type != 0x4d42) // the BMP magic number
	{
		Logger(logERROR) << file_name << " bad magic number in BMP header: " << bmp_header.type << std::endl;
		return false;
	}
	height = bmp_header.height_px;
	width = bmp_header.width_px;
	ref_x = _ref_x;
	ref_y = _ref_y;

	int bytes_per_pixel = bmp_header.bits_per_pixel / 8;
	if (bytes_per_pixel != 3)
	{
		Logger(logERROR) << "Bad BMP file format. It should be 24 bit color depth: " << file_name << std::endl;
		return false;
	}

	raw_buffer = (unsigned char*)calloc(width * height * bytes_per_pixel, sizeof(unsigned char));

	// BMP format requires each row to be padded at the end such that each row is represented by multiples of 4 bytes of data
	int nr_of_padding_bytes = 4 - ((width * bytes_per_pixel) % 4);
	if (nr_of_padding_bytes == 4)
		nr_of_padding_bytes = 0;

	Logger(logTRACE) << "BMP file (" << file_name << ") has " << nr_of_padding_bytes << " padding byte" << std::endl;

	for (int row = 0; row < height; row++)
	{
		file.read((char*)&raw_buffer[row * width * bytes_per_pixel], width * bytes_per_pixel);

		if (nr_of_padding_bytes != 0)
			file.seekg(nr_of_padding_bytes, std::ios::cur); // skip padding bytes

		if (!file.good())
		{
			Logger(logERROR) << "BMP file (" << file_name << "): Error occurred during read" << std::endl;
			return false;
		}
	}

	Logger(logTRACE) << file_name << " load BMP done. height=" << height << " width=" << width << std::endl;
	return true;
}