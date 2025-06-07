#ifndef Fixed_capacity_vector_h_INCLUDED
#define Fixed_capacity_vector_h_INCLUDED

#include <array>
#include <concepts>
#include <type_traits>

template <typename T, std::size_t size_v> requires (std::is_trivially_destructible_v<T> && std::default_initializable<T>)
class Fixed_capacity_vector
{
	public:

	constexpr T& operator[](const std::size_t index) noexcept
	{
		if(index >= used_capacity_)
			used_capacity_ = index+1;
		return data_[index];
	}

	constexpr T& front() noexcept { return data_[0]; }

	constexpr T& back() noexcept { return data_[used_capacity_-1]; }

	constexpr std::size_t size() const noexcept { return used_capacity_; };

	constexpr void push_back(const T& value)
	{
		data_[used_capacity_++]=value;
	}

	constexpr void pop_back()
	{
		--used_capacity_;
	}

	constexpr void erase_if(const auto& predicate) noexcept
	{
		std::size_t new_index{0};
		for(std::size_t old_index{0}; old_index<used_capacity_; ++old_index)
		{
			if(!predicate(data_[old_index]))
			{
				if(old_index!=new_index)
					data_[new_index]=std::move(data_[old_index]);
				++new_index;
			}
		}
		used_capacity_ = new_index;
	}

	constexpr void insert(const auto& pos, const auto& r_begin, const auto& r_end) noexcept
	{
		used_capacity_ += std::distance(r_begin, r_end);
		std::copy(r_begin, r_end, pos);
	}

	constexpr void clear() noexcept
	{
		used_capacity_ = 0;
	}

	constexpr bool empty() const noexcept { return used_capacity_ == 0; }

	constexpr auto begin() const noexcept { return data_.begin(); }
	constexpr auto end() const noexcept { return data_.begin()+used_capacity_; }

	constexpr auto begin() noexcept { return data_.begin(); }
	constexpr auto end() noexcept { return data_.begin()+used_capacity_; }

	constexpr auto cbegin() const noexcept { return data_.cbegin(); }
	constexpr auto cend() const noexcept { return data_.cbegin()+used_capacity_; }

	private:

	std::size_t used_capacity_{0};
	std::array<T, size_v> data_;
};

#endif // Fixed_capacity_vector_h_INCLUDED
