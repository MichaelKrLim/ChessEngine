#ifndef NNUE_header_h_INCLUDED
#define NNUE_header_h_INCLUDED

#include <cassert>
#include <fstream>

struct NNUE_header
{
	NNUE_header(std::ifstream& net_file) noexcept;

	std::uint32_t version{0}
				, hash{0}
				, str_size{0};
	std::string description{""};
};

std::ostream& operator<<(std::ostream& os, NNUE_header header) noexcept;

#endif // NNUE_header_h_INCLUDED
