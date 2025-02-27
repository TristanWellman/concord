#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h> /* PRIu64 */
#include <assert.h>

#include "discord.h"

void
print_usage(void)
{
    printf(
        "\n\nThis bot demonstrates how easy it is to fetch/delete invites\n"
        "1. Type 'invite.get <invite_code>' to get a invite object from its "
        "particular code\n"
        "2. Type 'invite.delete <invite_code>' to delete a invite object by "
        "its particular code\n"
        "\nTYPE ANY KEY TO START BOT\n");
}

void
on_ready(struct discord *client)
{
    const struct discord_user *bot = discord_get_self(client);

    log_info("Invite-Bot succesfully connected to Discord as %s#%s!",
             bot->username, bot->discriminator);
}

void
done(struct discord *client, void *data, const struct discord_invite *invite)
{
    u64_snowflake_t *channel_id = data;
    char text[256];

    snprintf(text, sizeof(text), "Success: https://discord.gg/%s",
             invite->code);

    struct discord_create_message params = { .content = text };
    discord_create_message(client, *channel_id, &params, NULL);
}

void
fail(struct discord *client, CCORDcode code, void *data)
{
    u64_snowflake_t *channel_id = data;

    struct discord_create_message params = {
        .content = "Couldn't perform operation."
    };
    discord_create_message(client, *channel_id, &params, NULL);
}

void
on_invite_get(struct discord *client, const struct discord_message *msg)
{
    if (msg->author->bot) return;

    u64_snowflake_t *channel_id = malloc(sizeof(u64_snowflake_t));
    *channel_id = msg->channel_id;

    struct discord_ret_invite ret = {
        .done = &done,
        .fail = &fail,
        .data = channel_id,
        .cleanup = &free,
    };

    struct discord_get_invite params = {
        .with_counts = true,
        .with_expiration = true,
    };
    discord_get_invite(client, msg->content, &params, &ret);
}

void
on_invite_delete(struct discord *client, const struct discord_message *msg)
{
    if (msg->author->bot) return;

    u64_snowflake_t *channel_id = malloc(sizeof(u64_snowflake_t));
    *channel_id = msg->channel_id;

    struct discord_ret_invite ret = {
        .done = &done,
        .fail = &fail,
        .data = channel_id,
        .cleanup = &free,
    };
    discord_delete_invite(client, msg->content, &ret);
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
    assert(NULL != client && "Could not initialize client");

    discord_set_on_ready(client, &on_ready);

    discord_set_prefix(client, "invite.");
    discord_set_on_command(client, "get", &on_invite_get);
    discord_set_on_command(client, "delete", &on_invite_delete);

    print_usage();
    fgetc(stdin); // wait for input

    discord_run(client);

    discord_cleanup(client);
    ccord_global_cleanup();
}
