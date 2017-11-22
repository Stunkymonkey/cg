#version 430

/** Input image texture */
layout(RGBA32F) readonly uniform image2D src_tx2D;
/** Output image texture */
layout(RGBA32F) writeonly uniform image2D tgt_tx2D;

/** Texture with kernel values */
layout(R32F) readonly uniform image2D kernel_tx2D;

/** Image size */
uniform ivec2 img_size;
/** Extent of filter kernel */
uniform int kernel_extents;
/**
 * Offset direction.
 * For horizontal filter pass it is set to vec2(1,0).
 * For the vertical filter pass it is set to vec2(0,1).
 */
uniform ivec2 pixel_offset;

layout(local_size_x = 4, local_size_y = 4, local_size_z = 1) in;

void main()
{
    ivec3 global_pos = ivec3(gl_GlobalInvocationID.xyz);

    // early exit if global id outside of grid (due to group size)
    if( global_pos.x >= img_size.x || global_pos.y >= img_size.y )
        return;

    // Intermediate storage for compute sum of multiplications
    vec4 pixel_value = vec4(0.0);
	
    ///////
    // TODO
    // Iterate over the 1D kernel and sum up the multiplication
    // of image values with kernel values in the variable pixel_value.
    // The extent of the kernel is given by the uniform variable kernel_extents.
    //
    // To access the kernel values, use the function
    //   imageLoad(kernel_tx2D, kernel_coords).x
    // where kernel_coords is the kernel (texture) coordinates given in pixel.
    //
    // To access the source image values, use the function
    //   imageLoad(src_tx2D, img_coords)
    // where img_coords is the image (texture) coordinates given in pixel.
    // Use the uniform variable pixel_offset to compute the
    // image coordinates.
    //
    // Check https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/imageLoad.xhtml
    // for details regarding the imageLoad function.
	result = exp(-(i*i)/(double)(2*sigma*sigma));

    imageStore(tgt_tx2D, global_pos.xy, pixel_value);
}