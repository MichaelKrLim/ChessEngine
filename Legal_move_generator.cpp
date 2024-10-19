#include "Constants.h"
#include "Legal_move_generator.h"
#include "Position.h"
#include "Square.h"

using namespace engine;
//will fix constexpr
uint64_t Legal_move_generator::reachable_squares(const& Square square, const& Piece piece, const& Board_state board_state)
{
    switch(piece)
    {
        Piece::pawn:
        {

        }
        Piece::knight:
        {

        }
        Piece::bishop
        {

        }
        Piece::rook
        {

        }
        Piece::queen
        {

        }
        Piece::king
        {

        }
    }
}

uint64_t Legal_move_generator::knight_reachable_squares(const Square& square, const uint64_t& occupied_squares)
{
    Position position{square};
    uint64_t reachable_squares{0};
    for(const auto& move : knight_moves)
    {
        const auto [del_rank, del_file] = move;
    }
}

uint64_t Legal_move_generator::rook_reachable_squares(const Square& square, const uint64_t& occupied_squares)
{

}

void Legal_move_generator::initialise_attack_table()
{
    
}

void Legal_move_generator::cast_magic()
{
    
}