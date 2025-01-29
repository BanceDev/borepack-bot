#include <unordered_map>
#include <cstdlib>
#include <string>
#include "commands.h"

std::unordered_map<uint64_t, std::string> user_data; // Maps user IDs to their stored info

void handle_set_command(const dpp::slashcommand_t& event) {
    if (event.get_parameter("nickname").valueless_by_exception()) {
        event.reply("Please provide some information to store.");
        return;
    }

    std::string nick = std::get<std::string>(event.get_parameter("nickname"));
    user_data[event.command.usr.id] = nick;

    event.reply("Your nickname has been set!");
}

