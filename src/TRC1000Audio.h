#pragma once
#include "TRC1000.h"

class TRC1000Audio : public TRC1000
{
private:
	std::vector<PanelButton> trc1000audio_buttons;
	std::vector<PanelRotaryEncoder> trc1000audio_encoders;
	std::vector<PanelLight> trc1000audio_lights;
public:
	TRC1000Audio(DeviceConfiguration& config);
	~TRC1000Audio();
};

