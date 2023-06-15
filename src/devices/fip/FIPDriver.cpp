/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <string>
#include <inttypes.h>
#include <stdlib.h>
#include <mutex>
#include "core/Logger.h"
#include "fip/FIPDriver.h"

#ifdef WIN32
#include "DirectOutput.h"
#include <Windows.h>
static HMODULE m_module = NULL;
std::map<std::string, FIP_device_handle*> fip_device_handles;
static bool direct_output_initialized = false;
static int ref_count = 0;
static bool enumerate_done = false;
static std::mutex guard;

static void __stdcall on_enumerate_device(void* hDevice, void* pCtxt);
static void __stdcall on_page_changed(void* hDevice, DWORD dwPage, bool bSetActive, void* pCtxt);
static void __stdcall on_soft_button_changed(void* hDevice, DWORD dwButtons, void* pCtxt);
static void __stdcall on_device_changed(void* hDevice, bool bAdded, void* pCtxt);

Pfn_DirectOutput_Initialize					_DirectOutput_Initialize;
Pfn_DirectOutput_Deinitialize				_DirectOutput_Deinitialize;
Pfn_DirectOutput_RegisterDeviceCallback		_DirectOutput_RegisterDeviceCallback;
Pfn_DirectOutput_Enumerate					_DirectOutput_Enumerate;
Pfn_DirectOutput_RegisterPageCallback		_DirectOutput_RegisterPageCallback;
Pfn_DirectOutput_RegisterSoftButtonCallback	_DirectOutput_RegisterSoftButtonCallback;
Pfn_DirectOutput_GetDeviceType				_DirectOutput_GetDeviceType;
Pfn_DirectOutput_GetDeviceInstance			_DirectOutput_GetDeviceInstance;
Pfn_DirectOutput_SetProfile					_DirectOutput_SetProfile;
Pfn_DirectOutput_AddPage					_DirectOutput_AddPage;
Pfn_DirectOutput_RemovePage					_DirectOutput_RemovePage;
Pfn_DirectOutput_SetLed						_DirectOutput_SetLed;
Pfn_DirectOutput_SetString					_DirectOutput_SetString;
Pfn_DirectOutput_SetImage					_DirectOutput_SetImage;
Pfn_DirectOutput_SetImageFromFile			_DirectOutput_SetImageFromFile;
Pfn_DirectOutput_StartServer				_DirectOutput_StartServer;
Pfn_DirectOutput_CloseServer				_DirectOutput_CloseServer;
Pfn_DirectOutput_SendServerMsg				_DirectOutput_SendServerMsg;
Pfn_DirectOutput_SendServerFile				_DirectOutput_SendServerFile;
Pfn_DirectOutput_SaveFile					_DirectOutput_SaveFile;
Pfn_DirectOutput_DisplayFile				_DirectOutput_DisplayFile;
Pfn_DirectOutput_DeleteFile					_DirectOutput_DeleteFile;
Pfn_DirectOutput_GetSerialNumber			_DirectOutput_GetSerialNumber;

bool get_direct_output_filename(LPTSTR filename, DWORD length)
{
	bool retval(false);
	HKEY hk;
	// Read the Full Path to DirectOutput.dll from the registry
	long lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Saitek\\DirectOutput"), 0, KEY_READ, &hk);
	if (ERROR_SUCCESS == lRet)
	{
		DWORD size(length * sizeof(filename[0]));
		// Note: this DirectOutput entry will point to the correct version on x86 or x64 systems
		lRet = RegQueryValueEx(hk, TEXT("DirectOutput_Saitek"), 0, 0, (LPBYTE)filename, &size);
		if (ERROR_SUCCESS == lRet)
		{
			retval = true;
		}
		RegCloseKey(hk);
	}
	return retval;
}

