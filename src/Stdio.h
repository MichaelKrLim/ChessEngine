#ifndef Thread_safe_io_h_INCLUDED
#define Thread_safe_io_h_INCLUDED

#include <iostream>
#include <mutex>
#include <string>

class Stdio
{
	public:

	[[nodiscard]] std::string input() const noexcept
	{
		std::lock_guard<std::mutex> lock{input_mtx};

		static std::string str;
		std::getline(std::cin,str);
		return str;
	}
	
	template <typename... T>
	void output(T&&... values) const noexcept
	{
		std::lock_guard<std::mutex> lock{output_mtx};

		((std::cout<<std::forward<T>(values)),...);
		std::cout<<std::endl;
	}
	
	private:

	inline static std::mutex input_mtx, output_mtx;
};

#endif // Thread_safe_io_h_INCLUDED
