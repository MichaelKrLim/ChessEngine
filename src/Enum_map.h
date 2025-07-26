#ifndef Enum_map_h_INCLUDED
#define Enum_map_h_INCLUDED

#include <array>
#include <type_traits>
#include <utility>

template <typename enum_type, typename mapped_type, std::size_t number_of_values>
	requires std::is_enum_v<enum_type> && std::is_default_constructible_v<mapped_type>
class Enum_map
{
	public:

	constexpr static Enum_map initialised_with(const mapped_type& value) noexcept
	{
		Enum_map result;
		for(std::size_t i{0}; i<number_of_values; ++i)
			result[i]=value;
		return result;
	}

	constexpr auto& operator[](enum_type v) noexcept { return data[std::to_underlying(v)]; }
	constexpr const auto& operator[](enum_type v) const noexcept { return data[std::to_underlying(v)]; }

	constexpr auto begin() noexcept { return data.begin(); }
	constexpr auto end() noexcept { return data.end(); }

	constexpr auto begin() const noexcept { return data.begin(); }
	constexpr auto end() const noexcept { return data.end(); }

	constexpr auto cbegin() const noexcept { return data.cbegin(); }
	constexpr auto cend() const noexcept { return data.cend(); }

	constexpr auto size() const noexcept { return number_of_values; }

	constexpr friend auto operator<=>(const Enum_map&, const Enum_map&) = default;

	std::array<mapped_type, number_of_values> data{};
};

template <typename enum_type, typename mapped_type> 
using Enum_map_from_size = Enum_map<enum_type, mapped_type, std::to_underlying(enum_type::size)>;

#endif // Enum_map_h_INCLUDED
