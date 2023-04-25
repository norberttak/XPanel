/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <string>
#include <vector>

#include "FIPLayer.h"
#include "RawBMP.h"

class FIPPage
{
private:
    std::vector<FIPLayer*> layers;
    /* we use two page buffers. one for hold the rendered page and one as a
       temporary buffer used during the interpolation. After successful
       interpolation we swap the two buffer's pointer */
    unsigned char* page_raw_buffer_1;
    unsigned char* page_raw_buffer_2;
    unsigned char* page_raw_buffer_ptr_act; //pointer for actual buffer
    unsigned char* page_raw_buffer_ptr_int; //pointer for interpolating buffer
    
    int raw_buffer_size;
    int screen_width;
    int screen_height;
    int bit_per_pixel;
    std::string page_name;
    void get_pixel_act_buffer(Pixel& px, int row, int col);
    void set_pixel_int_buffer(Pixel px, int row, int col);
    bool neighbour_average_act_buffer(Pixel& px, int row, int col);
    void interpolate();
    void render_layer(int layer_index);
public:
    FIPPage(int _screen_width, int _screen_height, int _bit_per_pixel, std::string _page_name);
    ~FIPPage();
    int add_layer_from_bmp_file(std::string filename, int ref_x, int ref_y, int base_rot);
    void set_mask(int layer_index, MaskWindow& mask);
    void rotate_layer(int layer_index, int angle);
    void translate_layer_x(int layer_index, int offset_x);
    void translate_layer_y(int layer_index, int offset_y);
    void render_page();
    int get_last_layer_index();
    unsigned char* get_raw_buffer();
    std::string get_name();
};
