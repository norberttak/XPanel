/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "XPLMGraphics.h"
#include "XPLMPlanes.h"
#include "XPLMDisplay.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"
#include "XPLMMenus.h"
#include "XPLMProcessing.h"

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
