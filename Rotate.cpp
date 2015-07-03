﻿// Rotate.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include <SDL.h>
#include <emmintrin.h>
#define _USE_MATH_DEFINES
#include <math.h>


#ifdef	_MSC_VER
#define __ASM__	_asm
#else
#define __ASM__	__asm__ __volatile__
#endif

#define BILINEAR_24	BiLinear24
void BiLinear24(SDL_Surface *src, float X, float Y, SDL_Surface *dst, int x, int y);
void biLinear24(SDL_Surface *src, float X, float Y, SDL_Surface *dst, int x, int y);

inline Uint8 *scanLine(SDL_Surface *surface, int y)
{
	return (Uint8 *)(surface->pixels) + (y * surface->pitch);
}


bool _SDL_Rotate(SDL_Surface *src, SDL_Surface *dst, int cx, int cy, double degree, SDL_Rect *bound)
{
	int x, y, bitsPerPixel = src->format->BitsPerPixel;
	double radian = M_PI / 180 * degree;
	float c = (float)cos(radian);
	float s = (float)sin(radian);

	float X = cx + s * (cy - bound->y) - c * (cx - bound->x);
	float Y = cy - c * (cy - bound->y) - s * (cx - bound->x);

	if (bitsPerPixel != dst->format->BitsPerPixel)
	{
		return false;
	}
	switch (bitsPerPixel)
	{
	case 32:
		/*
		for (int y = bound->y; y < bound->h + bound->y; y++)
		{
			float Xx = X;
			float Yx = Y;
			for (int x = bound->x; x < bound->w + bound->x; x++)
			{
				biLinear32(src, Xx, Yx, dst, x, y);
				Xx += c;
				Yx += s;
			}
			X -= s;
			Y += c;
		}
		*/
		break;

	case 24:
		for (y = bound->y; y < bound->h + bound->y; y++)
		{
			float Xx = X;
			float Yx = Y;
			for (x = bound->x; x < bound->w + bound->x; x++)
			{
				BILINEAR_24(src, Xx, Yx, dst, x, y);
				Xx += c;
				Yx += s;
			}
			X -= s;
			Y += c;
		}
		return true;
	}

	return false;
}

// 双二次近似
static void BiLinear24(SDL_Surface *src, float X, float Y, SDL_Surface *dst, int x, int y)
{
	int	iX, iY;
	Uint8	*pPixel = scanLine(dst, y);
	iX = (int)floor(X);
	iY = (int)floor(Y);
	if (0 <= iX && iX < src->w - 1 && 0 <= iY && iY < src->h - 1)
	{
		float	r, g, b, fX, fY;
		Uint8	*pPixel0, *pPixel1;

		pPixel0 = scanLine(src, iY);
		pPixel1 = scanLine(src, iY + 1);
		fX = X - iX;
		fY = Y - iY;
		b = (pPixel0[iX * 3] * (1 - fX) + pPixel0[(iX + 1) * 3] * fX) * (1 - fY) + (pPixel1[iX * 3] * (1 - fX) + pPixel1[(iX + 1) * 3] * fX) * fY;
		pPixel0 += 3;
		pPixel1 += 3;
		g = (pPixel0[iX * 3] * (1 - fX) + pPixel0[(iX + 1) * 3] * fX) * (1 - fY) + (pPixel1[iX * 3] * (1 - fX) + pPixel1[(iX + 1) * 3] * fX) * fY;
		pPixel0 += 3;
		pPixel1 += 3;
		r = (pPixel0[iX * 3] * (1 - fX) + pPixel0[(iX + 1) * 3] * fX) * (1 - fY) + (pPixel1[iX * 3] * (1 - fX) + pPixel1[(iX + 1) * 3] * fX) * fY;
		pPixel[x * 3] = (Uint8)b;
		pPixel[x * 3 + 1] = (Uint8)g;
		pPixel[x * 3 + 2] = (Uint8)r;
	} else {
		pPixel[x * 3] = pPixel[x * 3 + 1] = pPixel[x * 3 + 2] = 0;
	}
}

