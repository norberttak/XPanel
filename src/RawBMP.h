/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <string>
#include <vector>
#include <fstream>

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

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} Pixel;
struct MaskWindow {
	MaskWindow(int _pos_x, int _pos_y, int _width, int _height)
	{
		pos_x = _pos_x;
		pos_y = _pos_y;
		width = _width;
		height = _height;
		enabled = true;
	}

	MaskWindow()
	{
		pos_x = 0;
		pos_y = 0;
		width = 0;
		height = 0;
		enabled = false;
	}

	MaskWindow& operator=(const MaskWindow& other)
	{
		pos_x = other.pos_x;
		pos_y = other.pos_y;
		width = other.width;
		height = other.height;
		enabled = other.enabled;

		return *this;
	}

	bool enabled;
	int pos_x;
	int pos_y; //x and y coordinate of lower left corner
	int width;
	int height;
};

class RawBMP
{
private:
	unsigned char* raw_buffer;
	BMPHeader bmp_header;
	int height;
	int width;
	int ref_x;
	int ref_y;
	int base_rot;
	int pos_x;
	int pos_y;
	int angle;
	MaskWindow mask;
public:
	RawBMP();
	~RawBMP();
	bool load_file(std::string file_name, int ref_x, int ref_y);
	void set_mask(MaskWindow& mask);
	MaskWindow& get_mask();
	int get_width();
	int get_height();
	int get_pos_x();
	int get_pos_y();
	int get_angle();
	int get_ref_x();
	int get_ref_y();
	void set_angle(int _angle);
	void set_pos_x(int _pos_x);
	void set_pos_y(int _pos_y);
	void set_base_rot(int _base_rot);
	void get_pixel_at_pos(Pixel* pixel, int row, int col);
	unsigned char* get_raw_buffer();
};