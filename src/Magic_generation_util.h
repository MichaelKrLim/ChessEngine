#ifndef Magic_generation_util_h
#define Magic_generation_util_h

#include <array>
#include <cstdint>
#include <random>
#include <vector>

#include "Bitboard.h"
#include "Constants.h"
#include "Magic_util.h"
#include "Pieces.h"
#include "Position.h"

namespace engine
{
	namespace magic
	{
		constexpr static std::array<std::uint8_t, board_size*board_size> rook_relevant_bits = 
		{
			12, 11, 11, 11, 11, 11, 11, 12,
			11, 10, 10, 10, 10, 10, 10, 11,
			11, 10, 10, 10, 10, 10, 10, 11,
			11, 10, 10, 10, 10, 10, 10, 11,
			11, 10, 10, 10, 10, 10, 10, 11,
			11, 10, 10, 10, 10, 10, 10, 11,
			11, 10, 10, 10, 10, 10, 10, 11,
			12, 11, 11, 11, 11, 11, 11, 12
		};

		constexpr static std::array<std::uint8_t, board_size*board_size> bishop_relevant_bits = 
		{
			6, 5, 5, 5, 5, 5, 5, 6,
			5, 5, 5, 5, 5, 5, 5, 5,
			5, 5, 7, 7, 7, 7, 5, 5,
			5, 5, 7, 9, 9, 7, 5, 5,
			5, 5, 7, 9, 9, 7, 5, 5,
			5, 5, 7, 7, 7, 7, 5, 5,
			5, 5, 5, 5, 5, 5, 5, 5,
			6, 5, 5, 5, 5, 5, 5, 6
		};

		constexpr std::uint64_t rook_mask(const Position& square)
		{
			std::uint64_t result{0ULL};
			for(int r = square.rank_+1; r <= 6; r++) result |= (1ULL << (square.file_ + r*8));
			for(int r = square.rank_-1; r >= 1; r--) result |= (1ULL << (square.file_ + r*8));
			for(int f = square.file_+1; f <= 6; f++) result |= (1ULL << (f + square.rank_*8));
			for(int f = square.file_-1; f >= 1; f--) result |= (1ULL << (f + square.rank_*8)); 
			return result;
		}

		constexpr std::uint64_t bishop_mask(const Position& square)
		{
			std::uint64_t result{0ULL};
			for(int r = square.rank_+1, f = square.file_+1; r<=6 && f<=6; r++, f++) result |= (1ULL << (f + r*8));
			for(int r = square.rank_+1, f = square.file_-1; r<=6 && f>=1; r++, f--) result |= (1ULL << (f + r*8));
			for(int r = square.rank_-1, f = square.file_+1; r>=1 && f<=6; r--, f++) result |= (1ULL << (f + r*8));
			for(int r = square.rank_-1, f = square.file_-1; r>=1 && f>=1; r--, f--) result |= (1ULL << (f + r*8));
			return result;
		}

		constexpr std::vector<Bitboard> generate_blocker_configurations(const Position& square, const Piece& piece_type)
		{
			std::vector<Bitboard> blocker_configurations{};
			blocker_configurations.push_back(Bitboard{0ULL});
			bool is_bishop = piece_type == Piece::bishop;
			const std::uint64_t attack_mask = is_bishop? bishop_mask(square) : rook_mask(square);
			for(std::uint64_t blockers = attack_mask; blockers != 0; blockers = (blockers - 1) & attack_mask) 
			{
				blocker_configurations.push_back(Bitboard{blockers});
			}
			return blocker_configurations;
		}

		constexpr Bitboard rook_reachable_squares(const Position& rook_square, const Bitboard& occupied_squares)
		{
			Bitboard valid_moves{0ULL};
			for(const auto& [dr, df] : rook_moves_) 
			{
				const Position offset{dr, df};
				for(Position target_square{rook_square+offset}; is_on_board(target_square); target_square+=offset)
				{
					valid_moves.add_piece(target_square);
					if(!is_free(target_square, occupied_squares)) break;
				}
			}
			return valid_moves;
		}

		constexpr Bitboard bishop_reachable_squares(const Position& bishop_square, const Bitboard& occupied_squares)
		{
			Bitboard valid_moves{0ULL};
			for(const auto& [dr, df] : bishop_moves_) 
			{
				const Position offset{dr, df};
				for(Position target_square{bishop_square+offset}; is_on_board(target_square); target_square+=offset)
				{	
					valid_moves.add_piece(target_square);
					if(!is_free(target_square, occupied_squares)) break;
				}
			}
			return valid_moves;
		}

		const std::uint64_t random_uint64()
		{
			static std::random_device rd;
			static std::mt19937_64 gen(rd());
			return gen();
		}

		const std::uint64_t random_uint64_fewbits()
		{
			return random_uint64() & random_uint64() & random_uint64();
		}

		Magic_square find_magic(const Position& square, const Piece& piece_type)
		{
			try
			{
				const auto square_index = to_index(square);
				const bool is_bishop = piece_type == Piece::bishop;
				const auto relevant_bits = is_bishop? bishop_relevant_bits[square_index] : rook_relevant_bits[square_index];
				const auto shift = 64-relevant_bits;
				const std::uint64_t mask = is_bishop? bishop_mask(square) : rook_mask(square);
				const auto& reachable_squares = is_bishop? bishop_reachable_squares : rook_reachable_squares;
				const std::vector<Bitboard> blocker_configurations = generate_blocker_configurations(square, piece_type);
				const std::vector<Bitboard> attacks = [&reachable_squares, &blocker_configurations, &square]()
				{
					std::vector<Bitboard> attacks{};
					for(const auto& blocker_configuration : blocker_configurations)
					{
						attacks.push_back(reachable_squares(square, blocker_configuration));
					}
					return attacks;
				}();
				for(std::size_t k{0}; k < 100000000; ++k) 
				{
					std::vector<Bitboard> used(1 << relevant_bits, Bitboard{0xFFFFFFFFFFFFFFFFULL});
					const std::uint64_t magic = random_uint64_fewbits();
					if(std::popcount((mask * magic) & 0xFF00000000000000ULL) < 6) continue;
					bool fail{false};
					for(std::size_t i{0}; !fail && i < blocker_configurations.size(); ++i) 
					{
						const std::uint64_t magic_index = magic_hash(blocker_configurations[i], magic, shift);
						if(used[magic_index] == 0xFFFFFFFFFFFFFFFFULL) used[magic_index] = attacks[i];
						else if(used[magic_index] != attacks[i]) fail = true;
					}
					if(!fail) return Magic_square{used, mask, magic, std::uint8_t(64-relevant_bits)};
				}
				throw std::runtime_error("could not find magic in: find_magic(...)");
			}
			catch(std::runtime_error& r)
			{
				std::cout << "Move_generator failed initialisiation with: " << r.what();
				std::exit(EXIT_FAILURE);
			}
		}
	}
}

#endif // Magic_generation_util_h_INCLUDED