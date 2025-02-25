// Copyright (C) 2002-2012 Nikolaus Gebhardt / Thomas Alten
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef _C_BLIT_H_INCLUDED_
#define _C_BLIT_H_INCLUDED_

#include "SoftwareDriver2_helper.h"

namespace irr
{

	struct SBlitJob
	{
		AbsRectangle Dest;
		AbsRectangle Source;

		uint32_t argb;

		void * src;
		void * dst;

		int32_t width;
		int32_t height;

		uint32_t srcPitch;
		uint32_t dstPitch;

		uint32_t srcPixelMul;
		uint32_t dstPixelMul;

		bool stretch;
		float x_stretch;
		float y_stretch;

		SBlitJob() : stretch(false) {}
	};

	// Bitfields Cohen Sutherland
	enum eClipCode
	{
		CLIPCODE_EMPTY	=	0,
		CLIPCODE_BOTTOM	=	1,
		CLIPCODE_TOP	=	2,
		CLIPCODE_LEFT	=	4,
		CLIPCODE_RIGHT	=	8
	};

inline uint32_t GetClipCode( const AbsRectangle &r, const core::position2d<int32_t> &p )
{
	uint32_t code = CLIPCODE_EMPTY;

	if ( p.X < r.x0 )
		code = CLIPCODE_LEFT;
	else
	if ( p.X > r.x1 )
		code = CLIPCODE_RIGHT;

	if ( p.Y < r.y0 )
		code |= CLIPCODE_TOP;
	else
	if ( p.Y > r.y1 )
		code |= CLIPCODE_BOTTOM;

	return code;
}


/*!
	Cohen Sutherland clipping
	@return: 1 if valid
*/

static int ClipLine(const AbsRectangle &clipping,
			core::position2d<int32_t> &p0,
			core::position2d<int32_t> &p1,
			const core::position2d<int32_t>& p0_in,
			const core::position2d<int32_t>& p1_in)
{
	uint32_t code0;
	uint32_t code1;
	uint32_t code;

	p0 = p0_in;
	p1 = p1_in;

	code0 = GetClipCode( clipping, p0 );
	code1 = GetClipCode( clipping, p1 );

	// trivial accepted
	while ( code0 | code1 )
	{
		int32_t x=0;
		int32_t y=0;

		// trivial reject
		if ( code0 & code1 )
			return 0;

		if ( code0 )
		{
			// clip first point
			code = code0;
		}
		else
		{
			// clip last point
			code = code1;
		}

		if ( (code & CLIPCODE_BOTTOM) == CLIPCODE_BOTTOM )
		{
			// clip bottom viewport
			y = clipping.y1;
			x = p0.X + ( p1.X - p0.X ) * ( y - p0.Y ) / ( p1.Y - p0.Y );
		}
		else
		if ( (code & CLIPCODE_TOP) == CLIPCODE_TOP )
		{
			// clip to viewport
			y = clipping.y0;
			x = p0.X + ( p1.X - p0.X ) * ( y - p0.Y ) / ( p1.Y - p0.Y );
		}
		else
		if ( (code & CLIPCODE_RIGHT) == CLIPCODE_RIGHT )
		{
			// clip right viewport
			x = clipping.x1;
			y = p0.Y + ( p1.Y - p0.Y ) * ( x - p0.X ) / ( p1.X - p0.X );
		}
		else
		if ( (code & CLIPCODE_LEFT) == CLIPCODE_LEFT )
		{
			// clip left viewport
			x = clipping.x0;
			y = p0.Y + ( p1.Y - p0.Y ) * ( x - p0.X ) / ( p1.X - p0.X );
		}

		if ( code == code0 )
		{
			// modify first point
			p0.X = x;
			p0.Y = y;
			code0 = GetClipCode( clipping, p0 );
		}
		else
		{
			// modify second point
			p1.X = x;
			p1.Y = y;
			code1 = GetClipCode( clipping, p1 );
		}
	}

	return 1;
}

/*
*/
inline void GetClip(AbsRectangle &clipping, video::IImage * t)
{
	clipping.x0 = 0;
	clipping.y0 = 0;
	clipping.x1 = t->getDimension().Width - 1;
	clipping.y1 = t->getDimension().Height - 1;
}

/*
	return alpha in [0;256] Granularity from 32-Bit ARGB
	add highbit alpha ( alpha > 127 ? + 1 )
*/
static inline uint32_t extractAlpha(const uint32_t c)
{
	return ( c >> 24 ) + ( c >> 31 );
}

/*
	return alpha in [0;255] Granularity and 32-Bit ARGB
	add highbit alpha ( alpha > 127 ? + 1 )
*/
static inline uint32_t packAlpha(const uint32_t c)
{
	return (c > 127 ? c - 1 : c) << 24;
}


/*!
	Scale Color by (1/value)
	value 0 - 256 ( alpha )
*/
inline uint32_t PixelLerp32(const uint32_t source, const uint32_t value)
{
	uint32_t srcRB = source & 0x00FF00FF;
	uint32_t srcXG = (source & 0xFF00FF00) >> 8;

	srcRB *= value;
	srcXG *= value;

	srcRB >>= 8;
	//srcXG >>= 8;

	srcXG &= 0xFF00FF00;
	srcRB &= 0x00FF00FF;

	return srcRB | srcXG;
}


/*
*/
static void RenderLine32_Decal(video::IImage *t,
				const core::position2d<int32_t> &p0,
				const core::position2d<int32_t> &p1,
				uint32_t argb )
{
	int32_t dx = p1.X - p0.X;
	int32_t dy = p1.Y - p0.Y;

	int32_t c;
	int32_t m;
	int32_t d = 0;
	int32_t run;

	int32_t xInc = 4;
	int32_t yInc = (int32_t) t->getPitch();

	if ( dx < 0 )
	{
		xInc = -xInc;
		dx = -dx;
	}

	if ( dy < 0 )
	{
		yInc = -yInc;
		dy = -dy;
	}

	uint32_t *dst;
	dst = (uint32_t*) ( (uint8_t*) t->getData() + ( p0.Y * t->getPitch() ) + ( p0.X << 2 ) );

	if ( dy > dx )
	{
		int32_t tmp;
		tmp = dx;
		dx = dy;
		dy = tmp;
		tmp = xInc;
		xInc = yInc;
		yInc = tmp;
	}

	c = dx << 1;
	m = dy << 1;

	run = dx;
	do
	{
		*dst = argb;

		dst = (uint32_t*) ( (uint8_t*) dst + xInc );	// x += xInc
		d += m;
		if ( d > dx )
		{
			dst = (uint32_t*) ( (uint8_t*) dst + yInc );	// y += yInc
			d -= c;
		}
		run -= 1;
	} while (run>=0);
}


/*
*/
static void RenderLine32_Blend(video::IImage *t,
				const core::position2d<int32_t> &p0,
				const core::position2d<int32_t> &p1,
				uint32_t argb, uint32_t alpha)
{
	int32_t dx = p1.X - p0.X;
	int32_t dy = p1.Y - p0.Y;

	int32_t c;
	int32_t m;
	int32_t d = 0;
	int32_t run;

	int32_t xInc = 4;
	int32_t yInc = (int32_t) t->getPitch();

	if ( dx < 0 )
	{
		xInc = -xInc;
		dx = -dx;
	}

	if ( dy < 0 )
	{
		yInc = -yInc;
		dy = -dy;
	}

	uint32_t *dst;
	dst = (uint32_t*) ( (uint8_t*) t->getData() + ( p0.Y * t->getPitch() ) + ( p0.X << 2 ) );

	if ( dy > dx )
	{
		int32_t tmp;
		tmp = dx;
		dx = dy;
		dy = tmp;
		tmp = xInc;
		xInc = yInc;
		yInc = tmp;
	}

	c = dx << 1;
	m = dy << 1;

	run = dx;
	const uint32_t packA = packAlpha ( alpha );
	do
	{
		*dst = packA | PixelBlend32( *dst, argb, alpha );

		dst = (uint32_t*) ( (uint8_t*) dst + xInc );	// x += xInc
		d += m;
		if ( d > dx )
		{
			dst = (uint32_t*) ( (uint8_t*) dst + yInc );	// y += yInc
			d -= c;
		}
		run -= 1;
	} while (run>=0);
}

/*
*/
static void RenderLine16_Decal(video::IImage *t,
				const core::position2d<int32_t> &p0,
				const core::position2d<int32_t> &p1,
				uint32_t argb )
{
	int32_t dx = p1.X - p0.X;
	int32_t dy = p1.Y - p0.Y;

	int32_t c;
	int32_t m;
	int32_t d = 0;
	int32_t run;

	int32_t xInc = 2;
	int32_t yInc = (int32_t) t->getPitch();

	if ( dx < 0 )
	{
		xInc = -xInc;
		dx = -dx;
	}

	if ( dy < 0 )
	{
		yInc = -yInc;
		dy = -dy;
	}

	uint16_t *dst;
	dst = (uint16_t*) ( (uint8_t*) t->getData() + ( p0.Y * t->getPitch() ) + ( p0.X << 1 ) );

	if ( dy > dx )
	{
		int32_t tmp;
		tmp = dx;
		dx = dy;
		dy = tmp;
		tmp = xInc;
		xInc = yInc;
		yInc = tmp;
	}

	c = dx << 1;
	m = dy << 1;

	run = dx;
	do
	{
		*dst = (uint16_t)argb;

		dst = (uint16_t*) ( (uint8_t*) dst + xInc );	// x += xInc
		d += m;
		if ( d > dx )
		{
			dst = (uint16_t*) ( (uint8_t*) dst + yInc );	// y += yInc
			d -= c;
		}
		run -= 1;
	} while (run>=0);
}

/*
*/
static void RenderLine16_Blend(video::IImage *t,
				const core::position2d<int32_t> &p0,
				const core::position2d<int32_t> &p1,
				uint16_t argb,
				uint16_t alpha)
{
	int32_t dx = p1.X - p0.X;
	int32_t dy = p1.Y - p0.Y;

	int32_t c;
	int32_t m;
	int32_t d = 0;
	int32_t run;

	int32_t xInc = 2;
	int32_t yInc = (int32_t) t->getPitch();

	if ( dx < 0 )
	{
		xInc = -xInc;
		dx = -dx;
	}

	if ( dy < 0 )
	{
		yInc = -yInc;
		dy = -dy;
	}

	uint16_t *dst;
	dst = (uint16_t*) ( (uint8_t*) t->getData() + ( p0.Y * t->getPitch() ) + ( p0.X << 1 ) );

	if ( dy > dx )
	{
		int32_t tmp;
		tmp = dx;
		dx = dy;
		dy = tmp;
		tmp = xInc;
		xInc = yInc;
		yInc = tmp;
	}

	c = dx << 1;
	m = dy << 1;

	run = dx;
	const uint16_t packA = alpha ? 0x8000 : 0;
	do
	{
		*dst = packA | PixelBlend16( *dst, argb, alpha );

		dst = (uint16_t*) ( (uint8_t*) dst + xInc );	// x += xInc
		d += m;
		if ( d > dx )
		{
			dst = (uint16_t*) ( (uint8_t*) dst + yInc );	// y += yInc
			d -= c;
		}
		run -= 1;
	} 	while (run>=0);
}


/*!
*/
static void executeBlit_TextureCopy_x_to_x( const SBlitJob * job )
{
	const uint32_t w = job->width;
	const uint32_t h = job->height;
	if (job->stretch)
	{
		const uint32_t *src = static_cast<const uint32_t*>(job->src);
		uint32_t *dst = static_cast<uint32_t*>(job->dst);
		const float wscale = 1.f/job->x_stretch;
		const float hscale = 1.f/job->y_stretch;

		for ( uint32_t dy = 0; dy < h; ++dy )
		{
			const uint32_t src_y = (uint32_t)(dy*hscale);
			src = (uint32_t*) ( (uint8_t*) (job->src) + job->srcPitch*src_y );

			for ( uint32_t dx = 0; dx < w; ++dx )
			{
				const uint32_t src_x = (uint32_t)(dx*wscale);
				dst[dx] = src[src_x];
			}
			dst = (uint32_t*) ( (uint8_t*) (dst) + job->dstPitch );
		}
	}
	else
	{
		const uint32_t widthPitch = job->width * job->dstPixelMul;
		const void *src = (void*) job->src;
		void *dst = (void*) job->dst;

		for ( uint32_t dy = 0; dy != h; ++dy )
		{
			memcpy( dst, src, widthPitch );

			src = (void*) ( (uint8_t*) (src) + job->srcPitch );
			dst = (void*) ( (uint8_t*) (dst) + job->dstPitch );
		}
	}
}

/*!
*/
static void executeBlit_TextureCopy_32_to_16( const SBlitJob * job )
{
	const uint32_t w = job->width;
	const uint32_t h = job->height;
	const uint32_t *src = static_cast<const uint32_t*>(job->src);
	uint16_t *dst = static_cast<uint16_t*>(job->dst);

	if (job->stretch)
	{
		const float wscale = 1.f/job->x_stretch;
		const float hscale = 1.f/job->y_stretch;

		for ( uint32_t dy = 0; dy < h; ++dy )
		{
			const uint32_t src_y = (uint32_t)(dy*hscale);
			src = (uint32_t*) ( (uint8_t*) (job->src) + job->srcPitch*src_y );

			for ( uint32_t dx = 0; dx < w; ++dx )
			{
				const uint32_t src_x = (uint32_t)(dx*wscale);
				//16 bit Blitter depends on pre-multiplied color
				const uint32_t s = PixelLerp32( src[src_x] | 0xFF000000, extractAlpha( src[src_x] ) );
				dst[dx] = video::A8R8G8B8toA1R5G5B5( s );
			}
			dst = (uint16_t*) ( (uint8_t*) (dst) + job->dstPitch );
		}
	}
	else
	{
		for ( uint32_t dy = 0; dy != h; ++dy )
		{
			for ( uint32_t dx = 0; dx != w; ++dx )
			{
				//16 bit Blitter depends on pre-multiplied color
				const uint32_t s = PixelLerp32( src[dx] | 0xFF000000, extractAlpha( src[dx] ) );
				dst[dx] = video::A8R8G8B8toA1R5G5B5( s );
			}

			src = (uint32_t*) ( (uint8_t*) (src) + job->srcPitch );
			dst = (uint16_t*) ( (uint8_t*) (dst) + job->dstPitch );
		}
	}
}

/*!
*/
static void executeBlit_TextureCopy_24_to_16( const SBlitJob * job )
{
	const uint32_t w = job->width;
	const uint32_t h = job->height;
	const uint8_t *src = static_cast<const uint8_t*>(job->src);
	uint16_t *dst = static_cast<uint16_t*>(job->dst);

	if (job->stretch)
	{
		const float wscale = 3.f/job->x_stretch;
		const float hscale = 1.f/job->y_stretch;

		for ( uint32_t dy = 0; dy < h; ++dy )
		{
			const uint32_t src_y = (uint32_t)(dy*hscale);
			src = (uint8_t*)(job->src) + job->srcPitch*src_y;

			for ( uint32_t dx = 0; dx < w; ++dx )
			{
				const uint8_t* src_x = src+(uint32_t)(dx*wscale);
				dst[dx] = video::RGBA16(src_x[0], src_x[1], src_x[2]);
			}
			dst = (uint16_t*) ( (uint8_t*) (dst) + job->dstPitch );
		}
	}
	else
	{
		for ( uint32_t dy = 0; dy != h; ++dy )
		{
			const uint8_t* s = src;
			for ( uint32_t dx = 0; dx != w; ++dx )
			{
				dst[dx] = video::RGBA16(s[0], s[1], s[2]);
				s += 3;
			}

			src = src+job->srcPitch;
			dst = (uint16_t*) ( (uint8_t*) (dst) + job->dstPitch );
		}
	}
}


/*!
*/
static void executeBlit_TextureCopy_16_to_32( const SBlitJob * job )
{
	const uint32_t w = job->width;
	const uint32_t h = job->height;
	const uint16_t *src = static_cast<const uint16_t*>(job->src);
	uint32_t *dst = static_cast<uint32_t*>(job->dst);

	if (job->stretch)
	{
		const float wscale = 1.f/job->x_stretch;
		const float hscale = 1.f/job->y_stretch;

		for ( uint32_t dy = 0; dy < h; ++dy )
		{
			const uint32_t src_y = (uint32_t)(dy*hscale);
			src = (uint16_t*) ( (uint8_t*) (job->src) + job->srcPitch*src_y );

			for ( uint32_t dx = 0; dx < w; ++dx )
			{
				const uint32_t src_x = (uint32_t)(dx*wscale);
				dst[dx] = video::A1R5G5B5toA8R8G8B8(src[src_x]);
			}
			dst = (uint32_t*) ( (uint8_t*) (dst) + job->dstPitch );
		}
	}
	else
	{
		for ( uint32_t dy = 0; dy != h; ++dy )
		{
			for ( uint32_t dx = 0; dx != w; ++dx )
			{
				dst[dx] = video::A1R5G5B5toA8R8G8B8( src[dx] );
			}

			src = (uint16_t*) ( (uint8_t*) (src) + job->srcPitch );
			dst = (uint32_t*) ( (uint8_t*) (dst) + job->dstPitch );
		}
	}
}

static void executeBlit_TextureCopy_16_to_24( const SBlitJob * job )
{
	const uint32_t w = job->width;
	const uint32_t h = job->height;
	const uint16_t *src = static_cast<const uint16_t*>(job->src);
	uint8_t *dst = static_cast<uint8_t*>(job->dst);

	if (job->stretch)
	{
		const float wscale = 1.f/job->x_stretch;
		const float hscale = 1.f/job->y_stretch;

		for ( uint32_t dy = 0; dy < h; ++dy )
		{
			const uint32_t src_y = (uint32_t)(dy*hscale);
			src = (uint16_t*) ( (uint8_t*) (job->src) + job->srcPitch*src_y );

			for ( uint32_t dx = 0; dx < w; ++dx )
			{
				const uint32_t src_x = (uint32_t)(dx*wscale);
				uint32_t color = video::A1R5G5B5toA8R8G8B8(src[src_x]);
				uint8_t * writeTo = &dst[dx * 3];
				*writeTo++ = (color >> 16)& 0xFF;
				*writeTo++ = (color >> 8) & 0xFF;
				*writeTo++ = color & 0xFF;
			}
			dst += job->dstPitch;
		}
	}
	else
	{
		for ( uint32_t dy = 0; dy != h; ++dy )
		{
			for ( uint32_t dx = 0; dx != w; ++dx )
			{
				uint32_t color = video::A1R5G5B5toA8R8G8B8(src[dx]);
				uint8_t * writeTo = &dst[dx * 3];
				*writeTo++ = (color >> 16)& 0xFF;
				*writeTo++ = (color >> 8) & 0xFF;
				*writeTo++ = color & 0xFF;
			}

			src = (uint16_t*) ( (uint8_t*) (src) + job->srcPitch );
			dst += job->dstPitch;
		}
	}
}

/*!
*/
static void executeBlit_TextureCopy_24_to_32( const SBlitJob * job )
{
	const uint32_t w = job->width;
	const uint32_t h = job->height;
	const uint8_t *src = static_cast<const uint8_t*>(job->src);
	uint32_t *dst = static_cast<uint32_t*>(job->dst);

	if (job->stretch)
	{
		const float wscale = 3.f/job->x_stretch;
		const float hscale = 1.f/job->y_stretch;

		for ( uint32_t dy = 0; dy < h; ++dy )
		{
			const uint32_t src_y = (uint32_t)(dy*hscale);
			src = (const uint8_t*)job->src+(job->srcPitch*src_y);

			for ( uint32_t dx = 0; dx < w; ++dx )
			{
				const uint8_t* s = src+(uint32_t)(dx*wscale);
				dst[dx] = 0xFF000000 | s[0] << 16 | s[1] << 8 | s[2];
			}
			dst = (uint32_t*) ( (uint8_t*) (dst) + job->dstPitch );
		}
	}
	else
	{
		for ( int32_t dy = 0; dy != job->height; ++dy )
		{
			const uint8_t* s = src;

			for ( int32_t dx = 0; dx != job->width; ++dx )
			{
				dst[dx] = 0xFF000000 | s[0] << 16 | s[1] << 8 | s[2];
				s += 3;
			}

			src = src + job->srcPitch;
			dst = (uint32_t*) ( (uint8_t*) (dst) + job->dstPitch );
		}
	}
}

static void executeBlit_TextureCopy_32_to_24( const SBlitJob * job )
{
	const uint32_t w = job->width;
	const uint32_t h = job->height;
	const uint32_t *src = static_cast<const uint32_t*>(job->src);
	uint8_t *dst = static_cast<uint8_t*>(job->dst);

	if (job->stretch)
	{
		const float wscale = 1.f/job->x_stretch;
		const float hscale = 1.f/job->y_stretch;

		for ( uint32_t dy = 0; dy < h; ++dy )
		{
			const uint32_t src_y = (uint32_t)(dy*hscale);
			src = (uint32_t*) ( (uint8_t*) (job->src) + job->srcPitch*src_y);

			for ( uint32_t dx = 0; dx < w; ++dx )
			{
				const uint32_t src_x = src[(uint32_t)(dx*wscale)];
				uint8_t * writeTo = &dst[dx * 3];
				*writeTo++ = (src_x >> 16)& 0xFF;
				*writeTo++ = (src_x >> 8) & 0xFF;
				*writeTo++ = src_x & 0xFF;
			}
			dst += job->dstPitch;
		}
	}
	else
	{
		for ( uint32_t dy = 0; dy != h; ++dy )
		{
			for ( uint32_t dx = 0; dx != w; ++dx )
			{
				uint8_t * writeTo = &dst[dx * 3];
				*writeTo++ = (src[dx] >> 16)& 0xFF;
				*writeTo++ = (src[dx] >> 8) & 0xFF;
				*writeTo++ = src[dx] & 0xFF;
			}

			src = (uint32_t*) ( (uint8_t*) (src) + job->srcPitch );
			dst += job->dstPitch;
		}
	}
}

/*!
*/
static void executeBlit_TextureBlend_16_to_16( const SBlitJob * job )
{
	const uint32_t w = job->width;
	const uint32_t h = job->height;
	const uint32_t rdx = w>>1;

	const uint32_t *src = (uint32_t*) job->src;
	uint32_t *dst = (uint32_t*) job->dst;

	if (job->stretch)
	{
		const float wscale = 1.f/job->x_stretch;
		const float hscale = 1.f/job->y_stretch;
		const uint32_t off = core::if_c_a_else_b(w&1, (uint32_t)((w-1)*wscale), 0);
		for ( uint32_t dy = 0; dy < h; ++dy )
		{
			const uint32_t src_y = (uint32_t)(dy*hscale);
			src = (uint32_t*) ( (uint8_t*) (job->src) + job->srcPitch*src_y );

			for ( uint32_t dx = 0; dx < rdx; ++dx )
			{
				const uint32_t src_x = (uint32_t)(dx*wscale);
				dst[dx] = PixelBlend16_simd( dst[dx], src[src_x] );
			}
			if ( off )
			{
				((uint16_t*) dst)[off] = PixelBlend16( ((uint16_t*) dst)[off], ((uint16_t*) src)[off] );
			}

			dst = (uint32_t*) ( (uint8_t*) (dst) + job->dstPitch );
		}
	}
	else
	{
		const uint32_t off = core::if_c_a_else_b(w&1, w-1, 0);
		for (uint32_t dy = 0; dy != h; ++dy )
		{
			for (uint32_t dx = 0; dx != rdx; ++dx )
			{
				dst[dx] = PixelBlend16_simd( dst[dx], src[dx] );
			}

			if ( off )
			{
				((uint16_t*) dst)[off] = PixelBlend16( ((uint16_t*) dst)[off], ((uint16_t*) src)[off] );
			}

			src = (uint32_t*) ( (uint8_t*) (src) + job->srcPitch );
			dst = (uint32_t*) ( (uint8_t*) (dst) + job->dstPitch );
		}
	}
}

/*!
*/
static void executeBlit_TextureBlend_32_to_32( const SBlitJob * job )
{
	const uint32_t w = job->width;
	const uint32_t h = job->height;
	const uint32_t *src = (uint32_t*) job->src;
	uint32_t *dst = (uint32_t*) job->dst;

	if (job->stretch)
	{
		const float wscale = 1.f/job->x_stretch;
		const float hscale = 1.f/job->y_stretch;
		for ( uint32_t dy = 0; dy < h; ++dy )
		{
			const uint32_t src_y = (uint32_t)(dy*hscale);
			src = (uint32_t*) ( (uint8_t*) (job->src) + job->srcPitch*src_y );

			for ( uint32_t dx = 0; dx < w; ++dx )
			{
				const uint32_t src_x = (uint32_t)(dx*wscale);
				dst[dx] = PixelBlend32( dst[dx], src[src_x] );
			}

			dst = (uint32_t*) ( (uint8_t*) (dst) + job->dstPitch );
		}
	}
	else
	{
		for ( uint32_t dy = 0; dy != h; ++dy )
		{
			for ( uint32_t dx = 0; dx != w; ++dx )
			{
				dst[dx] = PixelBlend32( dst[dx], src[dx] );
			}
			src = (uint32_t*) ( (uint8_t*) (src) + job->srcPitch );
			dst = (uint32_t*) ( (uint8_t*) (dst) + job->dstPitch );
		}
	}
}

/*!
*/
static void executeBlit_TextureBlendColor_16_to_16( const SBlitJob * job )
{
	uint16_t *src = (uint16_t*) job->src;
	uint16_t *dst = (uint16_t*) job->dst;

	uint16_t blend = video::A8R8G8B8toA1R5G5B5 ( job->argb );
	for ( int32_t dy = 0; dy != job->height; ++dy )
	{
		for ( int32_t dx = 0; dx != job->width; ++dx )
		{
			if ( 0 == (src[dx] & 0x8000) )
				continue;

			dst[dx] = PixelMul16_2( src[dx], blend );
		}
		src = (uint16_t*) ( (uint8_t*) (src) + job->srcPitch );
		dst = (uint16_t*) ( (uint8_t*) (dst) + job->dstPitch );
	}
}


/*!
*/
static void executeBlit_TextureBlendColor_32_to_32( const SBlitJob * job )
{
	uint32_t *src = (uint32_t*) job->src;
	uint32_t *dst = (uint32_t*) job->dst;

	for ( int32_t dy = 0; dy != job->height; ++dy )
	{
		for ( int32_t dx = 0; dx != job->width; ++dx )
		{
			dst[dx] = PixelBlend32( dst[dx], PixelMul32_2( src[dx], job->argb ) );
		}
		src = (uint32_t*) ( (uint8_t*) (src) + job->srcPitch );
		dst = (uint32_t*) ( (uint8_t*) (dst) + job->dstPitch );
	}
}

/*!
*/
static void executeBlit_Color_16_to_16( const SBlitJob * job )
{
	const uint16_t c = video::A8R8G8B8toA1R5G5B5(job->argb);
	uint16_t *dst = (uint16_t*) job->dst;

	for ( int32_t dy = 0; dy != job->height; ++dy )
	{
		memset16(dst, c, job->srcPitch);
		dst = (uint16_t*) ( (uint8_t*) (dst) + job->dstPitch );
	}
}

/*!
*/
static void executeBlit_Color_32_to_32( const SBlitJob * job )
{
	uint32_t *dst = (uint32_t*) job->dst;

	for ( int32_t dy = 0; dy != job->height; ++dy )
	{
		memset32( dst, job->argb, job->srcPitch );
		dst = (uint32_t*) ( (uint8_t*) (dst) + job->dstPitch );
	}
}

/*!
*/
static void executeBlit_ColorAlpha_16_to_16( const SBlitJob * job )
{
	uint16_t *dst = (uint16_t*) job->dst;

	const uint16_t alpha = extractAlpha( job->argb ) >> 3;
	if ( 0 == alpha )
		return;
	const uint32_t src = video::A8R8G8B8toA1R5G5B5( job->argb );

	for ( int32_t dy = 0; dy != job->height; ++dy )
	{
		for ( int32_t dx = 0; dx != job->width; ++dx )
		{
			dst[dx] = 0x8000 | PixelBlend16( dst[dx], src, alpha );
		}
		dst = (uint16_t*) ( (uint8_t*) (dst) + job->dstPitch );
	}
}

/*!
*/
static void executeBlit_ColorAlpha_32_to_32( const SBlitJob * job )
{
	uint32_t *dst = (uint32_t*) job->dst;

	const uint32_t alpha = extractAlpha( job->argb );
	const uint32_t src = job->argb;

	for ( int32_t dy = 0; dy != job->height; ++dy )
	{
		for ( int32_t dx = 0; dx != job->width; ++dx )
		{
			dst[dx] = (job->argb & 0xFF000000 ) | PixelBlend32( dst[dx], src, alpha );
		}
		dst = (uint32_t*) ( (uint8_t*) (dst) + job->dstPitch );
	}
}

// Blitter Operation
enum eBlitter
{
	BLITTER_INVALID = 0,
	BLITTER_COLOR,
	BLITTER_COLOR_ALPHA,
	BLITTER_TEXTURE,
	BLITTER_TEXTURE_ALPHA_BLEND,
	BLITTER_TEXTURE_ALPHA_COLOR_BLEND
};

typedef void (*tExecuteBlit) ( const SBlitJob * job );


/*!
*/
struct blitterTable
{
	eBlitter operation;
	int32_t destFormat;
	int32_t sourceFormat;
	tExecuteBlit func;
};

static const blitterTable blitTable[] =
{
	{ BLITTER_TEXTURE, -2, -2, executeBlit_TextureCopy_x_to_x },
	{ BLITTER_TEXTURE, asset::EF_A1R5G5B5_UNORM_PACK16, asset::EF_B8G8R8A8_UNORM, executeBlit_TextureCopy_32_to_16 },
	{ BLITTER_TEXTURE, asset::EF_A1R5G5B5_UNORM_PACK16, asset::EF_R8G8B8_UNORM, executeBlit_TextureCopy_24_to_16 },
	{ BLITTER_TEXTURE, asset::EF_B8G8R8A8_UNORM, asset::EF_A1R5G5B5_UNORM_PACK16, executeBlit_TextureCopy_16_to_32 },
	{ BLITTER_TEXTURE, asset::EF_B8G8R8A8_UNORM, asset::EF_R8G8B8_UNORM, executeBlit_TextureCopy_24_to_32 },
	{ BLITTER_TEXTURE, asset::EF_R8G8B8_UNORM, asset::EF_A1R5G5B5_UNORM_PACK16, executeBlit_TextureCopy_16_to_24 },
	{ BLITTER_TEXTURE, asset::EF_R8G8B8_UNORM, asset::EF_B8G8R8A8_UNORM, executeBlit_TextureCopy_32_to_24 },
	{ BLITTER_TEXTURE_ALPHA_BLEND, asset::EF_A1R5G5B5_UNORM_PACK16, asset::EF_A1R5G5B5_UNORM_PACK16, executeBlit_TextureBlend_16_to_16 },
	{ BLITTER_TEXTURE_ALPHA_BLEND, asset::EF_B8G8R8A8_UNORM, asset::EF_B8G8R8A8_UNORM, executeBlit_TextureBlend_32_to_32 },
	{ BLITTER_TEXTURE_ALPHA_COLOR_BLEND, asset::EF_A1R5G5B5_UNORM_PACK16, asset::EF_A1R5G5B5_UNORM_PACK16, executeBlit_TextureBlendColor_16_to_16 },
	{ BLITTER_TEXTURE_ALPHA_COLOR_BLEND, asset::EF_B8G8R8A8_UNORM, asset::EF_B8G8R8A8_UNORM, executeBlit_TextureBlendColor_32_to_32 },
	{ BLITTER_COLOR, asset::EF_A1R5G5B5_UNORM_PACK16, -1, executeBlit_Color_16_to_16 },
	{ BLITTER_COLOR, asset::EF_B8G8R8A8_UNORM, -1, executeBlit_Color_32_to_32 },
	{ BLITTER_COLOR_ALPHA, asset::EF_A1R5G5B5_UNORM_PACK16, -1, executeBlit_ColorAlpha_16_to_16 },
	{ BLITTER_COLOR_ALPHA, asset::EF_B8G8R8A8_UNORM, -1, executeBlit_ColorAlpha_32_to_32 },
	{ BLITTER_INVALID, -1, -1, 0 }
};


static inline tExecuteBlit getBlitter2( eBlitter operation,const video::IImage * dest,const video::IImage * source )
{
    asset::E_FORMAT sourceFormat = (asset::E_FORMAT) ( source ? source->getColorFormat() : -1 );
    asset::E_FORMAT destFormat = (asset::E_FORMAT) ( dest ? dest->getColorFormat() : -1 );

	const blitterTable * b = blitTable;

	while ( b->operation != BLITTER_INVALID )
	{
		if ( b->operation == operation )
		{
			if (( b->destFormat == -1 || b->destFormat == destFormat ) &&
				( b->sourceFormat == -1 || b->sourceFormat == sourceFormat ) )
					return b->func;
			else
			if ( b->destFormat == -2 && ( sourceFormat == destFormat ) )
					return b->func;
		}
		b += 1;
	}
	return 0;
}


// bounce clipping to texture
inline void setClip ( AbsRectangle &out, const core::rect<int32_t> *clip,
					 const video::IImage * tex, int32_t passnative )
{
	if ( clip && 0 == tex && passnative )
	{
		out.x0 = clip->UpperLeftCorner.X;
		out.x1 = clip->LowerRightCorner.X;
		out.y0 = clip->UpperLeftCorner.Y;
		out.y1 = clip->LowerRightCorner.Y;
		return;
	}

	const int32_t w = tex ? tex->getDimension().Width : 0;
	const int32_t h = tex ? tex->getDimension().Height : 0;
	if ( clip )
	{
		out.x0 = core::s32_clamp ( clip->UpperLeftCorner.X, 0, w );
		out.x1 = core::s32_clamp ( clip->LowerRightCorner.X, out.x0, w );
		out.y0 = core::s32_clamp ( clip->UpperLeftCorner.Y, 0, h );
		out.y1 = core::s32_clamp ( clip->LowerRightCorner.Y, out.y0, h );
	}
	else
	{
		out.x0 = 0;
		out.y0 = 0;
		out.x1 = w;
		out.y1 = h;
	}

}

/*!
	a generic 2D Blitter
*/
static int32_t Blit(eBlitter operation,
		video::IImage * dest,
		const core::rect<int32_t> *destClipping,
		const core::position2d<int32_t> *destPos,
		video::IImage * const source,
		const core::rect<int32_t> *sourceClipping,
		uint32_t argb)
{
	tExecuteBlit blitter = getBlitter2( operation, dest, source );
	if ( 0 == blitter )
	{
		return 0;
	}

	// Clipping
	AbsRectangle sourceClip;
	AbsRectangle destClip;
	AbsRectangle v;

	SBlitJob job;

	setClip ( sourceClip, sourceClipping, source, 1 );
	setClip ( destClip, destClipping, dest, 0 );

	v.x0 = destPos ? destPos->X : 0;
	v.y0 = destPos ? destPos->Y : 0;
	v.x1 = v.x0 + ( sourceClip.x1 - sourceClip.x0 );
	v.y1 = v.y0 + ( sourceClip.y1 - sourceClip.y0 );

	if ( !intersect( job.Dest, destClip, v ) )
		return 0;

	job.width = job.Dest.x1 - job.Dest.x0;
	job.height = job.Dest.y1 - job.Dest.y0;

	job.Source.x0 = sourceClip.x0 + ( job.Dest.x0 - v.x0 );
	job.Source.x1 = job.Source.x0 + job.width;
	job.Source.y0 = sourceClip.y0 + ( job.Dest.y0 - v.y0 );
	job.Source.y1 = job.Source.y0 + job.height;

	job.argb = argb;

	if ( source )
	{
		job.srcPitch = source->getPitch();
		job.srcPixelMul = source->getBitsPerPixel()/8;
		job.src = (void*) ( (uint8_t*) source->getData() + ( job.Source.y0 * job.srcPitch ) + ( job.Source.x0 * job.srcPixelMul ) );
	}
	else
	{
		// use srcPitch for color operation on dest
		job.srcPitch = job.width * dest->getBitsPerPixel()/8;
	}

	job.dstPitch = dest->getPitch();
	job.dstPixelMul = dest->getBitsPerPixel()/8;
	job.dst = (void*) ( (uint8_t*) dest->getData() + ( job.Dest.y0 * job.dstPitch ) + ( job.Dest.x0 * job.dstPixelMul ) );

	blitter( &job );

	return 1;
}

static int32_t StretchBlit(eBlitter operation,
		video::IImage* dest, const core::rect<int32_t> *destRect,
		const core::rect<int32_t> *srcRect, video::IImage* const source,
		uint32_t argb)
{
	tExecuteBlit blitter = getBlitter2( operation, dest, source );
	if ( 0 == blitter )
	{
		return 0;
	}

	SBlitJob job;

	// Clipping
	setClip ( job.Source, srcRect, source, 1 );
	setClip ( job.Dest, destRect, dest, 0 );

	job.width = job.Dest.x1-job.Dest.x0;
	job.height = job.Dest.y1-job.Dest.y0;

	job.argb = argb;

	// use original dest size, despite any clipping
	job.x_stretch = (float)destRect->getWidth() / (float)(job.Source.x1-job.Source.x0);
	job.y_stretch = (float)destRect->getHeight() / (float)(job.Source.y1-job.Source.y0);
	job.stretch = (job.x_stretch != 1.f) || (job.y_stretch != 1.f);

	if ( source )
	{
		job.srcPitch = source->getPitch();
		job.srcPixelMul = source->getBitsPerPixel()/8;
		job.src = (void*) ( (uint8_t*) source->getData() + ( job.Source.y0 * job.srcPitch ) + ( job.Source.x0 * job.srcPixelMul ) );
	}
	else
	{
		// use srcPitch for color operation on dest
		job.srcPitch = job.width * dest->getBitsPerPixel()/8;
	}

	job.dstPitch = dest->getPitch();
	job.dstPixelMul = dest->getBitsPerPixel()/8;
	job.dst = (void*) ( (uint8_t*) dest->getData() + ( job.Dest.y0 * job.dstPitch ) + ( job.Dest.x0 * job.dstPixelMul ) );

	blitter( &job );

	return 1;
}


// Methods for Software drivers
//! draws a rectangle
static void drawRectangle(video::IImage* img, const core::rect<int32_t>& rect, const video::SColor &color)
{
	Blit(color.getAlpha() == 0xFF ? BLITTER_COLOR : BLITTER_COLOR_ALPHA,
			img, 0, &rect.UpperLeftCorner, 0, &rect, color.color);
}


//! draws a line from to with color
static void drawLine(video::IImage* img, const core::position2d<int32_t>& from,
					 const core::position2d<int32_t>& to, const video::SColor &color)
{
	AbsRectangle clip;
	GetClip(clip, img);

	core::position2d<int32_t> p[2];
	if (ClipLine( clip, p[0], p[1], from, to))
	{
		uint32_t alpha = extractAlpha(color.color);

		switch(img->getColorFormat())
		{
		case asset::EF_A1R5G5B5_UNORM_PACK16:
				if (alpha == 256)
				{
					RenderLine16_Decal(img, p[0], p[1], video::A8R8G8B8toA1R5G5B5(color.color));
				}
				else
				{
					RenderLine16_Blend(img, p[0], p[1], video::A8R8G8B8toA1R5G5B5(color.color), alpha >> 3);
				}
				break;
		case asset::EF_B8G8R8A8_UNORM:
				if (alpha == 256)
				{
					RenderLine32_Decal(img, p[0], p[1], color.color);
				}
				else
				{
					RenderLine32_Blend(img, p[0], p[1], color.color, alpha);
				}
				break;
		default:
				break;
		}
	}
}


}

#endif

