/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <string>
#include <vector>
#include <fstream>
#include "FIPLayer.h"

#pragma pack(push)
#pragma pack(1)
typedef struct {
	uint16_t type;              // Magic identifier: 0x4d42
	uint32_t size;              // File size in bytes
	uint16_t reserved1;         // Not used
	uint16_t reserved2;         // Not used
	uint32_t offset;            // Offset to image data in bytes from beginning of file
	uint32_t dib_header_size;   // DIB Header size in bytes
	int32_t  width_px;          // Width of the image
	int32_t  height_px;         // Height of image
	uint16_t num_planes;        // Number of color planes
	uint16_t bits_per_pixel;    // Bits per pixel
	uint32_t compression;       // Compression type
	uint32_t image_size_bytes;  // Image size in bytes
	int32_t  x_resolution_ppm;  // Pixels per meter
	int32_t  y_resolution_ppm;  // Pixels per meter
	uint32_t num_colors;        // Number of colors
	uint32_t important_colors;  // Important colors
} BMPHeader;
#pragma pack(pop)

class ImageBuffer {
private:
	BMPHeader bmp_header;
	unsigned char* raw_buffer;
public:
	ImageBuffer();
	void set_and_allocate_buffer(int _width, int _height);
	virtual ~ImageBuffer();
	int width;
	int height;
	bool load_from_bmp_file(std::string file_name);
	//bool load_from_png_file(std::string file_name);
	void set_pixel_at_pos(Pixel pixel, int row, int col);
	void get_pixel_at_pos(Pixel* pixel, int row, int col);
	unsigned char* get_raw_buffer();
	void clear_raw_buffer();
};

class FIPImageLayer: public FIPLayer
{
protected:
	ImageBuffer image_buffer;
public:
	FIPImageLayer();
	virtual ~FIPImageLayer();
	bool load_file(std::string file_name, int ref_x, int ref_y);
	virtual void get_pixel_at_pos(Pixel* pixel, int row, int col);
	virtual unsigned char* get_raw_buffer();
};