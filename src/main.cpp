#include <dpp/dpp.h>
#include <cstdlib>
#include <dpp/presence.h>
#include <unordered_map>
#include <string>

std::unordered_map<uint64_t, std::string> user_data; // Maps user IDs to their stored info

void handle_set_command(const dpp::slashcommand_t& event) {
    if (event.get_parameter("info").valueless_by_exception()) {
        event.reply("Please provide some information to store.");
        return;
    }

    std::string info = std::get<std::string>(event.get_parameter("info"));
    user_data[event.command.usr.id] = info;

    event.reply("Your information has been stored!");
}

void handle_get_command(const dpp::slashcommand_t& event) {
    auto it = user_data.find(event.command.usr.id);
    if (it != user_data.end()) {
        event.reply("Your stored information: " + it->second);
    } else {
        event.reply("You have no stored information.");
    }
}

int main() {
	dpp::cluster bot(std::getenv("BOT_TOKEN"));

	bot.on_slashcommand([](auto event) {
        if (event.command.get_command_name() == "setinfo") {
            handle_set_command(event);
        } else if (event.command.get_command_name() == "getinfo") {
            handle_get_command(event);
        }
	});

	bot.on_ready([&bot](auto event) {
        if (dpp::run_once<struct register_bot_commands>()) {
            bot.global_command_create(dpp::slashcommand("setinfo", "Store some information", bot.me.id)
                .add_option(dpp::command_option(dpp::co_string, "info", "The info to store", true)));

            bot.global_command_create(dpp::slashcommand("getinfo", "Retrieve your stored information", bot.me.id));
        }

        bot.set_presence(dpp::presence(dpp::ps_dnd, dpp::at_game, "boring with quad"));
	});

	bot.start(dpp::st_wait);
	return 0;
}
