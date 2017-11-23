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
				// Implement computation of gaussian values.
				// Store values in k.
				// problem is, we were not able to compile because of intel
				float const M_PI = 3.1417;
				int x_middle = k.getHorizontalRange().first / 2;
				int y_middle = k.getVerticalRange().second / 2;
				int x_distance = 0;
				int y_distance = 0;
				float value = 0.0;
				for (int i = k.getHorizontalRange().first; i <= k.getHorizontalRange().second; i++) {
					for (int j = k.getVerticalRange().first; j <= k.getVerticalRange().second; j++) {
						x_distance = std::abs(j - x_middle);
						y_distance = std::abs(j - y_middle);
						value = (1.0f / (float)std::sqrt(2 * M_PI *sigma)) * std::pow(exp(1.0), ((-(std::pow(2, x_distance)) + (std::pow(2, y_distance)))/2*std::pow(2, sigma)));
						k.setValue(j, i, value);
					}
				}
				
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
			
			// Set to edge detection kernel
			k.setValue(0, 0, 0);
			k.setValue(1, 0, -1);
			k.setValue(2, 0, 0);
			k.setValue(0, 1, -1);
			k.setValue(1, 1, 4);
			k.setValue(2, 1, -1);
			k.setValue(0, 2, 0);
			k.setValue(1, 2, -1);
			k.setValue(2, 2, 0);

			return k;
		}
	}
}