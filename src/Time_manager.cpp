#include "Time_manager.h"

#include <chrono>

namespace chrono=std::chrono;

Time_manager::Time_manager(const std::optional<chrono::milliseconds>& time_on_clock
						 , const std::optional<chrono::milliseconds>& movetime
						 , const chrono::milliseconds& increment
						 , const chrono::milliseconds& move_overhead
						 , const int movestogo
						 , const int ply) noexcept
	: start_time(std::chrono::steady_clock::now())
{
	if(!time_on_clock.has_value())
	{
		if(movetime.has_value())
			maximum_time=optimum_time=*movetime;
		return;
	}
	int moves_to_go{movestogo==0? 30:movestogo};
	chrono::milliseconds projected_time_left{std::max(chrono::milliseconds{1}, *time_on_clock+(increment-move_overhead)*moves_to_go)};
	constexpr static chrono::milliseconds ms_to_reach_depth_one{4};
	maximum_time=projected_time_left-(moves_to_go*ms_to_reach_depth_one);
	optimum_time=projected_time_left/moves_to_go;
}
