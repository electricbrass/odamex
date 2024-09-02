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

#pragma once
#include <string>

class Fl_Image;
class Fl_Window;
typedef int Fl_Font;

void GUI_SetIcon(Fl_Window* win);

struct GUIRes
{
	static Fl_Image* icon_odamex_128();
};

class GUI_FontLoader
{
  private:
	int m_font_loaded;
	std::string m_loaded_path;
	const Fl_Font m_font;
  public:
	Fl_Font font();
	bool load_font(std::string path, std::string name);
	GUI_FontLoader(Fl_Font f);
	~GUI_FontLoader();
};