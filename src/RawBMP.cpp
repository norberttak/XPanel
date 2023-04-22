/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <stdlib.h>
#include <iostream>
#include "Logger.h"
#include "RawBMP.h"

RawBMP::RawBMP()
{
	width = 0;
	height = 0;
	ref_x = 0;
	ref_y = 0;
	pos_x = 0;
	pos_y = 0;
	angle = 0;
	base_rot = 0;
	raw_buffer = NULL;
	mask.enabled = false;
}

RawBMP::~RawBMP()
{
	if (raw_buffer)
		free(raw_buffer);

	raw_buffer = NULL;
}

bool RawBMP::load_file(std::string file_name, int _ref_x, int _ref_y)
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

void RawBMP::set_mask(MaskWindow& _mask)
{
	mask = _mask;
}

MaskWindow& RawBMP::get_mask()
{
	return mask;
}

int RawBMP::get_width()
{
	return width;
}

int RawBMP::get_height()
{
	return height;
}

int RawBMP::get_pos_x()
{
	return pos_x;
}

int RawBMP::get_pos_y()
{
	return pos_y;
}

int RawBMP::get_angle()
{
	return angle;
}

void RawBMP::set_angle(int _angle)
{
	angle = _angle + base_rot;
}

void RawBMP::set_pos_x(int _pos_x)
{
	pos_x = _pos_x;
}

void RawBMP::set_pos_y(int _pos_y)
{
	pos_y = _pos_y;
}

void RawBMP::set_base_rot(int _base_rot)
{
	base_rot = _base_rot;
}

int RawBMP::get_ref_x()
{
	return ref_x;
}

int RawBMP::get_ref_y()
{
	return ref_y;
}

void RawBMP::get_pixel_at_pos(Pixel* pixel, int row, int col)
{
	pixel->r = raw_buffer[3 * (row * width + col)];
	pixel->g = raw_buffer[3 * (row * width + col) + 1];
	pixel->b = raw_buffer[3 * (row * width + col) + 2];
}

unsigned char* RawBMP::get_raw_buffer()
{
	return raw_buffer;
}
