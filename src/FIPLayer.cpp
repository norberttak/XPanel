/*
 * Copyright 2023 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "FIPLayer.h"

FIPLayer::FIPLayer()
{
	width = 0;
	height = 0;
	ref_x = 0;
	ref_y = 0;
	pos_x = 0;
	pos_y = 0;
	angle = 0;
	base_rot = 0;
//	raw_buffer = NULL;
	mask.enabled = false;
}

FIPLayer::~FIPLayer()
{
/*	if (raw_buffer)
		free(raw_buffer);

	raw_buffer = NULL;
	*/
}

void FIPLayer::set_mask(MaskWindow& _mask)
{
	mask = _mask;
}

MaskWindow& FIPLayer::get_mask()
{
	return mask;
}

int FIPLayer::get_width()
{
	return width;
}

int FIPLayer::get_height()
{
	return height;
}

int FIPLayer::get_pos_x()
{
	return pos_x;
}

int FIPLayer::get_pos_y()
{
	return pos_y;
}

int FIPLayer::get_angle()
{
	return angle;
}

void FIPLayer::set_angle(int _angle)
{
	angle = _angle + base_rot;
}

void FIPLayer::set_pos_x(int _pos_x)
{
	pos_x = _pos_x;
}

void FIPLayer::set_pos_y(int _pos_y)
{
	pos_y = _pos_y;
}

void FIPLayer::set_base_rot(int _base_rot)
{
	base_rot = _base_rot;
}

int FIPLayer::get_ref_x()
{
	return ref_x;
}

int FIPLayer::get_ref_y()
{
	return ref_y;
}