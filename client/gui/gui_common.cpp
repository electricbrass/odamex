// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 2006-2020 by The Odamex Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	Boot GUI for WAD selection.
//
//-----------------------------------------------------------------------------

#include "gui_common.h"

#include "FL/Fl_PNG_Image.H"
#include "FL/Fl_Window.H"

#include "gui_resource.h"

#if defined(_WIN32) && !defined(_XBOX)

#include "FL/x.H"
#include "win32inc.h"

void GUI_SetIcon(Fl_Window* win)
{
	const HICON icon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(1));
	win->icon((const void*)icon);
}

#else

void GUI_SetIcon(Fl_Window* win) { }

#endif

Fl_Image* GUIRes::icon_odamex_128()
{
	static Fl_Image* image = new Fl_PNG_Image("icon_odamex_128", __icon_odamex_128_png,
	                                          __icon_odamex_128_png_len);
	return image;
}

#ifdef _WIN32

#include <windows.h>
#define LOAD_FONT(PATH) (bool)AddFontResourceEx((PATH),FR_PRIVATE,0)
#define UNLOAD_FONT(PATH) RemoveFontResourceEx((PATH),FR_PRIVATE,0)

#elif defined(FONTCONFIG)

#include <fontconfig/fontconfig.h>
#define USE_XFT 1
#define LOAD_FONT(PATH) (bool)FcConfigAppFontAddFile(NULL,(const FcChar8*)(PATH))
#define UNLOAD_FONT(PATH) FcConfigAppFontClear(NULL)

#else

bool LOAD_FONT(const char*) { return false; }
void UNLOAD_FONT(const char*) {}

#endif

GUI_FontLoader::GUI_FontLoader(Fl_Font f) : m_font(f), m_font_loaded(0), m_loaded_path() {}

bool GUI_FontLoader::load_font(std::string path, std::string name)
{
	if (m_font_loaded)
	{
		UNLOAD_FONT(m_loaded_path.c_str());
		m_font_loaded = true;
	}

	m_font_loaded = LOAD_FONT(path.c_str());

	if (m_font_loaded)
	{
		m_loaded_path = path;
		Fl::set_font(m_font, name.c_str());
		return true;
	}
	return false;
}

GUI_FontLoader::~GUI_FontLoader()
{
	if (m_font_loaded)
	{
		UNLOAD_FONT(m_loaded_path.c_str());
	}
}

Fl_Font GUI_FontLoader::font()
{
	return m_font;
}