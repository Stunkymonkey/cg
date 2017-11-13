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
			// Convert RGB to HSV image.
			// All values are in range [0.0, 1.0].
			float min, max, delta;
			float r, g, b;
			int h = 0;
			r = original(i, j)[0];
			g = original(i, j)[1];
			b = original(i, j)[2];
			min = std::min(std::min(r, g), b);
			max = std::max(std::max(r, g), b);
			converted(i, j)[2] = max;
			delta = max-min;
			if( max != 0 ){
                converted(i, j)[1] = delta / max;// s 
        	} else {
                // if r = g = b = 0 then s = 0, v is undefined 
                converted(i, j)[1] = 0; 
                converted(i, j)[2] = -1; 
                break;
 			}
			if( r == max ) // between yellow & magenta
                h = ( g - b ) / delta;
			else if( g == max ) // between cyan & yellow
				h = 2 + ( b - r ) / delta;
			else // between magenta & cyan
				h = 4 + ( r - g ) / delta;
			h *= 60;//degrees        
			if( h < 0 )
				h += 360;
			converted(i, j)[0] = h / 360.0;
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
			// Convert HSV to RGB image.
			// All values are in range [0.0, 1.0].
			float c, HPrime, x, m;
			float h, s, v;
			float r, g, b;
			h = converted(i, j)[0] * 360.0;
			s = converted(i, j)[1];
			v = converted(i, j)[2];
			c = v * s; // Chroma
			HPrime = std::fmod(h / 60.0, 6);
			x = c * (1 - std::fabs(fmod(HPrime, 2) - 1));
			m = v - c;
			
			if(0 <= HPrime && HPrime < 1) {
				r = c;
				g = x;
				b = 0;
			} else if(1 <= HPrime && HPrime < 2) {
				r = x;
				g = c;
				b = 0;
			} else if(2 <= HPrime && HPrime < 3) {
				r = 0;
				g = c;
				b = x;
			} else if(3 <= HPrime && HPrime < 4) {
				r = 0;
				g = x;
				b = c;
			} else if(4 <= HPrime && HPrime < 5) {
				r = x;
				g = 0;
				b = c;
			} else if(5 <= HPrime && HPrime < 6) {
				r = c;
				g = 0;
				b = x;
			} else {
				r = 0;
				g = 0;
				b = 0;
			}
			converted(i, j)[0] = r + m;
			converted(i, j)[1] = g + m;
			converted(i, j)[2] = b + m;
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
			// Convert RGB to grayscale image.
			// All values are in range [0.0, 1.0].
			converted(i, j)[0] = ( (0.3 * original(i, j)[0]) + (0.59 * original(i, j)[1]) + (0.11 * original(i, j)[2]) );
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
			// Convert grayscale to black-and-white image.
			// All grayscale values are in range [0.0, 1.0], bw values are either 1.0 (white) or 0.0 (black).
			float value = ( (0.3 * original(i, j)[0]) + (0.59 * original(i, j)[1]) + (0.11 * original(i, j)[2]) );
			if (value > 0.5) {
				converted(i, j)[0] = 1.0;
			} else {
				converted(i, j)[0] = 0.0;
			}
		}
	}

	return converted;
}