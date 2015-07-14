//
//
#include <SDL/SDL.h>
#include <arm_neon.h>

void test(Uint8 *pPixel0, Uint8 *pPixel1, Uint32 fX, Uint32 fY, Uint8 *pPixel)
{
        const Uint64 index = 0x1010100110101000;
	uint32x4_t coeffX = {0x100 - fX, 0x100 - fX, fX, fX};
	uint32x4_t coeffY = {0x100 - fY, fY, 0x100 - fY, fY};
                asm volatile (
                "vld3.8 {d0, d2, d4}, [%1] \n\t"
                "vld3.8 {d1, d3, d5}, [%2] \n\t"
                "vld1.64 d10, [%3] \n\t"
                "vtbl.8 d0, {d0}, d10 \n\t"
                "vtbl.8 d1, {d1}, d10 \n\t"
                "vtbl.8 d2, {d2}, d10 \n\t"
                "vtbl.8 d3, {d3}, d10 \n\t"
                "vtbl.8 d4, {d4}, d10 \n\t"
                "vtbl.8 d5, {d5}, d10 \n\t"
		"vld2.32 {d6, d7}, [%4] \n\t"
		"vld2.32 {d8, d9}, [%5] \n\t"
                "vmul.i32 q0, q0, q3 \n\t"
                "vmul.i32 q1, q1, q3 \n\t"
                "vmul.i32 q2, q2, q3 \n\t"
                "vmul.i32 q0, q0, q4 \n\t"
                "vmul.i32 q1, q1, q4 \n\t"
                "vmul.i32 q2, q2, q4 \n\t"
                "vpadd.s32 d0, d0, d1 \n\t"
                "vpaddl.s32 d0, d0 \n\t"
                "vpadd.s32 d1, d2, d3 \n\t"
                "vpaddl.s32 d1, d1 \n\t"
                "vpadd.s32 d2, d4, d5 \n\t"
                "vpaddl.s32 d2, d2 \n\t"
                "vst3.8 {d0[2], d1[2], d2[2]}, [%0] \n\t"
                : "=r" (pPixel)
                : "r" (pPixel0),
                  "r" (pPixel1),
                  "r" (&index),
		  "r" (&coeffX),
		  "r" (&coeffY)
                );
}

int main(int argc, char *argv[])
{
	Uint8 raster0[24] = {255, 255, 255, 3, 4, 5};
	Uint8 raster1[24] = {0, 0, 128, 9, 10, 11};
	Uint8 result[24] = {0};
	Uint32 fX = 0x80;
	Uint32 fY = 0x00;
	test(raster0, raster1, fX, fY, result);
	printf("%d %d %d\n", result[0], result[1], result[2]);
	return 0;
}

