/*
 * Copyright 2023 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <string>
#include <vector>
#include <fstream>

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

class FIPLayer
{
protected:
//	unsigned char* raw_buffer;
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
	FIPLayer();
	virtual ~FIPLayer();
//	virtual bool load_file(std::string file_name, int ref_x, int ref_y)=0;
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
};