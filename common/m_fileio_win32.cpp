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
//	File Input/Output Operations for windows-like platforms.
//
//-----------------------------------------------------------------------------

#if defined(_WIN32)

#if defined(UNIX)
#error "_WIN32 is mutually exclusive with UNIX"
#endif

#include "m_fileio.h"

#include <string>

#include "win32inc.h"
#include <shlobj.h>
#include <shlwapi.h>

#include "cmdlib.h"
#include "i_system.h"
#include "w_wad.h"

#ifdef UNIX
std::string I_GetHomeDir(std::string user = "")
{
	const char* envhome = getenv("HOME");
	std::string home = envhome ? envhome : "";

	if (!home.length())
	{
#ifdef HAVE_PWD_H
		// try the uid way
		passwd* p = user.length() ? getpwnam(user.c_str()) : getpwuid(getuid());
		if (p && p->pw_dir)
			home = p->pw_dir;
#endif

		if (!home.length())
			I_FatalError("Please set your HOME variable");
	}

	if (home[home.length() - 1] != PATHSEPCHAR)
		home += PATHSEP;

	return home;
}
#endif

std::string M_GetBinaryDir()
{
	std::string ret;

	char tmp[MAX_PATH]; // denis - todo - make separate function
	GetModuleFileName(NULL, tmp, sizeof(tmp));
	ret = tmp;

	M_FixPathSep(ret);

	size_t slash = ret.find_last_of(PATHSEPCHAR);
	if (slash == std::string::npos)
	{
		return "";
	}

	return ret.substr(0, slash);
}

std::string M_GetHomeDir(const std::string& user)
{
	// [AM] Use SHGetKnownFolderPath when we don't feel like supporting
	//      Windows XP anymore.
	TCHAR folderPath[MAX_PATH];
	if (!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, folderPath)))
	{
		I_FatalError("Could not get user's personal folder.\n");
	}

	// Now that we have the Documents folder, just go up one.
	std::string path;
	StrFormat(path, "%s\\..", folderPath);
	return M_CleanPath(path);
}

std::string M_GetUserDir()
{
#if defined(_XBOX)
	return "T:" PATHSEP;
#else
	// [AM] Use SHGetKnownFolderPath when we don't feel like supporting
	//      Windows XP anymore.
	TCHAR folderPath[MAX_PATH];
	if (!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, folderPath)))
	{
		I_FatalError("Could not get user's personal folder.\n");
	}

	std::string path;
	StrFormat(path, "%s\\My Games\\Odamex", folderPath);
	return path;
#endif
}

std::string M_GetWriteDir()
{
#if defined(_XBOX)
	return "T:" PATHSEP;
#else
	// Has Odamex been installed?
	std::string installed = M_GetBinaryDir() + PATHSEP "odamex-installed.txt";
	if (M_FileExists(installed))
	{
		// Does the user folder exist?
		std::string userPath = M_GetUserDir();
		int ok = SHCreateDirectoryEx(NULL, userPath.c_str(), NULL);
		if (ok == ERROR_SUCCESS || ok == ERROR_ALREADY_EXISTS)
		{
			return M_CleanPath(userPath);
		}
		else
		{
			I_FatalError("Failed to create %s directory.\n", userPath.c_str());
		}
	}

	// Our path is relative to the current working directory.
	return M_CleanPath(M_GetCWD());
#endif
}

std::string M_GetUserFileName(const std::string& file)
{
#if defined(_XBOX)
	std::string path = "T:";

	path += PATHSEP;
	path += file;

	return M_CleanPath(path);
#else
	// Is absolute path?  If so, stop here.
	if (!PathIsRelative(file.c_str()))
	{
		return file;
	}

	// Is this an explicitly relative path?  If so, stop here.
	size_t fileLen = file.length();
	if (fileLen >= 2 && file[0] == '.' && M_IsPathSep(file[1]))
	{
		return file;
	}
	else if (fileLen >= 3 && file[0] == '.' && file[1] == '.' && M_IsPathSep(file[2]))
	{
		return file;
	}

	// Direct our path to our write directory.
	std::string path = M_GetWriteDir();
	if (!M_IsPathSep(*(path.end() - 1)))
	{
		path += PATHSEP;
	}
	path += file;

	return path;
#endif
}

std::string M_BaseFileSearchDir(std::string dir, const std::string& file,
                                const std::string& ext, std::string hash)
{
	std::string found;
	dir = M_CleanPath(dir);
	hash = StdStringToUpper(hash);
	std::string dothash;
	if (!hash.empty())
		dothash = "." + hash;

	// denis - list files in the directory of interest, case-desensitize
	// then see if wanted wad is listed
	std::string all_ext = dir + "*";
	// all_ext += ext;

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(all_ext.c_str(), &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE)
		return "";

	do
	{
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;

		std::string tmp = StdStringToUpper(FindFileData.cFileName);

		if (file == tmp || (file + ext) == tmp || (file + dothash) == tmp ||
		    (file + ext + dothash) == tmp)
		{
			std::string local_file(dir + FindFileData.cFileName);
			std::string local_hash(W_MD5(local_file));

			if (hash.empty() || hash == local_hash)
			{
				found = FindFileData.cFileName;
				break;
			}
			else if (!hash.empty())
			{
				Printf(PRINT_WARNING, "WAD at %s does not match required copy\n",
				       local_file.c_str());
				Printf(PRINT_WARNING, "Local MD5: %s\n", local_hash.c_str());
				Printf(PRINT_WARNING, "Required MD5: %s\n\n", hash.c_str());
			}
		}
	} while (FindNextFile(hFind, &FindFileData));

	FindClose(hFind);
	return found;
}

#endif
