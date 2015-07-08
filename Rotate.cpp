//

#include <SDL/SDL.h>
//#include <emmintrin.h>
#define _USE_MATH_DEFINES
#include <math.h>


#ifdef	_MSC_VER
#define __ASM__	_asm
#else
//#include <arm_neon.h>
#define __ASM__	__asm__ __volatile__
#endif

#define FIXED_POINT_t	int32_t // 24bit + 8 bit


inline Uint8 *scanLine(SDL_Surface *surface, int y, int x)
{
	return (Uint8 *)(surface->pixels) + (y * surface->pitch) + (surface->format->BytesPerPixel * x);
}

// Bi Linear interpolation by Fixed Point
inline void BiLinear24_FP_NEON(SDL_Surface *src, FIXED_POINT_t X, FIXED_POINT_t Y, SDL_Surface *dst, int x, int y)
{
        Uint8   *pPixel = scanLine(dst, y, x);
        int     iX, iY;
        iX = X >> 8;
        iY = Y >> 8;;
        if (0 <= iX && iX < src->w - 1 && 0 <= iY && iY < src->h - 1)
        {
                Uint32  r, g, b, fX, fY;
                Uint8   *pPixel0, *pPixel1;

                pPixel0 = scanLine(src, iY, iX);
                pPixel1 = scanLine(src, iY + 1, iX);
                // fraction part
                fX = X & 0xFF;
                fY = Y & 0xFF;
		asm volatile (
		"vld3.8 {d0[], d2[], d4[]}, [%0] \n"
		"vld3.8 {d1[], d3[], d5[]}, [%1] \n"
		: "+r" (pPixel0),
		  "+r" (pPixel1)
		);
                b = (pPixel0[0] * (0x100 - fX) + pPixel0[3] * fX) * (0x100 - fY) + (pPixel1[0] * (0x100 - fX) + pPixel1[3] * fX) * fY;
                g = (pPixel0[1] * (0x100 - fX) + pPixel0[4] * fX) * (0x100 - fY) + (pPixel1[1] * (0x100 - fX) + pPixel1[4] * fX) * fY;
                r = (pPixel0[2] * (0x100 - fX) + pPixel0[5] * fX) * (0x100 - fY) + (pPixel1[2] * (0x100 - fX) + pPixel1[5] * fX) * fY;
                // 
                pPixel[0] = (Uint8)(b >> 16);
                pPixel[1] = (Uint8)(g >> 16);
                pPixel[2] = (Uint8)(r >> 16);
        } else {
                pPixel[0] = pPixel[1] = pPixel[2] = 0;
        }
}


// Bi Linear interpolation by Fixed Point
inline void BiLinear24_FP(SDL_Surface *src, FIXED_POINT_t X, FIXED_POINT_t Y, SDL_Surface *dst, int x, int y)
{
	Uint8	*pPixel = scanLine(dst, y, x);
	int	iX, iY;
	iX = X >> 8;
	iY = Y >> 8;;
	if (0 <= iX && iX < src->w - 1 && 0 <= iY && iY < src->h - 1)
	{
		Uint32	r, g, b, fX, fY;
		Uint8	*pPixel0, *pPixel1;

		pPixel0 = scanLine(src, iY, iX);
		pPixel1 = scanLine(src, iY + 1, iX);
		// fraction part
		fX = X & 0xFF;	
		fY = Y & 0xFF;
		b = (pPixel0[0] * (0x100 - fX) + pPixel0[3] * fX) * (0x100 - fY) + (pPixel1[0] * (0x100 - fX) + pPixel1[3] * fX) * fY;
		g = (pPixel0[1] * (0x100 - fX) + pPixel0[4] * fX) * (0x100 - fY) + (pPixel1[1] * (0x100 - fX) + pPixel1[4] * fX) * fY;
		r = (pPixel0[2] * (0x100 - fX) + pPixel0[5] * fX) * (0x100 - fY) + (pPixel1[2] * (0x100 - fX) + pPixel1[5] * fX) * fY;
		// 
		pPixel[0] = (Uint8)(b >> 16);
		pPixel[1] = (Uint8)(g >> 16);
		pPixel[2] = (Uint8)(r >> 16);
	} else {
		pPixel[0] = pPixel[1] = pPixel[2] = 0;
	}
}

