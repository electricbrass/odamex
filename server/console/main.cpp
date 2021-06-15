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
//  Entry point
//
//-----------------------------------------------------------------------------

#include "common.h"

#include <FL/Fl.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Window.H>

#include "subproc.h"

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	subproc_t proc = {0};
	WinStartServer(proc);

	const int WIDTH = 640;
	const int HEIGHT = 480;
	const int INPUT_HEIGHT = 24;

	Fl_Window* window = new Fl_Window(WIDTH, HEIGHT);
	{
		Fl_Text_Display* text = new Fl_Text_Display(0, 0, WIDTH, HEIGHT - INPUT_HEIGHT);
		window->add(text);

		Fl_Input* input = new Fl_Input(0, HEIGHT - INPUT_HEIGHT, WIDTH, INPUT_HEIGHT);
		window->add(input);
	}
	window->end();
	window->show(0, NULL);
	return Fl::run();
}