HRESULT DirectOutput_LoadLibrary()
{
	TCHAR filename[2048] = { 0 };
	if (get_direct_output_filename(filename, sizeof(filename) / sizeof(filename[0])))
	{
		m_module = LoadLibrary(filename);
		if (m_module)
		{
			_DirectOutput_Initialize = (Pfn_DirectOutput_Initialize)GetProcAddress(m_module, "DirectOutput_Initialize");
			_DirectOutput_Deinitialize = (Pfn_DirectOutput_Deinitialize)GetProcAddress(m_module, "DirectOutput_Deinitialize");
			_DirectOutput_RegisterDeviceCallback = (Pfn_DirectOutput_RegisterDeviceCallback)GetProcAddress(m_module, "DirectOutput_RegisterDeviceCallback");
			_DirectOutput_Enumerate = (Pfn_DirectOutput_Enumerate)GetProcAddress(m_module, "DirectOutput_Enumerate");
			_DirectOutput_RegisterPageCallback = (Pfn_DirectOutput_RegisterPageCallback)GetProcAddress(m_module, "DirectOutput_RegisterPageCallback");
			_DirectOutput_RegisterSoftButtonCallback = (Pfn_DirectOutput_RegisterSoftButtonCallback)GetProcAddress(m_module, "DirectOutput_RegisterSoftButtonCallback");
			_DirectOutput_GetDeviceType = (Pfn_DirectOutput_GetDeviceType)GetProcAddress(m_module, "DirectOutput_GetDeviceType");
			_DirectOutput_GetDeviceInstance = (Pfn_DirectOutput_GetDeviceInstance)GetProcAddress(m_module, "DirectOutput_GetDeviceInstance");
			_DirectOutput_SetProfile = (Pfn_DirectOutput_SetProfile)GetProcAddress(m_module, "DirectOutput_SetProfile");
			_DirectOutput_AddPage = (Pfn_DirectOutput_AddPage)GetProcAddress(m_module, "DirectOutput_AddPage");
			_DirectOutput_RemovePage = (Pfn_DirectOutput_RemovePage)GetProcAddress(m_module, "DirectOutput_RemovePage");
			_DirectOutput_SetLed = (Pfn_DirectOutput_SetLed)GetProcAddress(m_module, "DirectOutput_SetLed");
			_DirectOutput_SetString = (Pfn_DirectOutput_SetString)GetProcAddress(m_module, "DirectOutput_SetString");
			_DirectOutput_SetImage = (Pfn_DirectOutput_SetImage)GetProcAddress(m_module, "DirectOutput_SetImage");
			_DirectOutput_SetImageFromFile = (Pfn_DirectOutput_SetImageFromFile)GetProcAddress(m_module, "DirectOutput_SetImageFromFile");
			_DirectOutput_StartServer = (Pfn_DirectOutput_StartServer)GetProcAddress(m_module, "DirectOutput_StartServer");
			_DirectOutput_CloseServer = (Pfn_DirectOutput_CloseServer)GetProcAddress(m_module, "DirectOutput_CloseServer");
			_DirectOutput_SendServerMsg = (Pfn_DirectOutput_SendServerMsg)GetProcAddress(m_module, "DirectOutput_SendServerMsg");
			_DirectOutput_SendServerFile = (Pfn_DirectOutput_SendServerFile)GetProcAddress(m_module, "DirectOutput_SendServerFile");
			_DirectOutput_SaveFile = (Pfn_DirectOutput_SaveFile)GetProcAddress(m_module, "DirectOutput_SaveFile");
			_DirectOutput_DisplayFile = (Pfn_DirectOutput_DisplayFile)GetProcAddress(m_module, "DirectOutput_DisplayFile");
			_DirectOutput_DeleteFile = (Pfn_DirectOutput_DeleteFile)GetProcAddress(m_module, "DirectOutput_DeleteFile");
			_DirectOutput_GetSerialNumber = (Pfn_DirectOutput_GetSerialNumber)GetProcAddress(m_module, "DirectOutput_GetSerialNumber");

			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT DirectOutput_UnLoadLibrary()
{
	if (m_module)
	{
		FreeLibrary(m_module);
		m_module = NULL;
	}
	return S_OK;
}

static void __stdcall on_soft_button_changed(void* hDevice, DWORD dwButtons, void* pCtxt)
{
	FIP_device_handle* fip_device = (FIP_device_handle*)pCtxt;
	guard.lock();
	fip_device->button_states.push((uint16_t)dwButtons);
	guard.unlock();
}

static void __stdcall on_page_changed(void* hDevice, DWORD dwPage, bool bSetActive, void* pCtxt)
{
	FIP_device_handle* fip_device = (FIP_device_handle*)pCtxt;
	if (bSetActive)
	{
		guard.lock();
		fip_device->page_index = dwPage;
		guard.unlock();
		Logger(logTRACE) << "FIP driver: page changed to " << dwPage << std::endl;
	}
}

static void __stdcall on_enumerate_device(void* hDevice, void* pCtxt)
{
	on_device_changed(hDevice, true, pCtxt);
}

static void __stdcall on_device_changed(void* hDevice, bool bAdded, void* pCtxt)
{
	if (bAdded)
	{
		wchar_t serial_number_wchar[255];
		_DirectOutput_GetSerialNumber(hDevice, serial_number_wchar, 255);
		std::wstring ws(serial_number_wchar);
		std::string serial_number_str(ws.begin(), ws.end());

		fip_device_handles[serial_number_str] = new FIP_device_handle();
		fip_device_handles[serial_number_str]->hDevice = hDevice;
		fip_device_handles[serial_number_str]->page_index = 0;
		fip_device_handles[serial_number_str]->serial_number = serial_number_str;

		Logger(logINFO) << "FIP driver: new device added. Serial=" << serial_number_str << std::endl;
		// Register a callback that gets called when the page changes
		_DirectOutput_RegisterPageCallback(hDevice, on_page_changed, (void*)fip_device_handles[serial_number_str]);

		// Register a callback that gets called when the soft buttons get changed
		_DirectOutput_RegisterSoftButtonCallback(hDevice, on_soft_button_changed, (void*)fip_device_handles[serial_number_str]);
	}
	else
	{
		// device removed
		for (const auto& it : fip_device_handles)
		{
			if (it.second->hDevice == hDevice) {
				Logger(logERROR) << "FIP driver: device removed. Serial=" << it.second->serial_number << std::endl;
				it.second->hDevice = NULL;
			}
		}
	}
}

FIP_device_handle* fip_open(std::string serial_number)
{
	Logger(logTRACE) << "FIP driver: fip_open(" << serial_number << ")" << std::endl;
	ref_count++;
	if (!direct_output_initialized)
	{
		Logger(logTRACE) << "FIP driver: initialize DirectDriver" << std::endl;
		DirectOutput_LoadLibrary();
		_DirectOutput_Initialize(L"Xpanel plugin");
		_DirectOutput_RegisterDeviceCallback(on_device_changed, NULL);

		_DirectOutput_Enumerate(on_enumerate_device, NULL);
		while (!enumerate_done)
		{

		}
		direct_output_initialized = true;
	}

	for (const auto& it : fip_device_handles)
	{
		if (it.second->serial_number == serial_number)
			return it.second;
	}

	return NULL;
}

void fip_close(FIP_device_handle* device)
{
	if (!device->hDevice)
	{
		Logger(logERROR) << "FIP driver: device not opened" << std::endl;
		return;
	}

	Logger(logINFO) << "FIP driver: fip_close() serial=" << device->serial_number << std::endl;

	for (const auto& it : fip_device_handles)
	{
		if (it.second->serial_number == device->serial_number)
		{
			delete it.second;
			fip_device_handles.erase(it.first);
		}
	}

	ref_count--;
	if (ref_count <= 0)
	{
		Logger(logINFO) << "FIP driver: de-init direct driver" << std::endl;
		_DirectOutput_Deinitialize();
		DirectOutput_UnLoadLibrary();
		direct_output_initialized = false;
	}
}

void fip_add_page(FIP_device_handle* device, int page_index, bool set_active)
{
	if (!device->hDevice)
	{
		Logger(logERROR) << "FIP driver: device not opened" << std::endl;
		return;
	}

	Logger(logTRACE) << "FIP driver: fip_add_page serial=" << device->serial_number << " page:" << page_index << " active:" << set_active << std::endl;
	_DirectOutput_AddPage(device->hDevice, page_index, set_active ? FLAG_SET_AS_ACTIVE : 0);
}

void fip_set_led(FIP_device_handle* device, int led_index, int led_state)
{
	if (!device->hDevice)
	{
		Logger(logERROR) << "FIP driver: device not opened" << std::endl;
		return;
	}

	Logger(logTRACE) << "FIP driver: fip_set_led serial=" << device->serial_number << " led_index:" << led_index << " led_state:" << led_state << std::endl;
	_DirectOutput_SetLed(device->hDevice, device->page_index, led_index, led_state);
}

uint16_t fip_get_button_states(FIP_device_handle* device)
{
	if (!device->hDevice)
	{
		Logger(logERROR) << "FIP driver: device not opened" << std::endl;
		return 0;
	}

	uint16_t states = 0;

	guard.lock();
	if (!device->button_states.empty())
	{
		states = device->button_states.front();
		if (device->button_states.size() > 1)
			device->button_states.pop();
	}
	guard.unlock();

	//Logger(logTRACE) << "FIP driver: fip_get_button_state serial=" << device->serial_number << " state:" << states << std::endl;
	return states;
}

int fip_get_current_page(FIP_device_handle* device)
{
	guard.lock();
	int index = device->page_index;
	guard.unlock();

	return index;
}

void fip_set_image(FIP_device_handle* device, int page, const void* byte_buffer)
{
	if (!device->hDevice)
	{
		Logger(logERROR) << "FIP driver: device not opened" << std::endl;
		return;
	}
	_DirectOutput_SetImage(device->hDevice, page, 0, 320 * 240 * 3, (const void*)byte_buffer);
}

void fip_set_text(FIP_device_handle* device, int page, std::string text)
{
	if (!device->hDevice)
	{
		Logger(logERROR) << "FIP driver: device not opened" << std::endl;
		return;
	}
	std::wstring wide_string = std::wstring(text.begin(), text.end());
	_DirectOutput_SetString(device->hDevice, page, 0, wide_string.size(), wide_string.c_str());
}

#else

FIP_device_handle* fip_open(std::string serial_number)
{
	(void)serial_number;
	Logger(logERROR) << "FIP driver not implemented for Linux/Mac" << std::endl;
	return NULL;
}

void fip_close(FIP_device_handle* device)
{
	(void)device;
}

void fip_add_page(FIP_device_handle* device, int page_index, bool set_active)
{
	(void)device;
	(void)page_index;
	(void)set_active;
}

void fip_set_led(FIP_device_handle* device, int led_index, int led_state)
{
	(void)device;
	(void)led_index;
	(void)led_state;
}

uint16_t fip_get_button_states(FIP_device_handle* device)
{
	(void)device;
	return 0;
}

int fip_get_current_page(FIP_device_handle* device)
{
	(void)device;
	return 0;
}

void fip_set_image(FIP_device_handle* device, int page, const void* byte_buffer)
{
	(void)device;
	(void)page;
	(void)byte_buffer;
}

void fip_set_text(FIP_device_handle* device, int page, std::string text)
{
	(void)device;
	(void)page;
	(void)text;
}

#endif
