#ifndef Time_manager_h_INCLUDED
#define Time_manager_h_INCLUDED

#include <chrono>

class Time_manager
{
	public:

	explicit Time_manager(const std::optional<std::chrono::milliseconds>& time_on_clock
						, const std::optional<std::chrono::milliseconds>& movetime
						, const std::chrono::milliseconds& increment
						, const std::chrono::milliseconds& move_overhead
						, const int movestogo
						, const int ply) noexcept;

	[[nodiscard]] inline std::chrono::milliseconds optimum() const noexcept { return optimum_time; }
	[[nodiscard]] inline std::chrono::milliseconds maximum() const noexcept { return maximum_time; }
	[[nodiscard]] inline std::chrono::milliseconds used_time() const noexcept { return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-start_time); }

	private:

	std::chrono::time_point<std::chrono::steady_clock> start_time;
	std::chrono::milliseconds optimum_time, maximum_time;
};

#endif // Time_manager_h_INCLUDED
