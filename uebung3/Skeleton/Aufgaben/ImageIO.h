#pragma once

#include "Image.h"

#include <fstream>
#include <string>

namespace cg
{
	/// <summary>
	/// Class for loading and saving images to/from file
	/// This uses the simple Netpbm image formats PBM, PGM and PPM
	/// PBM: Netpbm bi-level image format (http://netpbm.sourceforge.net/doc/pbm.html)
	/// PGM: Netpbm grayscale image format (http://netpbm.sourceforge.net/doc/pgm.html)
	/// PPM: Netpbm color image format (http://netpbm.sourceforge.net/doc/ppm.html)
	/// </summary>
	class image_io
	{
	public:
		/// <summary>
		/// Load RGB image from file
		/// </summary>
		/// <param name="path">Path to image file</param>
		/// <returns>RGB image</returns>
		image<color_space_t::RGB> load_rgb_image(const std::string& path) const;

		/// <summary>
		/// Load grayscale image from file
		/// </summary>
		/// <param name="path">Path to image file</param>
		/// <returns>Grayscale image</returns>
		image<color_space_t::Gray> load_grayscale_image(const std::string& path) const;

		/// <summary>
		/// Load black and white image from file
		/// </summary>
		/// <param name="path">Path to image file</param>
		/// <returns>Black and white image</returns>
		image<color_space_t::BW> load_bw_image(const std::string& path) const;

		/// <summary>
		/// Save RGB image to file
		/// </summary>
		/// <param name="path">Path to image file</param>
		/// <param name="image">RGB image</param>
		void save_rgb_image(const std::string& path, const image<color_space_t::RGB>& image) const;

		/// <summary>
		/// Save grayscale image to file
		/// </summary>
		/// <param name="path">Path to image file</param>
		/// <param name="image">Grayscale image</param>
		void save_grayscale_image(const std::string& path, const image<color_space_t::Gray>& image) const;

		/// <summary>
		/// Save black and white image to file
		/// </summary>
		/// <param name="path">Path to image file</param>
		/// <param name="image">Black and white image</param>
		void save_bw_image(const std::string& path, const image<color_space_t::BW>& image) const;

	private:
		/// <summary>
		/// Struct for storing header data
		/// </summary>
		struct header
		{
			enum file_t
			{
				INVALID, PLAIN_PBM, PLAIN_PGM, PLAIN_PPM, PBM, PGM, PPM
			}
			file_type;

			unsigned int width;
			unsigned int height;
			unsigned int max_value;
		};

		/// <summary>
		/// Read file header
		/// </summary>
		/// <param name="stream">Input stream</param>
		/// <returns>File header</returns>
		header load_header(std::ifstream& stream) const;

		/// <summary>
		/// Write file header
		/// </summary>
		/// <param name="stream">Output stream</param>
		/// <param name="file_header">File header</param>
		void save_header(std::ofstream& stream, const header& file_header) const;
	};
}