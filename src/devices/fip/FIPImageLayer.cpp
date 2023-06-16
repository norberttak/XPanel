/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include "core/Logger.h"
#include "fip/FIPImageLayer.h"

bool ImageBuffer::load_from_bmp_file(std::string file_name)
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
}

ImageBuffer::ImageBuffer()
{
	raw_buffer = NULL;
	width = 0;
	height = 0;
}

void ImageBuffer::set_and_allocate_buffer(int _width, int _height)
{
	if (raw_buffer)
		free(raw_buffer);

	raw_buffer = (unsigned char*)calloc(_width * _height * 3, sizeof(unsigned char));
	width = _width;
	height = _height;
}

void ImageBuffer::get_pixel_at_pos(Pixel* pixel, int row, int col)
{
	pixel->r = raw_buffer[3 * (row * width + col)];
	pixel->g = raw_buffer[3 * (row * width + col) + 1];
	pixel->b = raw_buffer[3 * (row * width + col) + 2];
}

void ImageBuffer::set_pixel_at_pos(Pixel pixel, int row, int col)
{
	raw_buffer[3 * (row * width + col)] = pixel.r;
	raw_buffer[3 * (row * width + col) + 1] = pixel.g;
	raw_buffer[3 * (row * width + col) + 2] = pixel.b;
}

unsigned char* ImageBuffer::get_raw_buffer()
{
	return raw_buffer;
}

void ImageBuffer::clear_raw_buffer()
{
	if (!raw_buffer)
		return;

	memset(raw_buffer, 0, width * height * 3);
}

ImageBuffer::~ImageBuffer()
{
	if (raw_buffer)
		free(raw_buffer);

	raw_buffer = NULL;
}



FIPImageLayer::FIPImageLayer()
	:FIPLayer()
{
	
}

FIPImageLayer::~FIPImageLayer()
{
	
}

void FIPImageLayer::get_pixel_at_pos(Pixel* pixel, int row, int col)
{
	image_buffer.get_pixel_at_pos(pixel, row, col);
}

bool FIPImageLayer::load_file(std::string file_name, int _ref_x, int _ref_y)
{
	if (!image_buffer.load_from_bmp_file(file_name))
	{
		Logger(TLogLevel::logERROR) << "FIPImageLayer: error to load bmp file " << file_name << std::endl;
		return false;
	}

	ref_x = _ref_x;
	ref_y = _ref_y;
	height = image_buffer.height;
	width = image_buffer.width;

	return true;
}

unsigned char* FIPImageLayer::get_raw_buffer()
{
	return image_buffer.get_raw_buffer();
}