// 双二次近似
static void biLinear24(SDL_Surface *src, float X, float Y, SDL_Surface *dst, int x, int y)
{
	__m128i iXY;
	__m128 XY; //= _mm_set_ps(X, X, Y, Y);
	XY.m128_f32[0] = XY.m128_f32[1] = Y;
	XY.m128_f32[2] = XY.m128_f32[3] = X;

#define iX iXY.m128i_i32[2]
#define iY iXY.m128i_i32[0]
	__ASM__
	{
		movaps		xmm0, XY
		cvttps2dq	xmm1, xmm0
		movaps		iXY, xmm1
	}

	if (0 <= iX && iX < src->w - 1 && 0 <= iY && iY < src->h - 1)
	{
		Uint8	*pPixel0, *pPixel1, *pPixel;
		Uint32	r, g, b;
		__m64	fX;
		__m128i	blue, green, red; // _mm_set_epiで代入すると、レジスタに割り当てられる
		__m128	one;// = _mm_set_ps(1.0f, 1.0f, 1.0f, 1.0f);

		pPixel = scanLine(dst, y);
		pPixel0 = scanLine(src, iY);
		pPixel1 = scanLine(src, iY + 1);

		// Blue
		blue.m128i_i32[2] = pPixel0[(iX + 1) * 3];
		blue.m128i_i32[3] = pPixel0[(iX) * 3];
		blue.m128i_i32[0] = pPixel1[(iX + 1) * 3];
		blue.m128i_i32[1] = pPixel1[(iX) * 3];
		// Green
		pPixel0 += 3;
		pPixel1 += 3;
		green.m128i_i32[2] = pPixel0[(iX + 1) * 3];
		green.m128i_i32[3] = pPixel0[(iX) * 3];
		green.m128i_i32[0] = pPixel1[(iX + 1) * 3];
		green.m128i_i32[1] = pPixel1[(iX) * 3];
		// Red
		pPixel0 += 3;
		pPixel1 += 3;
		red.m128i_i32[2] = pPixel0[(iX + 1) * 3];
		red.m128i_i32[3] = pPixel0[(iX) * 3];
		red.m128i_i32[0] = pPixel1[(iX + 1) * 3];
		red.m128i_i32[1] = pPixel1[(iX) * 3];
		// One
		one.m128_f32[0] = one.m128_f32[1] = one.m128_f32[2] = one.m128_f32[3] = 1.0f;
		// xmm0 = fX, fY
		__ASM__
		{
			cvtdq2ps	xmm1, xmm1
			subps		xmm0, xmm1
			movhps	fX, xmm0
		}
		// xmm1 = pixels
		// 単精度浮動小数に変換
		__ASM__
		{
			cvtdq2ps	xmm1, blue
			cvtdq2ps	xmm4, green
			cvtdq2ps	xmm5, red
		}
		// xmm2 = 1 - fX, 1 - fY
		__ASM__
		{
			movaps		xmm2, one
			subps		xmm2, xmm0
			movaps		one, xmm2
		}
		// xmm3 = 1 - fY, fY
		__ASM__
		{
			movaps		xmm3, xmm0
			shufps		xmm3, xmm2, 0x00
			movaps		one, xmm3
		}
		// xmm1 *= 1 - fY, fY
		__ASM__
		{
			mulps		xmm1, xmm3
			mulps		xmm4, xmm3
			mulps		xmm5, xmm3
		}
		// xmm2 = 1 - fX, fX
		__ASM__
		{
			movlps		xmm2, fX
			movaps		one, xmm2
			shufps		xmm2, xmm2, 0xD8
			movaps		XY, xmm2
		}
		// xmm1 *= 1 -fX, fX
		__ASM__
		{
			mulps		xmm1, xmm2
			mulps		xmm4, xmm2
			mulps		xmm5, xmm2
			movaps		XY, xmm1
		}
		//	合計
		__ASM__
		{
			; blue
			movhlps		xmm0, xmm1
			addps		xmm0, xmm1
			movaps		xmm1, xmm0
			shufps		xmm1, xmm1, 1
			addss		xmm0, xmm1
			cvtss2si	eax, xmm0
			mov			b, eax
			; green
			movhlps		xmm0, xmm4
			addps		xmm0, xmm4
			movaps		xmm4, xmm0
			shufps		xmm4, xmm4, 1
			addss		xmm0, xmm4
			cvtss2si	eax, xmm0
			mov			g, eax
			; red
			movhlps		xmm0, xmm5
			addps		xmm0, xmm5
			movaps		xmm5, xmm0
			shufps		xmm5, xmm5, 1
			addss		xmm0, xmm5
			cvtss2si	eax, xmm0
			mov			r, eax
		}
		pPixel[x * 3] = b;
		pPixel[x * 3 + 1] = g;
		pPixel[x * 3 + 2] = r;
	}
}