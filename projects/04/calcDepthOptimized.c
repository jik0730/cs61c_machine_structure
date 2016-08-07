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
#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

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
    // for (int y = 0; y < imageHeight; y++)
    // {
    //     if ((y < featureHeight) || (y >= imageHeight - featureHeight)) {
    //         for (int x = 0; x < imageWidth / 4 * 4; x += 4) {
    //             _mm_storeu_ps((depth + y * imageWidth + x), _mm_setzero_ps());
    //         }
    //         for (int i = imageWidth / 4 * 4; i < imageWidth; i++) {
    //             depth[y * imageWidth + i] = 0;
    //         }
    //     } else {
    //         for (int x = 0; x < featureWidth; x++) {
    //             depth[y * imageWidth + x] = 0;
    //         }
    //         for (int x = imageWidth - featureWidth; x < imageWidth; x++) {
    //             depth[y * imageWidth + x] = 0;
    //         }
    //     }
    // }
    int y;
    for (y = 0; y < imageHeight; y += 4) {
        if (y + 4 > imageHeight) break;
        int x;
        for (x = 0; x < imageWidth; x += 4) {
            if (x + 4 > imageWidth) break;
            _mm_storeu_ps((depth + y * imageWidth + x), _mm_setzero_ps());
            _mm_storeu_ps((depth + (y+1) * imageWidth + x), _mm_setzero_ps());
            _mm_storeu_ps((depth + (y+2) * imageWidth + x), _mm_setzero_ps());
            _mm_storeu_ps((depth + (y+3) * imageWidth + x), _mm_setzero_ps());
        }
        for (; x < imageWidth; x++) {
            depth[y * imageWidth + x] = 0;
            depth[(y+1) * imageWidth + x] = 0;
            depth[(y+2) * imageWidth + x] = 0;
            depth[(y+3) * imageWidth + x] = 0;
        }
    }

    for (; y < imageHeight; y++) {
        int x;
        for (x = 0; x < imageWidth; x += 4) {
            if (x + 4 > imageWidth) break;
            _mm_storeu_ps((depth + y * imageWidth + x), _mm_setzero_ps());
        }
        for (; x < imageWidth; x++) {
            depth[y * imageWidth + x] = 0;
        }
    }

    // int endX = imageWidth - featureWidth;
    // int endY = imageHeight - featureHeight;
    //#pragma omp parallel for collapse(2) schedule(dynamic)
    for (int y = featureHeight; y < imageHeight - featureHeight; y++)
    {
        for (int x = featureWidth; x < imageWidth - featureWidth; x++)
        {   
            float minimumSquaredDifference = -1;
            int minimumDy = 0;
            int minimumDx = 0;

            /* Iterate through all feature boxes that fit inside the maximum displacement box. 
               centered around the current pixel. */
            for (int dy = MAX(-maximumDisplacement, featureHeight - y); dy <= MIN(maximumDisplacement, imageHeight - featureHeight - y - 1); dy++)
            {
                for (int dx = MAX(-maximumDisplacement, featureWidth - x); dx <= MIN(maximumDisplacement, imageWidth - featureWidth - x - 1); dx++)
                {
                    /* Skip feature boxes that dont fit in the displacement box. */
                    //y + dy - featureHeight < 0 || y + dy + featureHeight >= imageHeight || 
                    // if (x + dx - featureWidth < 0 || x + dx + featureWidth >= imageWidth)
                    // {
                    //     continue;
                    // }

                    float squaredDifference = 0;
                    __m128 squaredDifference_vector = _mm_setzero_ps();

                    /* Sum the squared difference within a box of +/- featureHeight and +/- featureWidth. */
                    for (int boxY = -featureHeight; boxY <= featureHeight; boxY++)
                    {
                        int leftY = y + boxY;
                        int rightY = y + dy + boxY;
                        for (int boxX = 0; boxX < (featureWidth + featureWidth) / 4 * 4; boxX += 4)
                        {
                            __m128 left_vector = _mm_loadu_ps(left + leftY * imageWidth + x + boxX - featureWidth);
                            __m128 right_vector = _mm_loadu_ps(right + rightY * imageWidth + x + dx + boxX - featureWidth);
                            __m128 difference_vector = _mm_sub_ps(left_vector, right_vector);
                            squaredDifference_vector = _mm_add_ps(squaredDifference_vector, 
                                _mm_mul_ps(difference_vector, difference_vector));
                        }

                        // tail case
                        for (int i = (featureWidth + featureWidth) / 4 * 4; i <= featureWidth + featureWidth; i++)
                        {
                            float difference = left[(y + boxY) * imageWidth + (x + i - featureWidth)] - right[(y + dy + boxY) * imageWidth + (x + dx + i - featureWidth)];
                            squaredDifference += difference * difference;
                        }
                    }

                    float vec_sum[4];
                    _mm_storeu_ps(vec_sum, squaredDifference_vector);
                    squaredDifference += vec_sum[0] + vec_sum[1] + vec_sum[2] + vec_sum[3];

                    /* 
                    Check if you need to update minimum square difference. 
                    This is when either it has not been set yet, the current
                    squared displacement is equal to the min and but the new
                    displacement is less, or the current squared difference
                    is less than the min square difference.
                    */
                    if ((minimumSquaredDifference == -1) || ((minimumSquaredDifference == squaredDifference) && (dx * dx + dy * dy < minimumDx * minimumDx + minimumDy * minimumDy)) || (minimumSquaredDifference > squaredDifference))
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
            // if (minimumSquaredDifference != -1)
            // {
            //     if (maximumDisplacement == 0)
            //     {
            //         depth[y * imageWidth + x] = 0;
            //     }
            //     else
            //     {
            //         depth[y * imageWidth + x] = sqrt(minimumDx * minimumDx + minimumDy * minimumDy);
            //     }
            // }
            // else
            // {
            //     depth[y * imageWidth + x] = 0;
            // }
            if (minimumSquaredDifference != -1 && maximumDisplacement != 0)
            {
                depth[y * imageWidth + x] = sqrt(minimumDx * minimumDx + minimumDy * minimumDy);
            }
        }
    }
    // #pragma omp parallel for
    // for (int y = featureHeight; y < imageHeight - featureHeight; y++)
    // {
    //     for (int x = featureWidth; x < imageWidth - featureWidth; x++)
    //     {   
    //         /* Set the depth to 0 if looking at edge of the image where a feature box cannot fit. */

    //         float minimumSquaredDifference = -1;
    //         int minimumDy = 0;
    //         int minimumDx = 0;

    //         /* Iterate through all feature boxes that fit inside the maximum displacement box. 
    //            centered around the current pixel. */
    //         for (int dy = MAX(-maximumDisplacement, featureHeight - y); dy <= MIN(maximumDisplacement, imageHeight - featureHeight - y - 1); dy++)
    //         {
    //             for (int dx = MAX(-maximumDisplacement, featureWidth - x); dx <= MIN(maximumDisplacement, imageHeight - featureHeight - y - 1); dx++)
    //             {
    //                 float squaredDifference = 0;

    //                 /* Sum the squared difference within a box of +/- featureHeight and +/- featureWidth. */
    //                 for (int boxY = -featureHeight; boxY <= featureHeight; boxY++)
    //                 {
    //                     for (int boxX = -featureWidth; boxX <= featureWidth; boxX++)
    //                     {
    //                         int leftX = x + boxX;
    //                         int leftY = y + boxY;
    //                         int rightX = x + dx + boxX;
    //                         int rightY = y + dy + boxY;

    //                         float difference = left[leftY * imageWidth + leftX] - right[rightY * imageWidth + rightX];
    //                         squaredDifference += difference * difference;
    //                     }
    //                 }

    //                 /* 
    //                 Check if you need to update minimum square difference. 
    //                 This is when either it has not been set yet, the current
    //                 squared displacement is equal to the min and but the new
    //                 displacement is less, or the current squared difference
    //                 is less than the min square difference.
    //                 */
    //                 if ((minimumSquaredDifference == -1) || ((minimumSquaredDifference == squaredDifference) && (displacementNaive(dx, dy) < displacementNaive(minimumDx, minimumDy))) || (minimumSquaredDifference > squaredDifference))
    //                 {
    //                     minimumSquaredDifference = squaredDifference;
    //                     minimumDx = dx;
    //                     minimumDy = dy;
    //                 }
    //             }
    //         }

    //         /* 
    //         Set the value in the depth map. 
    //         If max displacement is equal to 0, the depth value is just 0.
    //         */
    //         if (minimumSquaredDifference != -1)
    //         {
    //             if (maximumDisplacement == 0)
    //             {
    //                 depth[y * imageWidth + x] = 0;
    //             }
    //             else
    //             {
    //                 depth[y * imageWidth + x] = displacementNaive(minimumDx, minimumDy);
    //             }
    //         }
    //         else
    //         {
    //             depth[y * imageWidth + x] = 0;
    //         }
    //     }
    // }
}
