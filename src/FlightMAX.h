
#ifndef FlightMAX_H
	#define FlightMAX_H

	// (Open)GL
	#include "SystemGL.h"

	// Standard C/C++ header
	#include <stdexcept>
	#include <cstdlib>
	#include <ctime>
	#include <string>
	#include <vector>
	#include <algorithm>

	// X-Plane SDK header
	#include "XPLMDisplay.h"
	#include "XPLMGraphics.h"
	#include "XPLMUtilities.h"
	#include "XPLMProcessing.h"
	#include "XPLMPlugin.h"
	#include "XPLMDataAccess.h"
	#include "XPLMUtilities.h"
	#include "XPLMMenus.h"

	// Dear ImGui and ImgWindow
	#include "imgui.h"
	#include "imgui_stdlib.h"
	#include "ImgWindow.h"

	// Our Window definition
	#include "FlightMAX_starter_window.h"

	// Definitions for OpenFontIcons
	#include "IconsFontAwesome5.h"

	/// Is VR currently enabled?
	extern bool vr_is_enabled;

	/// Calculate window's standard coordinates
	void CalcWinCoords (int& left, int& top, int& right, int& bottom);

#endif // #ifndef FlightMAX_H
