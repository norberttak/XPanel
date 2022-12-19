#pragma once
#include <vector>
#include "TRC1000.h"
#include "UsbHidDevice.h"

class TRC1000PFD : public TRC1000
{
private:
	std::vector<PanelButton> trc1000pfd_buttons;
	std::vector<PanelRotaryEncoder> trc1000pfd_encoders;
public:
	TRC1000PFD(DeviceConfiguration& config);
	~TRC1000PFD();
};

