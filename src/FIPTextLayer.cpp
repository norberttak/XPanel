/*
 * Copyright 2023 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "FIPTextLayer.h"
#include "FIPFonts.h"
#include "Logger.h"

FIPTextLayer::FIPTextLayer() :
	FIPImageLayer()
{
	init_fip_fonts();
	text = "";
	image_buffer.set_and_allocate_buffer(MAX_CHAR_COUNT* (FIP_MAX_FONT_WIDTH + CHAR_SPACE), FIP_FONT_HEIGHT);
	width = image_buffer.width;
	height = image_buffer.height;
}

FIPTextLayer::~FIPTextLayer()
{
	fip_font_positions.clear();
}

bool FIPTextLayer::load_bmp_font_file(std::string file_name)
{
	return font_image_buffer.load_from_bmp_file(file_name);
}

void FIPTextLayer::set_text(std::string _text)
{
	if (text == _text)
		return;


	image_buffer.clear_raw_buffer();
	int char_count = 0;
	int render_col = 0;

	for (char ch : _text)
	{
		if (ch < FIP_MIN_FONT_ASCII_ID || ch > FIP_MAX_FONT_ASCII_ID)
			continue;

		if (++char_count > MAX_CHAR_COUNT)
			break;

		for (int row = fip_font_positions[ch].y; row < fip_font_positions[ch].y + fip_font_positions[ch].height; row++)
		{
			for (int col = fip_font_positions[ch].x; col < fip_font_positions[ch].x + fip_font_positions[ch].width; col++)
			{
				Pixel pixel;

				font_image_buffer.get_pixel_at_pos(&pixel, FIP_FONT_FILE_HEIGHT - row, col);
				image_buffer.set_pixel_at_pos(pixel, (fip_font_positions[ch].height-1) - (row - fip_font_positions[ch].y), render_col + col - fip_font_positions[ch].x);
			}
		}

		render_col += fip_font_positions[ch].width + CHAR_SPACE;
	}

	text = _text;
}
