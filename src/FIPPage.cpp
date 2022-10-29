/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "FIPPage.h"
#include <math.h>
#include <string.h>
#include "logger.h"

FIPPage::FIPPage(int _screen_width, int _screen_height, int _bit_per_pixel, std::string _page_name)
{
	bit_per_pixel = _bit_per_pixel;
	screen_height = _screen_height;
	screen_width = _screen_width;
	page_name = _page_name;
	raw_buffer_size = screen_height * screen_width * (bit_per_pixel / 8);

	page_raw_buffer = (unsigned char*)calloc(raw_buffer_size, sizeof(unsigned char));
}

FIPPage::~FIPPage()
{
	for (int i = 0; i < layers.size(); i++)
		delete(layers[i]);

	layers.clear();
	free(page_raw_buffer);
}

std::string FIPPage::get_name()
{
	return page_name;
}

int FIPPage::add_layer_from_bmp_file(std::string filename, int ref_x, int ref_y, int base_rot)
{
	RawBMP* new_layer = new RawBMP();
	new_layer->load_file(filename, ref_x, ref_y);
	new_layer->set_pos_x(0);
	new_layer->set_pos_y(0);
	new_layer->set_angle(0);
	new_layer->set_base_rot(base_rot);

	layers.push_back(new_layer);
	return (int)layers.size() - 1;
}

void FIPPage::rotate_layer(int layer_index, int angle)
{
	layers[layer_index]->set_angle(angle);
}

void FIPPage::translate_layer_x(int layer_index, int offset_x)
{
	layers[layer_index]->set_pos_x(offset_x);
}

void FIPPage::translate_layer_y(int layer_index, int offset_y)
{
	layers[layer_index]->set_pos_y(offset_y);
}

unsigned char* FIPPage::get_raw_buffer()
{
	return page_raw_buffer;
}

void FIPPage::render_layer(int layer_index)
{
	Pixel pixel;
	int layer_width = layers[layer_index]->get_width();
	int layer_height = layers[layer_index]->get_height();
	int pos_x = layers[layer_index]->get_pos_x();
	int pos_y = layers[layer_index]->get_pos_y();
	int ref_x = layers[layer_index]->get_ref_x();
	int ref_y = layers[layer_index]->get_ref_y();
	long double deg = -1 * (layers[layer_index]->get_angle() * 3.14) / 180;

	for (int row = 0; row < screen_height && row < layer_height; row++)
	{
		for (int col = 0; col < screen_width && col < layer_width; col++)
		{
			layers[layer_index]->get_pixel_at_pos(&pixel, row, col);

			// skip background color pixels
			if (pixel.r == 0 && pixel.g == 0 && pixel.b == 0)
				continue;

			//calculate rotation + translation
			int row_rot = (int)round(sin(deg) * (col - ref_x) + cos(deg) * (row - ref_y)) + pos_y;
			int col_rot = (int)round(cos(deg) * (col - ref_x) - sin(deg) * (row - ref_y)) + pos_x;

			//skip points that are outside of screen area
			if (row_rot < 0 || row_rot >= screen_height || col_rot < 0 || col_rot >= screen_width)
				continue;

			int buffer_linear_index = 3 * (row_rot * screen_width + col_rot);

			if (buffer_linear_index + 2 >= raw_buffer_size)
				continue;

			page_raw_buffer[buffer_linear_index + 0] = pixel.r;
			page_raw_buffer[buffer_linear_index + 1] = pixel.g;
			page_raw_buffer[buffer_linear_index + 2] = pixel.b;
		}
	}
}

int FIPPage::get_last_layer_index()
{
	return (int)layers.size() - 1;
}

void FIPPage::render_page()
{
	memset(page_raw_buffer, 0, raw_buffer_size);

	for (int i = 0; i < layers.size(); i++)
		render_layer(i);
}
