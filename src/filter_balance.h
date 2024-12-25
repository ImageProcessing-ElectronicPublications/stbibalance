/**

Balance filter an image v1.1
By zvezdochiot <mykaralw@yandex.ru>
This is free and unencumbered software released into the public domain.

QUICK START

    #include ...
    #include ...
    #define FILTER_BALANCE_IMPLEMENTATION
    #include "filter_balance.h"
    ...
    unsigned int width = 0, height = 0;
    unsigned int widthb1 = 0, heightb1 = 0;
    unsigned int widthb2 = 0, heightb2 = 0;
    unsigned char components = 1;
    unsigned char componentsb1 = 1, componentsb2 = 1;
    float mix = 1.0f;

    unsigned char* image = stbi_load("foo.png", &width, &height, &components, 0);
    unsigned char* blur1r = stbi_load("foo_blur1r.png", &widthb1, &heightb1, &componentsb1, 0);
    unsigned char* blur2r = stbi_load("foo_blur2r.png", &widthb2, &heightb2, &componentsb2, 0);

    if ((width == widthb1) && (height == heightb1) && (components == componentsb1) && (width == widthb2) && (height == heightb2) && (components == componentsb2))
    {
        image_filter_balance(width, height, components, mix, image, blur1r, blur2r);
        stbi_write_png("foo.thresgrad.png", width, height, components, image, 0);
    }

VERSION HISTORY

1.1  2024-12-25  "head"    Header release.
1.0  2024-12-16  "init"    Initial release.

**/

#ifndef FILTER_BALANCE_H
#define FILTER_BALANCE_H
#ifdef __cplusplus
    extern "C" {
#endif

void image_overlay_blur(unsigned int width, unsigned int height, unsigned char components, unsigned char* blur1r, unsigned char* blur2r);
void image_overlay(unsigned int width, unsigned int height, unsigned char components, unsigned char* image, unsigned char* blur);
void image_mix(unsigned int width, unsigned int height, unsigned char components, float coef, unsigned char* image1, unsigned char* image2);
void image_filter_balance(unsigned int width, unsigned int height, unsigned char components, float mix, unsigned char* image, unsigned char* blur1r, unsigned char* blur2r);

#ifdef __cplusplus
    }
#endif
#endif  /* FILTER_BALANCE_H */

#ifdef FILTER_BALANCE_IMPLEMENTATION
#include <stdlib.h>
#include <math.h>

void image_overlay_blur(unsigned int width, unsigned int height, unsigned char components, unsigned char* blur1r, unsigned char* blur2r)
{
    size_t image_size = height * width * components;
    if ((blur1r != NULL) && (blur2r != NULL))
    {
        for (size_t i = 0; i < image_size; i++)
        {
            float b1r = blur1r[i];
            float b2r = blur2r[i];

            /* overlay 1 */
            float base = 255.0f - b1r;
            float overlay = b2r;
            float retval1 = base;
            if (base > 127.5f)
            {
                retval1 = 255.0f - retval1;
                overlay = 255.0f - overlay;
            }
            retval1 *= overlay;
            retval1 += retval1;
            retval1 /= 255.0f;
            if (base > 127.5f)
            {
                retval1 = 255.0f - retval1;
            }

            /* overlay 2 */
            base = 255.0f - b2r;
            overlay = b1r;
            float retval2 = base;
            if (base > 127.5f)
            {
                retval2 = 255.0f - retval2;
                overlay = 255.0f - overlay;
            }
            retval2 *= overlay;
            retval2 += retval2;
            retval2 /= 255.0f;
            if (base > 127.5f)
            {
                retval2 = 255.0f - retval2;
            }

            blur1r[i] = (retval1 < 0.0f) ? 0 : ((retval1 < 255.0f) ? (uint8_t)(retval1 + 0.5f) : 255);
            blur2r[i] = (retval2 < 0.0f) ? 0 : ((retval2 < 255.0f) ? (uint8_t)(retval2 + 0.5f) : 255);
        }
    }
}

void image_overlay(unsigned int width, unsigned int height, unsigned char components, unsigned char* image, unsigned char* blur)
{
    size_t image_size = height * width * components;
    if ((image != NULL) && (blur != NULL))
    {
        for (size_t i = 0; i < image_size; i++)
        {
            float im = image[i];
            float bl = blur[i];

            /* overlay*/
            float base = im;
            float overlay = bl;
            float retval = base;
            if (base > 127.5f)
            {
                retval = 255.0f - retval;
                overlay = 255.0f - overlay;
            }
            retval *= overlay;
            retval += retval;
            retval /= 255.0f;
            if (base > 127.5f)
            {
                retval = 255.0f - retval;
            }

            blur[i] = (retval < 0.0f) ? 0 : ((retval < 255.0f) ? (uint8_t)(retval + 0.5f) : 255);
        }
    }
}

void image_mix(unsigned int width, unsigned int height, unsigned char components, float coef, unsigned char* image1, unsigned char* image2)
{
    size_t image_size = height * width * components;
    if ((image1 != NULL) && (image2 != NULL))
    {
        for (size_t i = 0; i < image_size; i++)
        {
            float im1 = image1[i];
            float im2 = image2[i];

            /* overlay*/
            float retval = coef * im1 + (1.0 - coef) * im2;

            image2[i] = (retval < 0.0f) ? 0 : ((retval < 255.0f) ? (uint8_t)(retval + 0.5f) : 255);
        }
    }
}

void image_filter_balance(unsigned int width, unsigned int height, unsigned char components, float mix, unsigned char* image, unsigned char* blur1r, unsigned char* blur2r)
{
    image_overlay_blur(width, height, components, blur1r, blur2r);
    image_overlay(width, height, components, image, blur1r);
    image_overlay(width, height, components, blur1r, blur2r);
    image_mix(width, height, components, mix, blur2r, image);
}

#endif  /* FILTER_BALANCE_IMPLEMENTATION */
