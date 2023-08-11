/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <vector>
#include <map>
#include "XPLMGraphics.h"
#include "XPLMPlanes.h"
#include "XPLMDisplay.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"
#include "XPLMMenus.h"
#include "XPLMProcessing.h"
#include "XPWidgets.h"
#include "XPWidgetDefs.h"
#include "XPStandardWidgets.h"

extern void XPLMSetGraphicsState(
	int                  inEnableFog,
	int                  inNumberTexUnits,
	int                  inEnableLighting,
	int                  inEnableAlphaTesting,
	int                  inEnableAlphaBlending,
	int                  inEnableDepthTesting,
	int                  inEnableDepthWriting)
{

}

extern void XPLMGetWindowGeometry(
	XPLMWindowID         inWindowID,
	int* outLeft,    /* Can be NULL */
	int* outTop,    /* Can be NULL */
	int* outRight,    /* Can be NULL */
	int* outBottom)
{

}

extern void XPLMDrawString(
	float* inColorRGB,
	int                  inXOffset,
	int                  inYOffset,
	char* inChar,
	int* inWordWrapWidth,    /* Can be NULL */
	XPLMFontID           inFontID)
{

}

extern void XPLMGetScreenBoundsGlobal(
	int* outLeft,    /* Can be NULL */
	int* outTop,    /* Can be NULL */
	int* outRight,    /* Can be NULL */
	int* outBottom)
{

}

extern XPLMWindowID XPLMCreateWindowEx(
	XPLMCreateWindow_t* inParams)
{
	return NULL;
}

extern void XPLMSetWindowPositioningMode(
	XPLMWindowID         inWindowID,
	XPLMWindowPositioningMode inPositioningMode,
	int                  inMonitorIndex)
{

}

extern void       XPLMSetWindowResizingLimits(
	XPLMWindowID         inWindowID,
	int                  inMinWidthBoxels,
	int                  inMinHeightBoxels,
	int                  inMaxWidthBoxels,
	int                  inMaxHeightBoxels)
{

}

extern void XPLMSetWindowTitle(
	XPLMWindowID inWindowID,
	const char* inWindowTitle)
{

}

extern void XPLMDestroyWindow(
	XPLMWindowID inWindowID)
{

}

struct WidgetMock {
	WidgetMock()
		:callback(NULL),visible(false)
	{
		properties.clear();
	}
	std::map<XPWidgetPropertyID, intptr_t> properties;
	XPWidgetFunc_t callback;
	bool visible;
};

std::vector<WidgetMock> test_widgets;

extern void XPAddWidgetCallback(
	XPWidgetID           inWidget,
	XPWidgetFunc_t       inNewCallback)
{
	WidgetMock* widget_ptr = (WidgetMock*)inWidget;
	widget_ptr->callback = inNewCallback;
}

/* if widget_id is NULL we invoke the callbacks of all widgets */
int test_invoke_widget_callback(XPWidgetMessage message, XPWidgetID widget_id, intptr_t param1, intptr_t param2)
{
	WidgetMock* widget_ptr = (WidgetMock*)widget_id;
	if (!widget_ptr)
	{
		for (const auto& widget : test_widgets)
		{
			if (widget.callback != NULL)
				widget.callback(message, (XPWidgetID) &widget, param1, param2);
		}
		return 1;
	}
	else
	{
		if (!widget_ptr->callback)
			return 0;

		return widget_ptr->callback(message, widget_id, param1, param2);
	}
}

extern XPWidgetID XPCreateWidget(
	int                  inLeft,
	int                  inTop,
	int                  inRight,
	int                  inBottom,
	int                  inVisible,
	const char* inDescriptor,
	int                  inIsRoot,
	XPWidgetID           inContainer,
	XPWidgetClass        inClass)
{
	test_widgets.emplace_back();
	return (XPWidgetID)&test_widgets.back();
}

extern XPWidgetID XPCreateCustomWidget(
	int                  inLeft,
	int                  inTop,
	int                  inRight,
	int                  inBottom,
	int                  inVisible,
	const char* inDescriptor,
	int                  inIsRoot,
	XPWidgetID           inContainer,
	XPWidgetFunc_t       inCallback)
{
	test_widgets.emplace_back();
	test_widgets.back().callback = inCallback;
	
	return (XPWidgetID)&test_widgets.back();
}

extern void XPDestroyWidget(
	XPWidgetID           inWidget,
	int                  inDestroyChildren)
{

}

extern void XPShowWidget(XPWidgetID inWidget)
{
	WidgetMock* widget_ptr = (WidgetMock*)inWidget;
	widget_ptr->visible = true;
}

extern void XPHideWidget( XPWidgetID inWidget )
{
	WidgetMock* widget_ptr = (WidgetMock*)inWidget;
	widget_ptr->visible = false;
}

extern void XPSetWidgetProperty(
	XPWidgetID           inWidget,
	XPWidgetPropertyID   inProperty,
	intptr_t             inValue)
{
	WidgetMock* widget_ptr = (WidgetMock*)inWidget;
	widget_ptr->properties[inProperty] = inValue;
}

extern intptr_t XPGetWidgetProperty(
	XPWidgetID           inWidget,
	XPWidgetPropertyID   inProperty,
	int* inExists)
{
	WidgetMock* widget_ptr = (WidgetMock*)inWidget;
	if (widget_ptr != NULL && widget_ptr->properties.count(inProperty) > 0)
	{
		if (inExists != NULL)
			*inExists = 1;

		return widget_ptr->properties[inProperty];
	}
	else if (inExists != NULL)
	{
		*inExists = 0;
		return 0;
	}
	else
	{
		return 0;
	}
}

extern void       XPLMGetFontDimensions(
	XPLMFontID           inFontID,
	int* outCharWidth,           /* Can be NULL */
	int* outCharHeight,          /* Can be NULL */
	int* outDigitsOnly)
{
	if (outCharWidth)
		*outCharWidth = 5;
	
	if (outCharHeight)
		*outCharHeight = 6;
	
	if (outDigitsOnly)
		*outDigitsOnly = 0;
}

extern void       XPGetWidgetGeometry(
	XPWidgetID           inWidget,
	int* outLeft,                /* Can be NULL */
	int* outTop,                 /* Can be NULL */
	int* outRight,               /* Can be NULL */
	int* outBottom)
{
	if (outLeft)
		*outLeft = 10;

	if (outTop)
		*outTop = 20;

	if (outRight)
		*outRight = 610;

	if (outBottom)
		*outBottom = 620;
}