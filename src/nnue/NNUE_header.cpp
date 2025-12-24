#include "common.h"
#include "NNUE_header.h"

NNUE_header::NNUE_header(std::istream& net_file) noexcept
{
	const auto read_numerics=[&]<numeric... types>(types&... values)
	{
		((values=read_little_endian<decltype(values)>(net_file)),...);
	};
	read_numerics(version, hash, str_size);
	assert(version==decltype(version){0x07AF32F16});

	description.resize(str_size);
	net_file.read(description.data(),str_size);
}

std::ostream& operator<<(std::ostream& os, NNUE_header header) noexcept
{
	return os << "version: "       << header.version
			  << "\nhash: "        << header.hash
			  << "\nstr_size: "    << header.str_size
			  << "\ndescription: " << header.description;
};
