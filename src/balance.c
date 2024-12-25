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
#define FILTER_BALANCE_IMPLEMENTATION
#include "filter_balance.h"

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

unsigned char* image_copy(unsigned int width, unsigned int height, unsigned char components, unsigned char* image)
{
    size_t image_size = height * width * components;
    unsigned char* dest = (unsigned char*)malloc(image_size * sizeof(unsigned char));
    if (dest != NULL)
    {
        for (size_t i = 0; i < image_size; i++)
        {
            dest[i] = image[i];
        }
    }
    return dest;
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
    unsigned char* image = stbi_load(argv[optind], &width, &height, &components, 0);
    if (image == NULL)
    {
        fprintf(stderr, "Failed to load %s: %s.\n", argv[optind], stbi_failure_reason());
        return 2;
    }

    unsigned char* blur1r = image_copy(width, height, components, image);
    if (blur1r == NULL)
    {
        fprintf(stderr, "ERROR: not use memmory\n");
        return 3;
    }

    unsigned char* blur2r = image_copy(width, height, components, image);
    if (blur2r == NULL)
    {
        fprintf(stderr, "ERROR: not use memmory\n");
        return 3;
    }

    iir_gauss_blur(width, height, components, blur1r, sigma);
    iir_gauss_blur(width, height, components, blur2r, (sigma + sigma));

    image_filter_balance(width, height, components, mix, image, blur1r, blur2r);

    if ( stbi_write_png(argv[optind+1], width, height, components, image, 0) == 0 )
    {
        fprintf(stderr, "Failed to save %s.\n", argv[optind+1]);
        return 4;
    }

    return 0;
}