bool _SDL_Rotate_FP(SDL_Surface *src, SDL_Surface *dst, int cx, int cy, double degree, SDL_Rect *bound)
{
	int bitsPerPixel = src->format->BitsPerPixel;
	double radian = M_PI / 180 * degree;
	float c = (float)cos(radian);
	float s = (float)sin(radian);

	FIXED_POINT_t C = (FIXED_POINT_t)(c * 256);
        FIXED_POINT_t S = (FIXED_POINT_t)(s * 256);

	FIXED_POINT_t X = (FIXED_POINT_t)(cx * 256 + S * (cy - bound->y) - C * (cx - bound->x));
	FIXED_POINT_t Y = (FIXED_POINT_t)(cy * 256 - C * (cy - bound->y) - S * (cx - bound->x));

	if (bitsPerPixel != dst->format->BitsPerPixel)
	{
		return false;
	}
	switch (bitsPerPixel)
	{
	case 24:
		for (int y = bound->y; y < bound->h + bound->y; y++)
		{
			FIXED_POINT_t Xx = X;
			FIXED_POINT_t Yx = Y;
			for (int x = bound->x; x < bound->w + bound->x; x++)
			{
				BiLinear24_FP_NEON(src, Xx, Yx, dst, x, y);
				Xx += C;
				Yx += S;
			}
			X -= S;
			Y += C;
		}
		return true;
	}

	return false;
}


#define BILINEAR_24	BiLinear24
inline void BiLinear24(SDL_Surface *src, float X, float Y, SDL_Surface *dst, int x, int y);
inline void BiLinear24_SIMD(SDL_Surface *src, float X, float Y, SDL_Surface *dst, int x, int y);
inline void Nearest24(SDL_Surface *src, float X, float Y, SDL_Surface *dst, int x, int y)
{
	int iX, iY;
	iX = (int)floor(X);
	iY = (int)floor(Y);
        Uint8 *pDst = scanLine(dst, y, x);

	if (0 <= iX && iX < src->w - 1 && 0 <= iY && iY < src->h - 1)
        {
		Uint8 *pSrc = scanLine(src, iY, iX);
		pDst[0] = pSrc[0];
		pDst[1] = pSrc[1];
		pDst[2] = pSrc[2];
	} else {
		pDst[0] = pDst[1] = pDst[2] = 0;
	}
}

