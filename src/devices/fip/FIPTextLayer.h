/*
 * Copyright 2023 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "FIPImageLayer.h"

class FIPTextLayer : public FIPImageLayer
{
private:
	std::string text;
	ImageBuffer font_image_buffer;
	static constexpr int MAX_CHAR_COUNT = 32;
	static constexpr int CHAR_SPACE = 2;
public:
	FIPTextLayer();
	FIPTextLayer(FIPTextLayer* other);
	virtual ~FIPTextLayer();
	bool load_bmp_font_file(std::string file_name);
	void set_text(std::string _text);
};
