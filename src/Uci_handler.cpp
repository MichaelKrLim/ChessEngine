#include "Board.h"
#include "Engine.h"
#include "Uci_handler.h"

#include <array>
#include <functional>
#include <string>
#include <unordered_map>
#include <variant>

namespace uci
{
    
}

using handler_function_map_t = std::unordered_map<std::string, std::function<void()>>;

handler_function_map_t to_handler_function = []()
{
    handler_function_map_t handler_functions;
    struct Handler_command_pair
    {
        std::string command;
        const void(&handler_function_pointer)() noexcept;
    };
    std::array<Handler_command_pair, 5> handler_command_pairs
    {{
        {"go", go_handler}, {"uci", uci_handler},
        {"isready", isready_handler}, {"ucinewgame", ucinewgame_handler},
        {"position", position_handler}
    }};
    for(const Handler_command_pair& handler_command_pair : handler_command_pairs)
    {
        handler_functions[handler_command_pair.command] = handler_command_pair.handler_function_pointer;
    }
    return handler_functions;
}();

void start_listening() noexcept
{
    for(std::string line{}; line != "quit"; std::getline(std::cin, line))
    {
        const auto trim_whitespace = [](std::string& to_trim)
        {
            const auto ws_front = std::find_if_not(to_trim.begin(), to_trim.end(), ::isspace);
            const auto ws_back = std::find_if_not(to_trim.rbegin(), to_trim.rend(), ::isspace).base();
            to_trim = (ws_front < ws_back ? std::string{ws_front, ws_back} : std::string{""});
        };
        trim_whitespace(line);
        if(to_handler_function.contains(line))
        {
            (to_handler_function[line])();
        }
        else //ungeneral for now, could use line.substr(0, line.find(' ')) for command
            position_handler(line);
    }
}

void isready_handler() noexcept
{
    std::cout << "readyok\n";
}

void ucinewgame_handler() noexcept
{
    engine_.board = engine::Board{};
}

void position_handler() noexcept
{

}

void go_handler() noexcept
{

}

void uci_handler() noexcept
{
    std::cout << "id: " << engine::engine_name << "\n" 
              << "author: " << engine::author << "\n"
              << "uciok\n";
}

