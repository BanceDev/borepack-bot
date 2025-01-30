#include <dpp/dpp.h>
#include <cstdlib>
#include <dpp/presence.h>
#include <string>
#include "commands.h"

int main() {
    dpp::cluster bot(std::getenv("BOT_TOKEN"));

    bot.global_bulk_command_delete();


    bot.on_slashcommand([](auto event) {
        if (event.command.get_command_name() == "setnick") {
            handle_set_command(event);
        }
        if (event.command.get_command_name() == "pickup") {
            start_pickup_command(event);
        }
        if (event.command.get_command_name() == "servers") {
            poll_servers_command(event);
        }
    });

    bot.on_ready([&bot](auto event) {
    if (dpp::run_once<struct register_bot_commands>()) {
        // userinfo
        bot.global_command_create(dpp::slashcommand("setnick", "set ingame nickname", bot.me.id)
            .add_option(dpp::command_option(dpp::co_string, "nickname", "your nickname", true)));
        // pickups
        bot.global_command_create(dpp::slashcommand("pickup", "start a pickup game", bot.me.id)
            .add_option(
                dpp::command_option(dpp::co_string, "gamemode", "select a game mode", true)
                .add_choice(dpp::command_option_choice("1on1", "1on1"))
                .add_choice(dpp::command_option_choice("2on2", "2on2"))
                .add_choice(dpp::command_option_choice("4on4", "4on4"))
                .add_choice(dpp::command_option_choice("ffa", "ffa"))
                .add_choice(dpp::command_option_choice("wipeout", "wipeout")))
        );
        bot.global_command_create(dpp::slashcommand("servers", "get list of active servers", bot.me.id));
    }

    bot.set_presence(dpp::presence(dpp::ps_dnd, dpp::at_game, "QuakeWorld"));
    });

    bot.start(dpp::st_wait);
    return 0;
}
