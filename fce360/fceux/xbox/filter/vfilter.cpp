#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <xtl.h>
#include "interp.h"

int finalw,finalh;

static void hq2x_32_def(uint32_t * dst0, uint32_t * dst1, const uint32_t * src0, const uint32_t * src1, const uint32_t * src2, unsigned count)
{
	static unsigned char cache_vert_mask[640];
	unsigned char cache_horiz_mask = 0;

	unsigned i;
	unsigned char mask;
	uint32_t  c[9];

	if (src0 == src1)	//processing first row
		memset(cache_vert_mask, 0, count);

	for(i=0;i<count;++i) {
		c[1] = src0[0];
		c[4] = src1[0];
		c[7] = src2[0];

		if (i>0) {
			c[0] = src0[-1];
			c[3] = src1[-1];
			c[6] = src2[-1];
		} else {
			c[0] = c[1];
			c[3] = c[4];
			c[6] = c[7];
		}

		if (i<count-1) {
			c[2] = src0[1];
			c[5] = src1[1];
			c[8] = src2[1];
		} else {
			c[2] = c[1];
			c[5] = c[4];
			c[8] = c[7];
		}

		mask = 0;

		mask |= interp_32_diff(c[0], c[4]) << 0;
		mask |= cache_vert_mask[i];
		mask |= interp_32_diff(c[2], c[4]) << 2;
		mask |= cache_horiz_mask;
		cache_horiz_mask = interp_32_diff(c[5], c[4]) << 3;
		mask |= cache_horiz_mask << 1;	// << 3 << 1 == << 4
		mask |= interp_32_diff(c[6], c[4]) << 5;
		cache_vert_mask[i] = interp_32_diff(c[7], c[4]) << 1;
		mask |= cache_vert_mask[i] << 5; // << 1 << 5 == << 6
		mask |= interp_32_diff(c[8], c[4]) << 7;

#define P0 dst0[0]
#define P1 dst0[1]
#define P2 dst1[0]
#define P3 dst1[1]
#define MUR interp_32_diff(c[1], c[5])
#define MDR interp_32_diff(c[5], c[7])
#define MDL interp_32_diff(c[7], c[3])
#define MUL interp_32_diff(c[3], c[1])
#define IC(p0) c[p0]
#define I11(p0,p1) interp_32_11(c[p0], c[p1])
#define I211(p0,p1,p2) interp_32_211(c[p0], c[p1], c[p2])
#define I31(p0,p1) interp_32_31(c[p0], c[p1])
#define I332(p0,p1,p2) interp_32_332(c[p0], c[p1], c[p2])
#define I431(p0,p1,p2) interp_32_431(c[p0], c[p1], c[p2])
#define I521(p0,p1,p2) interp_32_521(c[p0], c[p1], c[p2])
#define I53(p0,p1) interp_32_53(c[p0], c[p1])
#define I611(p0,p1,p2) interp_32_611(c[p0], c[p1], c[p2])
#define I71(p0,p1) interp_32_71(c[p0], c[p1])
#define I772(p0,p1,p2) interp_32_772(c[p0], c[p1], c[p2])
#define I97(p0,p1) interp_32_97(c[p0], c[p1])
#define I1411(p0,p1,p2) interp_32_1411(c[p0], c[p1], c[p2])
#define I151(p0,p1) interp_32_151(c[p0], c[p1])

		switch (mask) {
			#include "hq2x.h"
		}

#undef P0
#undef P1
#undef P2
#undef P3
#undef MUR
#undef MDR
#undef MDL
#undef MUL
#undef IC
#undef I11
#undef I211
#undef I31
#undef I332
#undef I431
#undef I521
#undef I53
#undef I611
#undef I71
#undef I772
#undef I97
#undef I1411
#undef I151

		src0 += 1;
		src1 += 1;
		src2 += 1;
		dst0 += 2;
		dst1 += 2;
	}
}

void hq2x_32( unsigned char * srcPtr,  DWORD srcPitch, unsigned char * dstPtr, int width, int height)
{
	const int dstPitch = srcPitch<<1;

	int count = height;

	uint32_t  *dst0 = (uint32_t  *)dstPtr;
	uint32_t  *dst1 = dst0 + (dstPitch >> 2);

	uint32_t  *src0 = (uint32_t  *)srcPtr;
	uint32_t  *src1 = src0 + (srcPitch >> 2);
	uint32_t  *src2 = src1 + (srcPitch >> 2);

	finalw=width*2;
	finalh=height*2;

	hq2x_32_def(dst0, dst1, src0, src0, src1, width);

	count -= 2;
	while(count) {
		dst0 += dstPitch >> 1;		//next 2 lines (dstPitch / 4 char per int * 2)
		dst1 += dstPitch >> 1;
		hq2x_32_def(dst0, dst1, src0, src1, src2, width);
		src0 = src1;
		src1 = src2;
		src2 += srcPitch >> 2;
		--count;
	}
	dst0 += dstPitch >> 1;
	dst1 += dstPitch >> 1;
	hq2x_32_def(dst0, dst1, src0, src1, src1, width);
}

