// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "os.h"

#include "IrrCompileConfig.h"
#include "irr/core/math/irrMath.h"


#if defined(_IRR_WINDOWS_API_)
// ----------------------------------------------------------------
// Windows specific functions
// ----------------------------------------------------------------
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <time.h>

namespace irr
{
namespace os
{
	//! prints a debuginfo string
	void Printer::print(const std::string& message)
	{
		std::string tmp(message);
		tmp += "\n";
		OutputDebugStringA(tmp.c_str());
		printf("%s", tmp.c_str());
	}
} // end namespace os


#else

// ----------------------------------------------------------------
// linux/ansi version
// ----------------------------------------------------------------

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

namespace irr
{
namespace os
{

	//! prints a debuginfo string
	void Printer::print(const std::string& message)
	{
		printf("%s\n", message.c_str());
	}

} // end namespace os

#endif // end linux / windows

namespace os
{
	// The platform independent implementation of the printer
	ILogger* Printer::Logger = 0;

	void Printer::log(const std::string& message, ELOG_LEVEL ll)
	{
		if (Logger)
			Logger->log(message, ll);
	}

	void Printer::log(const std::wstring& message, ELOG_LEVEL ll)
	{
		if (Logger)
			Logger->log(message, ll);
	}

	void Printer::log(const std::string& message, const std::string& hint, ELOG_LEVEL ll)
	{
		if (Logger)
			Logger->log(message, hint, ll);
	}

} // end namespace os
} // end namespace irr


