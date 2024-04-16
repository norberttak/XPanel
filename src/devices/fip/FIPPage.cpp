/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "FIPPage.h"
#include <math.h>
#include <string.h>
#include "core/Logger.h"

FIPPage::FIPPage(int _screen_width, int _screen_height, int _bit_per_pixel, std::string _page_name)
{
	bit_per_pixel = _bit_per_pixel;
	screen_height = _screen_height;
	screen_width = _screen_width;
	page_name = _page_name;
	raw_buffer_size = screen_height * screen_width * (bit_per_pixel / 8);

	page_raw_buffer_1 = (unsigned char*)calloc(raw_buffer_size, sizeof(unsigned char));
	page_raw_buffer_2 = (unsigned char*)calloc(raw_buffer_size, sizeof(unsigned char));

	page_raw_buffer_ptr_act = page_raw_buffer_1;
	page_raw_buffer_ptr_int = page_raw_buffer_2;
}

FIPPage::FIPPage(FIPPage* other)
{
	bit_per_pixel = other->bit_per_pixel;
	screen_height = other->screen_height;
	screen_width = other->screen_width;
	page_name = other->page_name;
	raw_buffer_size = other->raw_buffer_size;

	page_raw_buffer_1 = (unsigned char*)calloc(raw_buffer_size, sizeof(unsigned char));
	page_raw_buffer_2 = (unsigned char*)calloc(raw_buffer_size, sizeof(unsigned char));

	memcpy(page_raw_buffer_1, other->page_raw_buffer_1, raw_buffer_size);
	memcpy(page_raw_buffer_2, other->page_raw_buffer_2, raw_buffer_size);

	page_raw_buffer_ptr_act = page_raw_buffer_1;
	page_raw_buffer_ptr_int = page_raw_buffer_2;

	for (auto lay : other->layers)
	{
		if (lay->type == FIPImageLayer::FIPLayerType::IMAGE)
			layers.push_back(new FIPImageLayer(lay));
		else if (lay->type == FIPImageLayer::FIPLayerType::TEXT)
			layers.push_back(new FIPTextLayer((FIPTextLayer*)lay));
		else
			Logger(TLogLevel::logERROR) << "FIP Page: unknown layer type";
	}
}

FIPPage::~FIPPage()
{
	for (int i = 0; i < layers.size(); i++)
		delete(layers[i]);

	layers.clear();
	free(page_raw_buffer_1);
	free(page_raw_buffer_2);
}

std::string FIPPage::get_name()
{
	return page_name;
}

void FIPPage::set_mask(int layer_index, MaskWindow& mask)
{
	layers[layer_index]->set_mask(mask);
}

int FIPPage::add_layer_from_bmp_file(std::string filename, int ref_x, int ref_y, int base_rot)
{
	FIPImageLayer* new_layer = new FIPImageLayer();
	new_layer->load_file(filename, ref_x, ref_y);
	new_layer->set_pos_x(0);
	new_layer->set_pos_y(0);
	new_layer->set_angle(0);
	new_layer->set_base_rot(base_rot);
	new_layer->type = FIPImageLayer::FIPLayerType::IMAGE;
	layers.push_back(new_layer);
	return (int)layers.size() - 1;
}

int FIPPage::add_text_layer(std::string font_filename, int base_rot)
{
	FIPTextLayer* new_layer = new FIPTextLayer();
	new_layer->load_bmp_font_file(font_filename);
	new_layer->set_pos_x(0);
	new_layer->set_pos_y(0);
	new_layer->set_angle(0);
	new_layer->set_base_rot(base_rot);
	new_layer->type = FIPImageLayer::FIPLayerType::TEXT;

	layers.push_back(new_layer);
	return (int)layers.size() - 1;
}