static void hq3x_32_def(uint32_t*  dst0, uint32_t*  dst1, uint32_t*  dst2, const uint32_t* src0, const uint32_t* src1, const uint32_t* src2, unsigned count)
{
	static unsigned char cache_vert_mask[640];
	unsigned char cache_horiz_mask = 0;

	unsigned i;
	unsigned char mask;
	uint32_t  c[9];

	if (src0 == src1)	//processing first row
		memset(cache_vert_mask, 0, count);

	for(i=0;i<count;++i) {
		c[1] = src0[0];
		c[4] = src1[0];
		c[7] = src2[0];

		if (i>0) {
			c[0] = src0[-1];
			c[3] = src1[-1];
			c[6] = src2[-1];
		} else {
			c[0] = c[1];
			c[3] = c[4];
			c[6] = c[7];
		}

		if (i<count-1) {
			c[2] = src0[1];
			c[5] = src1[1];
			c[8] = src2[1];
		} else {
			c[2] = c[1];
			c[5] = c[4];
			c[8] = c[7];
		}

		mask = 0;

		mask |= interp_32_diff(c[0], c[4]) << 0;
		mask |= cache_vert_mask[i];
		mask |= interp_32_diff(c[2], c[4]) << 2;
		mask |= cache_horiz_mask;
		cache_horiz_mask = interp_32_diff(c[5], c[4]) << 3;
		mask |= cache_horiz_mask << 1;	// << 3 << 1 == << 4
		mask |= interp_32_diff(c[6], c[4]) << 5;
		cache_vert_mask[i] = interp_32_diff(c[7], c[4]) << 1;
		mask |= cache_vert_mask[i] << 5; // << 1 << 5 == << 6
		mask |= interp_32_diff(c[8], c[4]) << 7;

#define P0 dst0[0]
#define P1 dst0[1]
#define P2 dst0[2]
#define P3 dst1[0]
#define P4 dst1[1]
#define P5 dst1[2]
#define P6 dst2[0]
#define P7 dst2[1]
#define P8 dst2[2]
#define MUR interp_32_diff(c[1], c[5])
#define MDR interp_32_diff(c[5], c[7])
#define MDL interp_32_diff(c[7], c[3])
#define MUL interp_32_diff(c[3], c[1])
#define IC(p0) c[p0]
#define I11(p0,p1) interp_32_11(c[p0], c[p1])
#define I211(p0,p1,p2) interp_32_211(c[p0], c[p1], c[p2])
#define I31(p0,p1) interp_32_31(c[p0], c[p1])
#define I332(p0,p1,p2) interp_32_332(c[p0], c[p1], c[p2])
#define I431(p0,p1,p2) interp_32_431(c[p0], c[p1], c[p2])
#define I521(p0,p1,p2) interp_32_521(c[p0], c[p1], c[p2])
#define I53(p0,p1) interp_32_53(c[p0], c[p1])
#define I611(p0,p1,p2) interp_32_611(c[p0], c[p1], c[p2])
#define I71(p0,p1) interp_32_71(c[p0], c[p1])
#define I772(p0,p1,p2) interp_32_772(c[p0], c[p1], c[p2])
#define I97(p0,p1) interp_32_97(c[p0], c[p1])
#define I1411(p0,p1,p2) interp_32_1411(c[p0], c[p1], c[p2])
#define I151(p0,p1) interp_32_151(c[p0], c[p1])

		switch (mask) {
			#include "hq3x.h"
		}

#undef P0
#undef P1
#undef P2
#undef P3
#undef P4
#undef P5
#undef P6
#undef P7
#undef P8
#undef MUR
#undef MDR
#undef MDL
#undef MUL
#undef IC
#undef I11
#undef I211
#undef I31
#undef I332
#undef I431
#undef I521
#undef I53
#undef I611
#undef I71
#undef I772
#undef I97
#undef I1411
#undef I151

		src0 += 1;
		src1 += 1;
		src2 += 1;
		dst0 += 3;
		dst1 += 3;
		dst2 += 3;
	}
}

void hq3x_32( unsigned char * srcPtr,  DWORD srcPitch, unsigned char * dstPtr, int width, int height)
{
	int count = height;

	int dstPitch = srcPitch*3;
	int dstRowPixels = dstPitch>>2;

	uint32_t  *dst0 = (uint32_t  *)dstPtr;
	uint32_t  *dst1 = dst0 + dstRowPixels;
	uint32_t  *dst2 = dst1 + dstRowPixels;

	uint32_t  *src0 = (uint32_t  *)srcPtr;
	uint32_t  *src1 = src0 + (srcPitch >> 2);
	uint32_t  *src2 = src1 + (srcPitch >> 2);

	finalw=width*3;
	finalh=height*3;

	hq3x_32_def(dst0, dst1, dst2, src0, src0, src2, width);

	count -= 2;
	while(count) {
		dst0 += dstRowPixels * 3;
		dst1 += dstRowPixels * 3;
		dst2 += dstRowPixels * 3;

		hq3x_32_def(dst0, dst1, dst2, src0, src1, src2, width);
		src0 = src1;
		src1 = src2;
		src2 += srcPitch >> 2;
		--count;
	}
	dst0 += dstRowPixels * 3;
	dst1 += dstRowPixels * 3;
	dst2 += dstRowPixels * 3;

	hq3x_32_def(dst0, dst1, dst2, src0, src1, src1, width);

}
