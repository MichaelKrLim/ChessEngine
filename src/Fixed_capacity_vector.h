#ifndef Fixed_capacity_vector_h_INCLUDED
#define Fixed_capacity_vector_h_INCLUDED

#include <array>
#include <optional>
#include <stdexcept>

template <typename T, std::size_t size_v>
class Fixed_capacity_vector
{
	public:

	constexpr T& operator[](const std::size_t& index) const noexcept { return data_[index].value(); }

	constexpr std::size_t size() const noexcept { return size; };
	constexpr void push_back(const T& value)
	{
		if(used_capacity_>=size_v)
			throw std::out_of_range{"Pushed element beyond capacity"};
		
		data_[used_capacity_++]=value;
	}
	constexpr void erase_if(const auto& predicate) noexcept
	{
		for(std::size_t i{0}; i<used_capacity_; ++i)
		{
			if(predicate(data_[i].value()))
				data_[i] = std::nullopt;
		}
		std::size_t new_index{0};
		for(std::size_t old_index{0}; old_index<used_capacity_; ++old_index)
		{
			if(data_[old_index]!=std::nullopt)
			{
				if(old_index!=new_index)
					data_[new_index]=std::move(data_[old_index]);
				++new_index;
			}
		}
		used_capacity_ = new_index;
	}
	constexpr void clear() noexcept
	{
		data_ = {};
		used_capacity_ = 0;
	}
	constexpr bool empty() const noexcept { return used_capacity_ == 0; }

	constexpr auto begin() const noexcept { return data_.begin(); }
	constexpr auto end() const noexcept { return data_.begin()+used_capacity_; }

	constexpr auto begin() noexcept {return data_.begin(); }
	constexpr auto end() noexcept { return data_.begin()+used_capacity_; }

	constexpr auto cbegin() const noexcept { return data_.cbegin(); }
	constexpr auto cend() const noexcept { return data_.cbegin()+used_capacity_; }

	private:

	std::size_t used_capacity_{0};
	std::array<std::optional<T>, size_v> data_;
};

#endif // Fixed_capacity_vector_h_INCLUDED
