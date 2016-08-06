// CS 61C Fall 2015 Project 4

// include SSE intrinsics
#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <x86intrin.h>
#endif

// include OpenMP
#if !defined(_MSC_VER)
#include <pthread.h>
#endif
#include <omp.h>

#include "calcDepthOptimized.h"
#include "calcDepthNaive.h"

/* DO NOT CHANGE ANYTHING ABOVE THIS LINE. */
// Implements the displacement function
float displacementNaive2(int dx, int dy)
{
    //float squaredDisplacement = dx * dx + dy * dy;
    return sqrt(dx * dx + dy * dy);
}

float compareDisplacement(int dx, int dy) {
    return dx * dx + dy * dy;
}

void calcDepthOptimized(float *depth, float *left, float *right, int imageWidth, int imageHeight, int featureWidth, int featureHeight, int maximumDisplacement)
{
    /* The two outer for loops iterate through each pixel */
    // #pragma omp parallel for collapse(2)
    // for (int y = 0; y < featureHeight; y++) {
    //     for (int x = 0; x < imageWidth; x++) {
    //         depth[y * imageWidth + x] = 0;
    //     }
    // }

    // #pragma omp parallel for collapse(2)
    // for (int y = imageHeight - featureHeight; y < imageHeight; y++) {
    //     for (int x = 0; x < imageWidth; x++) {
    //         depth[y * imageWidth + x] = 0;
    //     }
    // }
    // for (int y = 0; y < imageHeight; y += 4) {
    //     for (int x = 0; x < imageWidth; x += 4) {

    //         // Depth 0 세팅, feature height나 width 작아서 고려 필요 없을듯
    //         if ((y + 4 < featureHeight) || (y >= imageHeight - featureHeight) || (x < featureWidth) || (x >= imageWidth - featureWidth))
    //         {
    //             depth[y * imageWidth + x] = 0;
    //             continue;
    //         }

    //         __m128i depth_vector = _mm_loadu_si128((__m128i *) (depth + y * imageWidth + x));



    //     }
    // }




    #pragma omp parallel for collapse(2) schedule(dynamic)
    for (int y = 0; y < imageHeight; y++)
    {
        for (int x = 0; x < imageWidth; x++)
        {   
            /* Set the depth to 0 if looking at edge of the image where a feature box cannot fit. */
            if ((y < featureHeight) || (y >= imageHeight - featureHeight) ||(x < featureWidth) || (x >= imageWidth - featureWidth))
            {
                depth[y * imageWidth + x] = 0;
                continue;
            }

            float minimumSquaredDifference = -1;
            int minimumDy = 0;
            int minimumDx = 0;

            /* Iterate through all feature boxes that fit inside the maximum displacement box. 
               centered around the current pixel. */
            for (int dy = -maximumDisplacement; dy <= maximumDisplacement; dy++)
            {
                for (int dx = -maximumDisplacement; dx <= maximumDisplacement; dx++)
                {
                    /* Skip feature boxes that dont fit in the displacement box. */
                    if (y + dy - featureHeight < 0 || y + dy + featureHeight >= imageHeight || x + dx - featureWidth < 0 || x + dx + featureWidth >= imageWidth)
                    {
                        continue;
                    }

                    float squaredDifference = 0;
                    __m128 squaredDifference_vector = _mm_setzero_ps();
                    float remaining = 0;

                    /* Sum the squared difference within a box of +/- featureHeight and +/- featureWidth. */
                    // #pragma omp reduction(+:squaredDifference)
                    for (int boxY = -featureHeight; boxY <= featureHeight; boxY++)
                    {
                        int leftY = y + boxY;
                        int rightY = y + dy + boxY;
                        for (int boxX = 0; boxX < (featureWidth + featureWidth) + 1; boxX += 4)
                        {
                            //int leftX = x + boxX;
                            //int rightX = x + dx + boxX;
                            __m128 left_vector = _mm_loadu_ps(left + leftY * imageWidth + x + boxX - featureWidth);
                            __m128 right_vector = _mm_loadu_ps(right + rightY * imageWidth + x + dx + boxX - featureWidth);
                            __m128 difference_vector = _mm_sub_ps(left_vector, right_vector);
                            //float difference = left[leftY * imageWidth + leftX] - right[rightY * imageWidth + rightX];
                            squaredDifference_vector = _mm_add_ps(squaredDifference_vector, 
                                _mm_mul_ps(difference_vector, difference_vector));
                            //squaredDifference += difference * difference;
                            if (boxX + 4 > 2 * featureWidth + 1) {
                                float re[4];
                                _mm_storeu_ps(re, difference_vector);
                                for (int k = 2 * featureWidth + 1; k < boxX + 4; k++) {
                                    remaining += re[4 - (boxX + 4 - k)];
                                }
                            }
                        }

                        

                        // tail case
                        // for (int i = (featureWidth + featureWidth) / 4 * 4; i <= featureWidth + featureWidth; i++)
                        // {
                        //     //int leftX = x + i - featureWidth;
                        //     //int rightX = x + dx + i - featureWidth;
                        //     float difference = left[(y + boxY) * imageWidth + (x + i - featureWidth)] - right[(y + dy + boxY) * imageWidth + (x + dx + i - featureWidth)];
                        //     squaredDifference += difference * difference;
                        // }


                    }

                    float vec_sum[4];
                    _mm_storeu_ps(vec_sum, squaredDifference_vector);
                    squaredDifference += vec_sum[0] + vec_sum[1] + vec_sum[2] + vec_sum[3];
                    squaredDifference -= remaining;
                    //printf("sD: %f", squaredDifference);

                    /* 
                    Check if you need to update minimum square difference. 
                    This is when either it has not been set yet, the current
                    squared displacement is equal to the min and but the new
                    displacement is less, or the current squared difference
                    is less than the min square difference.
                    */
                    if ((minimumSquaredDifference == -1) || ((minimumSquaredDifference == squaredDifference) && (compareDisplacement(dx, dy) < compareDisplacement(minimumDx, minimumDy))) || (minimumSquaredDifference > squaredDifference))
                    {
                        minimumSquaredDifference = squaredDifference;
                        minimumDx = dx;
                        minimumDy = dy;
                    }
                }
            }

            /* 
            Set the value in the depth map. 
            If max displacement is equal to 0, the depth value is just 0.
            */
            if (minimumSquaredDifference != -1)
            {
                if (maximumDisplacement == 0)
                {
                    depth[y * imageWidth + x] = 0;
                }
                else
                {
                    depth[y * imageWidth + x] = displacementNaive2(minimumDx, minimumDy);
                }
            }
            else
            {
                depth[y * imageWidth + x] = 0;
            }
        }
    }
}
