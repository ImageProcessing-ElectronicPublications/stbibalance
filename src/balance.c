#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#define IIR_GAUSS_BLUR_IMPLEMENTATION
#include "iir_gauss_blur.h"

void usage(char* progname)
{
    fprintf(stderr,
        "%s %s %s\n",
        "Usage:", progname, "[-h] [-s sigma] [-m mixed] input-file output.png\n"
        "Balance filter an image and save it as PNG.\n"
     );
}

void help(float sigma, float mix)
{
    fprintf(stderr,
        "%s %f %s %f %s\n",
        "  -s sigma     The sigma of the gauss normal distribution (number >= 0.5, default =", sigma, ").\n"
        "               Larger values result in a stronger blur.\n"
        "  -m mixed     The mixed coefficient (number, default =", mix, ").\n"
        "  -h           display this help and exit.\n"
        "\n"
        "You can use either sigma to specify the strengh of the blur.\n"
        "\n"
        "The performance is independent of the blur strengh (sigma). This tool is an\n"
        "implementation of the paper \"Recursive implementaion of the Gaussian filter\"\n"
        "by Ian T. Young and Lucas J. van Vliet.\n"
        "\n"
        "stb_image and stb_image_write by Sean Barrett and others is used to read and\n"
        "write images.\n"
    );
}

uint8_t* image_copy(unsigned int width, unsigned int height, unsigned char components, unsigned char* image)
{
    size_t image_size = height * width * components;
    uint8_t* dest = (unsigned char*)malloc(image_size * sizeof(unsigned char));
    if (dest != NULL)
    {
        for (size_t i = 0; i < image_size; i++)
        {
            dest[i] = image[i];
        }
    }
    return dest;
}

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

int main(int argc, char** argv)
{
    float sigma = 10.0f;
    float mix = 1.0f;

    int opt;
    while ( (opt = getopt(argc, argv, "s:m:h")) != -1 )
    {
        switch(opt)
        {
            case 's':
                sigma = strtof(optarg, NULL);
                break;
            case 'm':
                mix = strtof(optarg, NULL);
                break;
            case 'h':
                usage(argv[0]);
                help(sigma, mix);
                return 0;
            default:
                usage(argv[0]);
                return 1;
        }
    }

    // Need at least two filenames after the last option
    if (argc < optind + 2)
    {
        usage(argv[0]);
        return 1;
    }

    int width = 0, height = 0, components = 1;
    uint8_t* image = stbi_load(argv[optind], &width, &height, &components, 0);
    if (image == NULL)
    {
        fprintf(stderr, "Failed to load %s: %s.\n", argv[optind], stbi_failure_reason());
        return 2;
    }

    uint8_t* blur1r = image_copy(width, height, components, image);
    if (blur1r == NULL)
    {
        fprintf(stderr, "ERROR: not use memmory\n");
        return 3;
    }

    uint8_t* blur2r = image_copy(width, height, components, image);
    if (blur2r == NULL)
    {
        fprintf(stderr, "ERROR: not use memmory\n");
        return 3;
    }

    iir_gauss_blur(width, height, components, blur1r, sigma);
    iir_gauss_blur(width, height, components, blur2r, (sigma + sigma));

    image_overlay_blur(width, height, components, blur1r, blur2r);
    image_overlay(width, height, components, image, blur1r);
    image_overlay(width, height, components, blur1r, blur2r);
    image_mix(width, height, components, mix, blur2r, image);

    if ( stbi_write_png(argv[optind+1], width, height, components, image, 0) == 0 )
    {
        fprintf(stderr, "Failed to save %s.\n", argv[optind+1]);
        return 4;
    }

    return 0;
}