// ????? NEON?
void BiLinear24_NEON(SDL_Surface *src, float X, float Y, SDL_Surface *dst, int x, int y)
{
	int	iX, iY;
	Uint8	*pPixel = scanLine(dst, y, x);
	iX = (int)floor(X);
	iY = (int)floor(Y);
	if (0 <= iX && iX < src->w - 1 && 0 <= iY && iY < src->h - 1)
	{
		float	r, g, b, fX, fY;
		Uint8	*pPixel0, *pPixel1;

		pPixel0 = scanLine(src, iY, iX);
		pPixel1 = scanLine(src, iY + 1, iX);
		fX = X - iX;
		fY = Y - iY;
		__asm__ __volatile__ (
		"vld3.8     {d0[], d2[], d4[]}, [%0]!      \n" // pixel(0, 0)
		//"vld3.8     {d0[4], d2[4], d4[4]}, [%0]!      \n" // pixel(0, 1)
		"vld3.8     {d1[], d3[], d5[]}, [%1]!      \n" // pixel(1, 0)
		//"vld3.8     {d1[4], d3[4], d5[4]}, [%1]!      \n" // pixel(1, 1)
		// TODO
		"subs       %2, %2, #24                  \n"
		"vmov       d2, d3                       \n" // order d0, d1, d2
		"vst3.8     {d0, d1, d2}, [%1]!          \n"
		//"bgt        1b                           \n"
		 : "+r"(pPixel0),          // %0
	 		"+r"(pPixel1),          // %1
			"+r"(pPixel)         // %2
		 :
		 : "d0", "d1", "d2", "d3", "memory", "cc"
		);
		/*
		b = (pPixel0[0] * (1 - fX) + pPixel0[3] * fX) * (1 - fY) + (pPixel1[0] * (1 - fX) + pPixel1[3] * fX) * fY;
		g = (pPixel0[1] * (1 - fX) + pPixel0[4] * fX) * (1 - fY) + (pPixel1[1] * (1 - fX) + pPixel1[4] * fX) * fY;
		r = (pPixel0[2] * (1 - fX) + pPixel0[5] * fX) * (1 - fY) + (pPixel1[2] * (1 - fX) + pPixel1[5] * fX) * fY;
		pPixel[0] = (Uint8)b;
		pPixel[1] = (Uint8)g;
		pPixel[2] = (Uint8)r;
		*/
	} else {
		pPixel[0] = pPixel[1] = pPixel[2] = 0;
	}
}
bool _SDL_Rotate(SDL_Surface *src, SDL_Surface *dst, int cx, int cy, double degree, SDL_Rect *bound)
{
	int bitsPerPixel = src->format->BitsPerPixel;
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
		for (int y = bound->y; y < bound->h + bound->y; y++)
		{
			float Xx = X;
			float Yx = Y;
			for (int x = bound->x; x < bound->w + bound->x; x++)
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
void BiLinear24(SDL_Surface *src, float X, float Y, SDL_Surface *dst, int x, int y)
{
	Uint8	*pPixel = scanLine(dst, y, x);
	int	iX, iY;
	iX = (int)floor(X);
	iY = (int)floor(Y);
	if (0 <= iX && iX < src->w - 1 && 0 <= iY && iY < src->h - 1)
	{
		float	r, g, b, fX, fY;
		Uint8	*pPixel0, *pPixel1;

		pPixel0 = scanLine(src, iY, iX);
		pPixel1 = scanLine(src, iY + 1, iX);
		fX = X - iX;
		fY = Y - iY;
		b = (pPixel0[0] * (1 - fX) + pPixel0[3] * fX) * (1 - fY) + (pPixel1[0] * (1 - fX) + pPixel1[3] * fX) * fY;
		g = (pPixel0[1] * (1 - fX) + pPixel0[4] * fX) * (1 - fY) + (pPixel1[1] * (1 - fX) + pPixel1[4] * fX) * fY;
		r = (pPixel0[2] * (1 - fX) + pPixel0[5] * fX) * (1 - fY) + (pPixel1[2] * (1 - fX) + pPixel1[5] * fX) * fY;
		pPixel[0] = (Uint8)b;
		pPixel[1] = (Uint8)g;
		pPixel[2] = (Uint8)r;
	} else {
		pPixel[0] = pPixel[1] = pPixel[2] = 0;
	}
}

// 双二次近似
#ifdef _MSC_VER
void BiLinear24_SIMD(SDL_Surface *src, float X, float Y, SDL_Surface *dst, int x, int y)
{
	Uint8	*pPixel = scanLine(dst, y, x);
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
		Uint8	*pPixel0, *pPixel1;
		Uint32	r, g, b;
		__m64	fX;
		__m128i	blue, green, red; // _mm_set_epiで代入すると、レジスタを割り当てるため直接代入する
		__m128	one;// = _mm_set_ps(1.0f, 1.0f, 1.0f, 1.0f);　ここも同様

		pPixel0 = scanLine(src, iY, iX);
		pPixel1 = scanLine(src, iY + 1, iX);

		// Blue
		blue.m128i_i32[2] = pPixel0[3];
		blue.m128i_i32[3] = pPixel0[0];
		blue.m128i_i32[0] = pPixel1[3];
		blue.m128i_i32[1] = pPixel1[0];
		// Green
		green.m128i_i32[2] = pPixel0[4];
		green.m128i_i32[3] = pPixel0[1];
		green.m128i_i32[0] = pPixel1[4];
		green.m128i_i32[1] = pPixel1[1];
		// Red
		red.m128i_i32[2] = pPixel0[5];
		red.m128i_i32[3] = pPixel0[2];
		red.m128i_i32[0] = pPixel1[5];
		red.m128i_i32[1] = pPixel1[2];
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
		pPixel[0] = b;
		pPixel[1] = g;
		pPixel[2] = r;
	} else {
		pPixel[0] = pPixel[1] = pPixel[2] = 0;
	}
}
#else
void BiLinear24_SIMD(SDL_Surface *src, float X, float Y, SDL_Surface *dst, int x, int y)
{
	Uint8	*pPixel = scanLine(dst, y, x);
	int	iX, iY;
	iX = (int)floor(X);
	iY = (int)floor(Y);
	if (0 <= iX && iX < src->w - 1 && 0 <= iY && iY < src->h - 1)
	{
		float	r, g, b, fX, fY;
		Uint8	*pPixel0, *pPixel1;
		//float32x4_t XY;
		//uint8x8x3_t raster0, raster1;
		pPixel0 = scanLine(src, iY, iX);
		pPixel1 = scanLine(src, iY + 1, iX);
		//raster0 = vld3_u8(pPixel0);
		//raster1 = vld3_u8(pPixel1);
		fX = X - iX;
		fY = Y - iY;
	} else {
		pPixel[0] = pPixel[1] = pPixel[2] = 0;
	}
}
#endif
