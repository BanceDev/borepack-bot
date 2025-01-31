#include "commands.h"
#include <cpr/cpr.h>
#include <dpp/appcommand.h>
#include <dpp/nlohmann/json_fwd.hpp>
#include <iostream>
#include <sstream>

void start_pickup_command(const dpp::slashcommand_t &event) {
    if (event.get_parameter("gamemode").valueless_by_exception()) {
        event.reply("Please provide a game mode.");
        return;
    }

    std::string gamemode = std::get<std::string>(event.get_parameter("gamemode"));
    if (gamemode != "1on1" && gamemode != "2on2" &&
        gamemode != "4on4" && gamemode != "ffa" && gamemode != "wipeout") {
        event.reply("Invalid game mode selected.");
        return;
    }

    std::string map = "aerowalk";
    if (gamemode == "1on1" || gamemode == "2on2") {
        std::vector<std::string> options = {"dm2", "dm4", "dm6", "aerowalk", "ztndm3", "skull", "bravado"};
        map = options[rand() % options.size()];
    } else if (gamemode == "4on4") {
        std::vector<std::string> options = {"dm2", "dm3", "e1m2", "schloss", "phantombase", "aztek", "qobblestone"};
        map = options[rand() % options.size()];
    } else if (gamemode == "wipeout") {
        std::vector<std::string> options = {"dm2", "dm3", "rwild", "qwdm6qw", "qobblestone"};
        map = options[rand() % options.size()];
    }

    // Fetch the guild
    dpp::snowflake role_id;
    dpp::guild* guild = dpp::find_guild(event.command.guild_id);
    
    if (guild) {
        for (const auto& role_snowflake : guild->roles) {
            dpp::role* role = dpp::find_role(role_snowflake);
            if (role && role->name == gamemode) {
                role_id = role->id;
                break;
            }
        }
    }

    // Check if role was found
    std::string mention_text = role_id ? "<@&" + std::to_string(role_id) + ">" : "**(Role not found)**";

    // Create embed
    dpp::embed pickup_embed = dpp::embed()
        .set_color(0x6b5747)
        .set_title(gamemode + " pickup starting")
        .set_thumbnail("https://a.quake.world/mapshots/" + map + ".jpg")
        .add_field("Map", map, true)
        .add_field("Links", "[QTV](http://phobos.baseq.fr:9999/qtv?url=1@172.236.100.99:28000)|[WebQTV](https://hub.quakeworld.nu/qtv/?address=172.236.100.99:28501)|[Join](http://phobos.baseq.fr:9999/join?url=172.236.100.99:28501)", true);

    // Send message with mention
    dpp::message msg;
    msg.set_content(mention_text);
    msg.add_embed(pickup_embed);

    msg.allowed_mentions.parse_roles = true;

    event.reply(dpp::interaction_response_type::ir_channel_message_with_source, msg);
}

static cpr::Response r;

void poll_servers_command(const dpp::slashcommand_t &event) {
    r = cpr::Get(cpr::Url{"https://hubapi.quakeworld.nu/v2/servers/mvdsv"});
    if (r.status_code == 200) {
        try {
            nlohmann::json data = nlohmann::json::parse(r.text);
            if (data.empty()) {
                event.reply("No servers found.");
                return;
            }

            // Create the first server embed
            size_t page = 0;
            auto &server = data[page];

            dpp::embed server_embed = dpp::embed()
                .set_color(0x6b5747)
                .set_title(server["mode"].get<std::string>() + " on " + server["settings"]["map"].get<std::string>())
                .set_thumbnail("https://a.quake.world/mapshots/" + server["settings"]["map"].get<std::string>() + ".jpg")
                .add_field("Status", server["status"]["description"].get<std::string>());

            std::ostringstream players;
            for (const auto &player : server["players"]) {
                players << player["name"].get<std::string>() << "\n";
            }
            server_embed.add_field("Players", players.str());

            // Create buttons
            dpp::component row;
            row.add_component(dpp::component()
                .set_label("⬅️")
                .set_type(dpp::cot_button)
                .set_style(dpp::cos_primary)
                .set_id("prev_" + std::to_string(page)));

            row.add_component(dpp::component()
                .set_label("➡️")
                .set_type(dpp::cot_button)
                .set_style(dpp::cos_primary)
                .set_id("next_" + std::to_string(page)));

            dpp::message msg;
            msg.set_content("`" + server["address"].get<std::string>() + "`");
            msg.add_embed(server_embed);
            msg.add_component(row);

            event.reply(dpp::interaction_response_type::ir_channel_message_with_source, msg);
        } catch (const std::exception &e) {
            event.reply("Error parsing response.");
        }
    } else {
        event.reply("Error querying master server.");
    }
}

void handle_server_navigation(const dpp::button_click_t &event) {
    std::string id = event.custom_id;
    size_t page = std::stoi(id.substr(5));

    if (r.status_code == 200) {
        try {
            nlohmann::json data = nlohmann::json::parse(r.text);
            if (data.empty()) {
                event.reply("No servers found.");
                return;
            }

            if (id.rfind("prev_", 0) == 0 && page > 0) {
                page--;
            } else if (id.rfind("next_", 0) == 0 && page < data.size() - 1) {
                page++;
            }

            auto &server = data[page];

            dpp::embed server_embed = dpp::embed()
                .set_color(0x6b5747)
                .set_title(server["mode"].get<std::string>() + " on " + server["settings"]["map"].get<std::string>())
                .set_thumbnail("https://a.quake.world/mapshots/" + server["settings"]["map"].get<std::string>() + ".jpg")
                .add_field("Status", server["status"]["description"].get<std::string>());

            std::ostringstream players;
            for (const auto &player : server["players"]) {
                players << player["name"].get<std::string>() << "\n";
            }
            server_embed.add_field("Players", players.str());

            // Update buttons
            dpp::component row;
            row.add_component(dpp::component()
                .set_label("⬅️")
                .set_type(dpp::cot_button)
                .set_style(dpp::cos_primary)
                .set_id("prev_" + std::to_string(page)));

            row.add_component(dpp::component()
                .set_label("➡️")
                .set_type(dpp::cot_button)
                .set_style(dpp::cos_primary)
                .set_id("next_" + std::to_string(page)));

            dpp::message msg;
            msg.set_content("`" + server["address"].get<std::string>() + "`");
            msg.add_embed(server_embed);
            msg.add_component(row);

            event.edit_response(msg);
        } catch (const std::exception &e) {
            event.reply("Error parsing response.");
        }
    } else {
        event.reply("Error querying master server.");
    }
}
