/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "fip/FIPDriver.h"

FIP_device_handle f_device;

FIP_device_handle* fip_open(std::string serial_number)
{
	f_device.serial_number = serial_number;

	return &f_device;
}

void fip_close(FIP_device_handle* device)
{

}

void fip_add_page(FIP_device_handle* device, int page_index, bool set_active)
{

}

int led_states[8];
int test_fip_get_led_state(int led_index)
{
	return led_states[led_index];
}

void fip_set_led(FIP_device_handle* device, int led_index, int led_state)
{
	led_states[led_index] = led_state;
}

uint16_t button_states = 0;
void test_fip_set_button_states(uint16_t _button_states)
{
	button_states = _button_states;
}

uint16_t fip_get_button_states(FIP_device_handle* device)
{
	return button_states;
}

int current_page_index = 0;
void test_fip_set_current_page(int page)
{
	current_page_index = page;
}

int fip_get_current_page(FIP_device_handle* device)
{
	return current_page_index;
}

const size_t image_buffer_size = 320 * 240 * 3;
unsigned char image_buffer[image_buffer_size];
void fip_set_image(FIP_device_handle* device, int page, const void* byte_buffer)
{
	memcpy((void*)&image_buffer[0], byte_buffer, image_buffer_size);
}

void test_fip_get_image(unsigned char* buffer, size_t buffer_size)
{
	memcpy(buffer, (void*)&image_buffer[0], buffer_size);
}

void fip_set_text(FIP_device_handle* device, int page, std::string text)
{

}
