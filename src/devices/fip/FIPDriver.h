/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <queue>
#include <map>

typedef struct {
	std::string serial_number;
	void* hDevice;
	std::queue<uint16_t> button_states;
	int page_index;
} FIP_device_handle;

FIP_device_handle* fip_open(std::string serial_number);
void fip_close(FIP_device_handle* device);
void fip_add_page(FIP_device_handle* device, int page_index, bool set_active);
void fip_set_led(FIP_device_handle* device, int led_index, int led_state);
uint16_t fip_get_button_states(FIP_device_handle* device);
int fip_get_current_page(FIP_device_handle* device);
void fip_set_image(FIP_device_handle* device, int page, const void* byte_buffer);
void fip_set_text(FIP_device_handle* device, int page, std::string text);
