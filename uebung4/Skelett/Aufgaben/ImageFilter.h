#ifndef ImageFilter_hpp
#define ImageFilter_hpp

#include <algorithm>
#include <utility>

#include "Image.h"

namespace cg
{
	namespace filter
	{
		enum BorderPolicy { CLAMP_TO_EDGE, MIRROR, REPEAT };

		// use empty namespace for "private" functions
		namespace
		{
			/**
			 * Applies offset to image coordinates.
			 * Guarantees to return valid coordinates even if the offset is
			 * theoretically out of bounds.
			 *
			 * @param image Input image - defines the valid image region.
			 * @param coordinates Input coordinates within the valid image region.
			 * @param offset Offset values that are applied to the input coordinates.
			 * @param border_policy Policy for handling out of bounds coordinates.
			 */
			template <cg::color_space_t color_space>
			std::pair<unsigned int, unsigned int> offsetImageCoordinates(
				image<color_space> const& image,
				std::pair<unsigned int, unsigned int> coordinates,
				std::pair<int, int> offset,
				BorderPolicy border_policy)
			{
				if (std::get<0>(coordinates) < -(std::get<0>(offset)) ||
					(std::get<0>(coordinates) + std::get<0>(offset)) > image.get_width() ||
					std::get<1>(coordinates) < -(std::get<1>(offset)) ||
					(std::get<1>(coordinates) + std::get<1>(offset)) > image.get_height())
				{
					switch (border_policy)
					{
					case cg::filter::CLAMP_TO_EDGE:
						return std::make_pair(
							//std::clamp(static_cast<long>(std::get<0>(coordinates)) + std::get<0>(offset), 0, image.get_width()),
							//std::clamp(static_cast<long>(std::get<1>(coordinates)) + std::get<1>(offset), 0, image.get_height())
							std::min(std::max(static_cast<int>(std::get<0>(coordinates)) + std::get<0>(offset), 0), static_cast<int>(image.get_width()-1)),
							std::min(std::max(static_cast<int>(std::get<1>(coordinates)) + std::get<1>(offset), 0), static_cast<int>(image.get_height()-1))
							);

						break;
					case cg::filter::MIRROR:
						///////
						// TODO
						// Implement a border policy that mirrors out of bounds coordinates
						// at the image border.
						/* TODO: REPLACE */ return coordinates; /* TODO: REPLACE */
						break;
					case cg::filter::REPEAT:
						///////
						// TODO
						// Implement a border policy that virtually repeats the image for
						// out of bounds coordinates.
						/* TODO: REPLACE */ return coordinates; /* TODO: REPLACE */
						break;
					default:
						break;
					}
				}
				else
				{
					return std::make_pair(
						std::get<0>(coordinates) + std::get<0>(offset),
						std::get<1>(coordinates) + std::get<1>(offset)
						);
				}
			}
		}

		/**
		 * Basic Filter-Kernel class.
		 */
		class Kernel
		{
		public:
			/**
			 * Construct a new kernel with given extents in x and y direction.
			 * Note that the overal size of the kernel is (2 * extent_X +1) x (2 * extent_Y +1),
			 * i.e. a kernel of extent {2,2} has a size of 5x5.
			 */
			Kernel(std::pair<unsigned int, unsigned int> extents);

			/**
			 * Get a single entry of the filter kernel.
			 * Coordinates are given relative to the center of the kernel.
			 */
			float getValue(int x, int y) const;

			float* data();

			float const* data() const;

			/**
			 * Returns the min and max horizontal coordinate within the kernel.
			 * Coordinates are given relative to the center of the kernel,
			 * i.e. -2 and 2 for a kernel with a horizontal extent of 2.
			 */
			std::pair<int, int> getHorizontalRange() const;
			/**
			* Returns the min and max vertical coordinate within the kernel.
			* Coordinates are given relative to the center of the kernel,
			* i.e. -2 and 2 for a kernel with a vertical extent of 2.
			*/
			std::pair<int, int> getVerticalRange() const;

			/**
			 * Sets the value of the kernel for a given position.
			 * Note that the position is given relative to the center of the kernel.
			 */
			void setValue(int x, int y, float v);

		private:
			std::vector<float> m_data;
			std::pair<unsigned int, unsigned int> m_extents;

			/**
			* Compute actual data index for a position given relative to the center of the kernel.
			*/
			size_t getDataIndex(int x, int y) const;
		};

		/** Create 2D guassian filter kernel. */
		Kernel build2DGaussianKernel(std::pair<unsigned int, unsigned int> extents, float sigma);

		/** Create 1D horizontal guassian filter kernel for a seperated gaussian filter. */
		Kernel build1DHorizontalGaussianKernel(unsigned int extent, float sigma);

		/** Create 1D vertical guassian filter kernel for a seperated gaussian filter. */
		Kernel build1DVerticalGaussianKernel(unsigned int extent, float sigma);

		/** Create a simple 3x3 edge detection filter kernel. */
		Kernel buildEdgeDetectionKernel();

		/** Apply a filter kernel to an image. */
		template <color_space_t color_space>
		image<color_space> filterImage(image<color_space> const& original, Kernel const& filter_kernel, BorderPolicy border_policy = CLAMP_TO_EDGE);
	}
}

template <cg::color_space_t color_space>
cg::image<color_space> cg::filter::filterImage(image<color_space> const& original, Kernel const& filter_kernel, BorderPolicy border_policy)
{
	cg::image<color_space> filtered(original.get_width(), original.get_height());

	for (unsigned int j = 0; j < original.get_height(); ++j)
	{
		for (unsigned int i = 0; i < original.get_width(); ++i)
		{
			cg::image<color_space>::tuple_type pixel_value;
			for (auto& channel : pixel_value)
				channel = 0;

			///////
			// TODO
			// Implement image filtering using the given filter kernel.
			// Use "pixel_value" as intermediate storage for the computation and store the final result in "filtered".
			// Iterate over the kernel and use the function "offsetImageCoordinates" to get valid image coordinates for
			// accessing the original image data.
			
		}
	}

	return filtered;
}

#endif // !ImageFilter_hpp
