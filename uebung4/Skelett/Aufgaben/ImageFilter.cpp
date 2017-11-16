#include "ImageFilter.h"

namespace cg
{
	namespace filter
	{
		namespace
		{
			/** Computes and stores gaussian values in given kernel for the given sigma value */
			void setGaussianValues(Kernel& k, float sigma)
			{
				///////
				// TODO
				// Implement computation of gaussian values.
				// Store values in k.

				
			}
		}

		Kernel::Kernel(std::pair<unsigned int, unsigned int> extents)
			: m_data( (std::get<0>(extents) * 2 + 1) * (std::get<1>(extents) * 2 + 1) ), m_extents(extents)
		{
			std::fill(m_data.begin(), m_data.end(), 0.0f);
			setValue(0, 0, 1.0f);
		}

		float Kernel::getValue(int x, int y) const
		{
			// TODO check range

			return m_data[getDataIndex(x, y)];
		}

		float* Kernel::data()
		{
			return m_data.data();
		}

		float const* Kernel::data() const
		{
			return m_data.data();
		}

		std::pair<int, int> Kernel::getHorizontalRange() const
		{
			return std::make_pair( -static_cast<int>(std::get<0>(m_extents)), static_cast<int>(std::get<0>(m_extents)));
		}

		std::pair<int, int> Kernel::getVerticalRange() const
		{
			return std::make_pair( -static_cast<int>(std::get<1>(m_extents)), static_cast<int>(std::get<1>(m_extents)));
		}

		void Kernel::setValue(int x, int y, float v)
		{
			m_data[getDataIndex(x, y)] = v;
		}

		size_t Kernel::getDataIndex(int x, int y) const
		{
			//TODO bound check

			x += std::get<0>(m_extents);
			y += std::get<1>(m_extents);

			return y * (std::get<0>(m_extents) * 2 + 1) + x;
		}


		Kernel build2DGaussianKernel(std::pair<unsigned int, unsigned int> extents, float sigma)
		{
			Kernel k(extents);

			setGaussianValues(k, sigma);

			return k;
		}

		Kernel build1DHorizontalGaussianKernel(unsigned int extent, float sigma)
		{
			Kernel k(std::make_pair(extent,0));

			setGaussianValues(k, sigma);

			return k;
		}

		Kernel build1DVerticalGaussianKernel(unsigned int extent, float sigma)
		{
			Kernel k(std::make_pair(0, extent));

			setGaussianValues(k, sigma);

			return k;
		}

		Kernel buildEdgeDetectionKernel()
		{
			Kernel k(std::make_pair(1, 1));
			
			///////
			// TODO
			// Set to edge detection kernel

			return k;
		}
	}
}