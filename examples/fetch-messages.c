#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "discord.h"

void
print_usage(void)
{
    printf("\n\nThis bot demonstrates how easy it is to fetch"
           " messages from a particular user (without even connecting"
           " to Discord Gateway).\n"
           "\nTYPE ANY KEY TO START BOT\n");
}

u64_snowflake_t
select_guild(struct discord *client)
{
    struct discord_guild **guilds = NULL;
    struct discord_ret_guilds ret = { .sync = &guilds };
    CCORDcode code;

    code = discord_get_current_user_guilds(client, &ret);
    assert(CCORD_OK == code && guilds != NULL && "Couldn't fetch guilds");

    printf(
        "\n\nSelect the guild that the user you wish to fetch messages from "
        "is part of");

    int i = 0;
    while (guilds[i]) {
        printf("\n%d. %s", i + 1, guilds[i]->name);
        ++i;
    }

    while (1) {
        char strnum[10];

        puts("\n\nNUMBER >>");
        fgets(strnum, sizeof(strnum), stdin);

        int num = strtol(strnum, NULL, 10);
        if (num > 0 && num <= i) {
            u64_snowflake_t guild_id = guilds[num - 1]->id;

            discord_guild_list_free(guilds);

            return guild_id;
        }

        printf("\nPlease, insert a value between 1 and %d", i);
    }
}

u64_snowflake_t
select_member(struct discord *client, u64_snowflake_t guild_id)
{
    // get guilds bot is a part of
    struct discord_guild_member **members = NULL;
    struct discord_ret_guild_members ret = { .sync = &members };
    struct discord_list_guild_members params = { .limit = 1000, .after = 0 };
    CCORDcode code;

    code = discord_list_guild_members(client, guild_id, &params, &ret);
    assert(CCORD_OK == code && members != NULL
           && "Guild is empty or bot needs to activate its privileged "
              "intents.\n\t"
              "See this guide to activate it: "
              "https://discordpy.readthedocs.io/en/latest/"
              "intents.html#privileged-intents");

    printf("\n\nSelect the member that will have its messages fetched");
    int i = 0;
    while (members[i]) {
        printf("\n%d. %s", i + 1, members[i]->user->username);

        if (members[i]->nick && *members[i]->nick)
        { // prints nick if available
            printf(" (%s)", members[i]->nick);
        }
        ++i;
    }

    do {
        char strnum[10]; // 10 digits should be more than enough..

        puts("\n\nNUMBER >>");
        fgets(strnum, sizeof(strnum), stdin);

        int num = strtol(strnum, NULL, 10);
        if (num > 0 && num <= i) {
            u64_snowflake_t user_id = members[num - 1]->user->id;

            discord_guild_member_list_free(members);

            return user_id;
        }

        printf("\nPlease, insert a value between 1 and %d", i);
    } while (1);
}

void
fetch_member_msgs(struct discord *client,
                  u64_snowflake_t guild_id,
                  u64_snowflake_t user_id)
{
    struct discord_channel **channels = NULL;
    CCORDcode code;

    struct discord_ret_channels ret = { .sync = &channels };
    code = discord_get_guild_channels(client, guild_id, &ret);
    assert(CCORD_OK == code && "Couldn't fetch channels from guild");

    struct discord_get_channel_messages params = { .limit = 100 };
    for (int i = 0; channels[i]; ++i) {
        params.before = 0;

        int n_msg = 0;
        struct discord_message **msgs = NULL;
        struct discord_ret_messages ret = { .sync = &msgs };
        while (n_msg != params.limit) {
            discord_get_channel_messages(client, channels[i]->id, &params,
                                         &ret);
            if (!msgs) break;

            for (n_msg = 0; msgs[n_msg]; ++n_msg) {
                if (user_id == msgs[n_msg]->author->id
                    && *msgs[n_msg]->content) {
                    printf("%s\n", msgs[n_msg]->content);
                }
            }

            if (n_msg) params.before = msgs[n_msg - 1]->id;

            discord_message_list_free(msgs);
        }
    }

    discord_channel_list_free(channels);
}

int
main(int argc, char *argv[])
{
    const char *config_file;
    if (argc > 1)
        config_file = argv[1];
    else
        config_file = "../config.json";

    ccord_global_init();
    struct discord *client = discord_config_init(config_file);
    assert(NULL != client && "Couldn't initialize client");

    print_usage();
    fgetc(stdin); // wait for input

    u64_snowflake_t guild_id = select_guild(client);
    u64_snowflake_t user_id = select_member(client, guild_id);

    fetch_member_msgs(client, guild_id, user_id);

    discord_cleanup(client);
    ccord_global_cleanup();
}
