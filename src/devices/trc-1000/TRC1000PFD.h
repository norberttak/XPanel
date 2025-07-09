#pragma once
#include <vector>
#include "trc-1000/TRC1000.h"
#include "core/UsbHidDevice.h"

class TRC1000PFD : public TRC1000
{
private:
	std::vector<PanelButton> trc1000pfd_buttons;
	std::vector<PanelRotaryEncoder> trc1000pfd_encoders;
public:
	TRC1000PFD(ClassConfiguration& config);
	~TRC1000PFD();
};

