#ifndef __TYPES_HPP__
#define __TYPES_HPP__

#include <cstdint>
#include <vector>

using height_t = std::uint32_t;
using width_t  = std::uint32_t;

using Image1D_8U  = std::vector<std::uint8_t>;
using Image1D_16U = std::vector<std::uint16_t>;

#endif // __TYPES_HPP__
