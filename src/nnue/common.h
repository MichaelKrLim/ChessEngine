#ifndef common_h_INCLUDED
#define common_h_INCLUDED

#include <concepts>
#include <istream>

struct Dimensions { std::size_t features{0}, neurons{0}; };

template <typename type>
concept numeric=std::integral<type> || std::floating_point<type>;

template <numeric numeric_type>
numeric_type read_little_endian(std::istream& is)
{
	numeric_type result;
	is.read(reinterpret_cast<char*>(&result),sizeof(numeric_type));
	return result;
}

template <typename numeric_type>
	requires std::is_reference_v<numeric_type> && numeric<std::decay_t<numeric_type>>
auto read_little_endian(std::istream& is)
{
	return read_little_endian<std::decay_t<numeric_type>>(is);
}

#endif // common_h_INCLUDED
