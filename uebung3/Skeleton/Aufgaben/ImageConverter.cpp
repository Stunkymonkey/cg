#include "ImageConverter.h"

#include <cmath>

cg::image<cg::color_space_t::HSV> cg::image_converter::rgb_to_hsv(const image<color_space_t::RGB>& original)
{
	// Convert RGB to HSV
	cg::image<cg::color_space_t::HSV> converted(original.get_width(), original.get_height());

	for (unsigned int j = 0; j < original.get_height(); ++j)
	{
		for (unsigned int i = 0; i < original.get_width(); ++i)
		{
			///////
			// TODO
			// Convert RGB to HSV image.
			// All values are in range [0.0, 1.0].
		}
	}

	return converted;
}

cg::image<cg::color_space_t::RGB> cg::image_converter::hsv_to_rgb(const image<color_space_t::HSV>& original)
{
	// Convert HSV to RGB
	image<color_space_t::RGB> converted(original.get_width(), original.get_height());

	for (unsigned int j = 0; j < original.get_height(); ++j)
	{
		for (unsigned int i = 0; i < original.get_width(); ++i)
		{
			///////
			// TODO
			// Convert HSV to RGB image.
			// All values are in range [0.0, 1.0].
		}
	}

	return converted;
}

cg::image<cg::color_space_t::Gray> cg::image_converter::rgb_to_gray(const image<color_space_t::RGB>& original)
{
	// Convert RGB to grayscale
	image<color_space_t::Gray> converted(original.get_width(), original.get_height());

	for (unsigned int j = 0; j < original.get_height(); ++j)
	{
		for (unsigned int i = 0; i < original.get_width(); ++i)
		{
			///////
			// TODO
			// Convert RGB to grayscale image.
			// All values are in range [0.0, 1.0].
		}
	}

	return converted;
}

cg::image<cg::color_space_t::BW> cg::image_converter::gray_to_bw(const image<color_space_t::Gray>& original)
{
	// Convert grayscale to black and white
	image<color_space_t::BW> converted(original.get_width(), original.get_height());

	for (unsigned int j = 0; j < original.get_height(); ++j)
	{
		for (unsigned int i = 0; i < original.get_width(); ++i)
		{
			///////
			// TODO
			// Convert grayscale to black-and-white image.
			// All grayscale values are in range [0.0, 1.0], bw values are either 1.0 (white) or 0.0 (black).
		}
	}

	return converted;
}