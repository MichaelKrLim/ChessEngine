#ifndef ENUM_MAP_H
#define ENUM_MAP_H

#include <array>
#include <type_traits>

template <typename enum_type, typename mapped_type, std::size_t number_of_values>
	requires std::is_enum_v<enum_type> //&& std::is_default_constructible_v<mapped_type>
class Enum_map
{
	public:
	using underlying=std::underlying_type_t<enum_type>;

	static constexpr auto initialized_with(const mapped_type& value) noexcept
	{
		const auto create = [&]<std::size_t... indices>(std::index_sequence<indices...>)
		{
			return Enum_map{{[&](auto){ return value; }(indices)...}};
		};

		return create(std::make_index_sequence<number_of_values>{});
	}

	constexpr auto& operator[](enum_type v) noexcept { return data[static_cast<underlying>(v)]; }
	constexpr const auto& operator[](enum_type v) const noexcept { return data[static_cast<underlying>(v)]; }

	constexpr auto begin() noexcept { return data.begin(); }
	constexpr auto end() noexcept { return data.end(); }

	constexpr auto begin() const noexcept { return data.begin(); }
	constexpr auto end() const noexcept { return data.end(); }

	constexpr auto cbegin() const noexcept { return data.cbegin(); }
	constexpr auto cend() const noexcept { return data.cend(); }

	constexpr auto size() const noexcept { return number_of_values; }

	constexpr friend auto operator<=>(const Enum_map&, const Enum_map&) = default;

	std::array<mapped_type,number_of_values> data{};
};

template <typename enum_type, typename mapped_type>
using Enum_map_from_size = constexpr Enum_map<enum_type, mapped_type, static_cast<std::size_t>(enum_type::size)>;

#endif
