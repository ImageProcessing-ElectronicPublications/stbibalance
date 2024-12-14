# stbibalance

Balance filter based of Gauss blur.
Used:
* Gauss blur: [iir_gauss_blur](https://github.com/arkanis/iir_gauss_blur).
* STB: [stb](https://github.com/nothings/stb).

Balance is a thin filter (unlike others that use a thick difference between
the original image and its blurred version) that uses the difference between
a single and double radius blurred version of the original image
(single and double `sigma`).
This difference is used not directly, but by applying an `overlay` four times,
with the colors inverted.

This filter was first applied in [STEX](https://github.com/ImageProcessing-ElectronicPublications/scantailor-experimental) (2024)
in a single-component version (Y, the values of the color components were aligned
in accordance with the brightness values before and after the filter).

Here this filter is implemented in a full-color version.

## Usage

`./stbibalance [-h] [-s sigma] [-m mixed] input-file output-file`

`-s sigma`     The sigma of the gauss normal distribution (number >= 0.5).
               Larger values result in a stronger blur.

`-m mixed`     The mixed coefficient.

`-h`           display this help and exit.

You can use either sigma to specify the strengh of the blur.

The performance is independent of the blur strengh (sigma). This tool is an
implementation of the paper "Recursive implementaion of the Gaussian filter"
by Ian T. Young and Lucas J. van Vliet.

stb_image and stb_image_write by Sean Barrett and others is used to read and
write images.

## Installation

- Clone the repo or download the source `git clone --recurse-submodules https://github.com/ImageProcessing-ElectronicPublications/stbibalance`
- Execute `make`
- Done. Either use the `stbibalance` executable directly or copy it somewhere in your PATH.

## Links

* STB: [stb](https://github.com/nothings/stb).
* Gauss blur: [iir_gauss_blur](https://github.com/arkanis/iir_gauss_blur).
* Examples: [stbibalance-examples](https://github.com/ImageProcessing-ElectronicPublications/stbibalance-examples).
