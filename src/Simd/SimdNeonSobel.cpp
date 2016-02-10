/*
* Simd Library (http://simd.sourceforge.net).
*
* Copyright (c) 2011-2016 Yermalayeu Ihar.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy 
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
* copies of the Software, and to permit persons to whom the Software is 
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in 
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#include "Simd/SimdMemory.h"
#include "Simd/SimdStore.h"
#include "Simd/SimdMath.h"

namespace Simd
{
#ifdef SIMD_NEON_ENABLE    
    namespace Neon
    {
		template<bool align, bool abs> SIMD_INLINE void SobelDx(uint8x16_t a[3][3], int16_t * dst)
		{
			Store<align>(dst +  0, ConditionalAbs<abs>((int16x8_t)BinomialSum16(Sub<0>(a[0][2], a[0][0]), Sub<0>(a[1][2], a[1][0]), Sub<0>(a[2][2], a[2][0]))));
			Store<align>(dst + HA, ConditionalAbs<abs>((int16x8_t)BinomialSum16(Sub<1>(a[0][2], a[0][0]), Sub<1>(a[1][2], a[1][0]), Sub<1>(a[2][2], a[2][0]))));
		}

        template <bool align, bool abs> void SobelDx(const uint8_t * src, size_t srcStride, size_t width, size_t height, int16_t * dst, size_t dstStride)
        {
            assert(width > A);
            if(align)
                assert(Aligned(dst) && Aligned(dstStride, HA));

            size_t bodyWidth = Simd::AlignHi(width, A) - A;
            const uint8_t *src0, *src1, *src2;
            uint8x16_t a[3][3];

            for(size_t row = 0; row < height; ++row)
            {
                src0 = src + srcStride*(row - 1);
                src1 = src0 + srcStride;
                src2 = src1 + srcStride;
                if(row == 0)
                    src0 = src1;
                if(row == height - 1)
                    src2 = src1;

                LoadNoseDx(src0 + 0, a[0]);
                LoadNoseDx(src1 + 0, a[1]);
                LoadNoseDx(src2 + 0, a[2]);
                SobelDx<align, abs>(a, dst + 0);
                for(size_t col = A; col < bodyWidth; col += A)
                {
                    LoadBodyDx(src0 + col, a[0]);
                    LoadBodyDx(src1 + col, a[1]);
                    LoadBodyDx(src2 + col, a[2]);
                    SobelDx<align, abs>(a, dst + col);
                }
                LoadTailDx(src0 + width - A, a[0]);
                LoadTailDx(src1 + width - A, a[1]);
                LoadTailDx(src2 + width - A, a[2]);
                SobelDx<false, abs>(a, dst + width - A);

                dst += dstStride;
            }
        }

        void SobelDx(const uint8_t * src, size_t srcStride, size_t width, size_t height, uint8_t * dst, size_t dstStride)
        {
            assert(dstStride%sizeof(int16_t) == 0);

            if(Aligned(dst) && Aligned(dstStride))
                SobelDx<true, false>(src, srcStride, width, height, (int16_t *)dst, dstStride/sizeof(int16_t));
            else
                SobelDx<false, false>(src, srcStride, width, height, (int16_t *)dst, dstStride/sizeof(int16_t));
        }

		void SobelDxAbs(const uint8_t * src, size_t srcStride, size_t width, size_t height, uint8_t * dst, size_t dstStride)
		{
			assert(dstStride%sizeof(int16_t) == 0);

			if (Aligned(dst) && Aligned(dstStride))
				SobelDx<true, true>(src, srcStride, width, height, (int16_t *)dst, dstStride / sizeof(int16_t));
			else
				SobelDx<false, true>(src, srcStride, width, height, (int16_t *)dst, dstStride / sizeof(int16_t));
		}

		template<bool align> SIMD_INLINE void SobelDy(uint8x16_t a[3][3], int16_t * dst)
		{
			Store<align>(dst +  0, (int16x8_t)BinomialSum16(Sub<0>(a[2][0], a[0][0]), Sub<0>(a[2][1], a[0][1]), Sub<0>(a[2][2], a[0][2])));
			Store<align>(dst + HA, (int16x8_t)BinomialSum16(Sub<1>(a[2][0], a[0][0]), Sub<1>(a[2][1], a[0][1]), Sub<1>(a[2][2], a[0][2])));
		}

		template <bool align> void SobelDy(const uint8_t * src, size_t srcStride, size_t width, size_t height, int16_t * dst, size_t dstStride)
		{
			assert(width > A);
			if (align)
				assert(Aligned(dst) && Aligned(dstStride, HA));

			size_t bodyWidth = Simd::AlignHi(width, A) - A;
			const uint8_t *src0, *src1, *src2;
			uint8x16_t a[3][3];

			for (size_t row = 0; row < height; ++row)
			{
				src0 = src + srcStride*(row - 1);
				src1 = src0 + srcStride;
				src2 = src1 + srcStride;
				if (row == 0)
					src0 = src1;
				if (row == height - 1)
					src2 = src1;

				LoadNose3<align, 1>(src0 + 0, a[0]);
				LoadNose3<align, 1>(src2 + 0, a[2]);
				SobelDy<align>(a, dst + 0);
				for (size_t col = A; col < bodyWidth; col += A)
				{
					LoadBody3<align, 1>(src0 + col, a[0]);
					LoadBody3<align, 1>(src2 + col, a[2]);
					SobelDy<align>(a, dst + col);
				}
				LoadTail3<false, 1>(src0 + width - A, a[0]);
				LoadTail3<false, 1>(src2 + width - A, a[2]);
				SobelDy<false>(a, dst + width - A);

				dst += dstStride;
			}
		}

		void SobelDy(const uint8_t * src, size_t srcStride, size_t width, size_t height, uint8_t * dst, size_t dstStride)
		{
			assert(dstStride%sizeof(int16_t) == 0);

			if (Aligned(src) && Aligned(srcStride) && Aligned(dst) && Aligned(dstStride))
				SobelDy<true>(src, srcStride, width, height, (int16_t *)dst, dstStride / sizeof(int16_t));
			else
				SobelDy<false>(src, srcStride, width, height, (int16_t *)dst, dstStride / sizeof(int16_t));
		}
    }
#endif// SIMD_NEON_ENABLE
}