void FIPPage::set_text(int layer_index, std::string text)
{
	FIPTextLayer* layer = (FIPTextLayer*)layers[layer_index];
	layer->set_text(text);
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
	return page_raw_buffer_ptr_act;
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

	for (int row = 0; row < layer_height; row++)
	{
		for (int col = 0; col < layer_width; col++)
		{
			layers[layer_index]->get_pixel_at_pos(&pixel, row, col);
			
			// skip background color pixels
			if (pixel.r == 0 && pixel.g == 0 && pixel.b == 0)
				continue;

			//calculate rotation + translation
			int row_rot = (int)round(sin(deg) * (col - ref_x) + cos(deg) * (row - ref_y)) + pos_y;
			int col_rot = (int)round(cos(deg) * (col - ref_x) - sin(deg) * (row - ref_y)) + pos_x;

			MaskWindow mask = layers[layer_index]->get_mask();
			if (mask.enabled && (row_rot > mask.pos_y + mask.height || row_rot<mask.pos_y || col_rot>mask.pos_x + mask.width || col_rot < mask.pos_x))
				continue;

			//skip points that are outside of screen area
			if (row_rot < 0 || row_rot >= screen_height || col_rot < 0 || col_rot >= screen_width)
				continue;

			int buffer_linear_index = 3 * (row_rot * screen_width + col_rot);

			if (buffer_linear_index + 2 >= raw_buffer_size)
				continue;

			page_raw_buffer_ptr_act[buffer_linear_index + 0] = pixel.r;
			page_raw_buffer_ptr_act[buffer_linear_index + 1] = pixel.g;
			page_raw_buffer_ptr_act[buffer_linear_index + 2] = pixel.b;
		}
	}
}

int FIPPage::get_last_layer_index()
{
	return (int)layers.size() - 1;
}

void FIPPage::get_pixel_act_buffer(Pixel& px, int row, int col)
{
	int buffer_linear_index = 3 * (row * screen_width + col);

	px.r = page_raw_buffer_ptr_act[buffer_linear_index + 0];
	px.g = page_raw_buffer_ptr_act[buffer_linear_index + 1];
	px.b = page_raw_buffer_ptr_act[buffer_linear_index + 2];
}

void FIPPage::set_pixel_int_buffer(Pixel px, int row, int col)
{
	int buffer_linear_index = 3 * (row * screen_width + col);

	page_raw_buffer_ptr_int[buffer_linear_index + 0] = px.r;
	page_raw_buffer_ptr_int[buffer_linear_index + 1] = px.g;
	page_raw_buffer_ptr_int[buffer_linear_index + 2] = px.b;
}

/* Calculate the four neighbour (top,bottom,left,right) pixel's color average */
bool FIPPage::neighbour_average_act_buffer(Pixel& px, int row, int col)
{
	if (row<1 || col<1 || row>screen_height - 1 || col>screen_width - 1)
		return false;

	int index_top    = 3 * ((row+1) * screen_width + col);
	int index_bottom = 3 * ((row - 1) * screen_width + col);
	int index_left   = 3 * (row * screen_width + col - 1);
	int index_right  = 3 * (row * screen_width + col + 1);

	int red = (int)(page_raw_buffer_ptr_act[index_top + 0] +
		page_raw_buffer_ptr_act[index_bottom + 0] +
		page_raw_buffer_ptr_act[index_left + 0] +
		page_raw_buffer_ptr_act[index_right + 0]) / 4;

	int green = (int)(page_raw_buffer_ptr_act[index_top + 1] +
		page_raw_buffer_ptr_act[index_bottom + 1] +
		page_raw_buffer_ptr_act[index_left + 1] +
		page_raw_buffer_ptr_act[index_right + 1]) / 4;

	int blue = (int)(page_raw_buffer_ptr_act[index_top + 2] +
		page_raw_buffer_ptr_act[index_bottom + 2] +
		page_raw_buffer_ptr_act[index_left + 2] +
		page_raw_buffer_ptr_act[index_right + 2]) / 4;

	px.r = (uint8_t)red;
	px.g = (uint8_t)green;
	px.b = (uint8_t)blue;

	return true;
}

void FIPPage::interpolate()
{
	Pixel pixel;

	for (int row = 1; row < screen_height-1; row++)
	{
		for (int col = 1; col < screen_width-1; col++)
		{
			get_pixel_act_buffer(pixel, row, col);

			//calculate neighbour's average color in case of a black pixels
			if (pixel.r == 0 && pixel.g == 0 && pixel.b == 0)
				neighbour_average_act_buffer(pixel, row, col);
			
			set_pixel_int_buffer(pixel, row, col);
		}
	}

	//swap the two buffer pointers
	if (page_raw_buffer_ptr_act == page_raw_buffer_1)
	{
		page_raw_buffer_ptr_act = page_raw_buffer_2;
		page_raw_buffer_ptr_int = page_raw_buffer_1;
	}
	else
	{
		page_raw_buffer_ptr_act = page_raw_buffer_1;
		page_raw_buffer_ptr_int = page_raw_buffer_2;
	}
}

void FIPPage::render_page()
{
	memset(page_raw_buffer_ptr_act, 0, raw_buffer_size);

	for (int i = 0; i < layers.size(); i++)
		render_layer(i);

	interpolate();
}
