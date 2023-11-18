#ifndef CL_FUNC_H
#define CL_FUNC_H

#include <cstdint>

#define CL_TARGET_OPENCL_VERSION 210

void mandelbrot_ocl_calc( uint8_t *buffer, std::size_t width, std::size_t height, std::size_t channels );

#endif // #ifndef CL_FUNC_H