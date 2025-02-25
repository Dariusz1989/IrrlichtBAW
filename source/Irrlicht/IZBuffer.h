// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __I_Z_BUFFER_H_INCLUDED__
#define __I_Z_BUFFER_H_INCLUDED__

#include "irr/core/IReferenceCounted.h"
#include "dimension2d.h"
#include "S2DVertex.h"

namespace irr
{
namespace video
{
	class IZBuffer : public virtual core::IReferenceCounted
	{
        public:
            //! clears the zbuffer
            virtual void clear() = 0;

            //! sets the new size of the zbuffer
            virtual void setSize(const core::dimension2d<uint32_t>& size) = 0;

            //! returns the size of the zbuffer
            virtual const core::dimension2d<uint32_t>& getSize() const = 0;

            //! locks the zbuffer
            virtual TZBufferType* lock() = 0;

            //! unlocks the zbuffer
            virtual void unlock() = 0;
	};


	//! creates a ZBuffer
	IZBuffer* createZBuffer(const core::dimension2d<uint32_t>& size);

} // end namespace video
} // end namespace irr

#endif

