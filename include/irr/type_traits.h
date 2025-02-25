// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h
#ifndef __IRR_TYPE_TRAITS_H_INCLUDED__
#define __IRR_TYPE_TRAITS_H_INCLUDED__

#include <type_traits>

namespace irr
{
#if __cplusplus >= 201703L
template<bool B>
using bool_constant = std::bool_constant<B>;
#else
template<bool B>
using bool_constant = std::integral_constant<bool, B>;
#endif // C
}

#endif

