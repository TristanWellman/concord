#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <limits.h>
#include <errno.h>
#include <inttypes.h> /* SCNu64 */

#include "discord.h"

u64_snowflake_t g_sudo_id;

void
print_usage(void)
{
    printf("\n\nThis bot allows navigating its host machine like"
           " a shell terminal.\n\n"
           "DISCLAIMER: This bot is potentially dangerous if not"
           " used with care.\nOnly give admin privileges to yourself"
           " or someone trustworthy.\n\n\n");
}

void
on_ready(struct discord *client)
{
    const struct discord_user *bot = discord_get_self(client);

    log_info("Shell-Bot succesfully connected to Discord as %s#%s!",
             bot->username, bot->discriminator);
}

void
on_cd(struct discord *client, const struct discord_message *msg)
{
    if (msg->author->id != g_sudo_id) return;

    char path[PATH_MAX];

    chdir(*msg->content ? msg->content : ".");

    struct discord_create_message params = {
        .content = getcwd(path, sizeof(path)),
    };

    discord_create_message(client, msg->channel_id, &params, NULL);
}

void
on_less_like(struct discord *client, const struct discord_message *msg)
{
    if (msg->author->id != g_sudo_id) return;

    if (!msg->content || !*msg->content) {
        struct discord_create_message params = { .content =
                                                     "No file specified" };
        discord_create_message(client, msg->channel_id, &params, NULL);
    }
    else {
        struct discord_embed embed = { .title = msg->content };
        struct discord_attachment attachment = { .filename = msg->content };
        char text[512];

        snprintf(text, sizeof(text), "attachment://%s", msg->content);

        struct discord_create_message params = {
            .content = text,
            .embeds = (struct discord_embed *[]){ &embed, NULL },
            .attachments =
                (struct discord_attachment *[]){ &attachment, NULL },
        };

        discord_create_message(client, msg->channel_id, &params, NULL);
    }
}

void
on_fallback(struct discord *client, const struct discord_message *msg)
{
    const size_t MAX_FSIZE = 5e6; // 5 mb
    const size_t MAX_CHARS = 2000;
    FILE *fp;

    if (msg->author->id != g_sudo_id) return;

    if (NULL == (fp = popen(msg->content, "r"))) {
        perror("Failed to run command");
        return;
    }

    char *path = calloc(1, MAX_FSIZE);
    char *pathtmp = calloc(1, MAX_FSIZE);
    while (NULL != fgets(path, MAX_FSIZE, fp)) {
        strncat(pathtmp, path, MAX_FSIZE - 1);
    }

    const size_t fsize = strlen(pathtmp);

    if (fsize <= MAX_CHARS) {
        struct discord_create_message params = { .content = pathtmp };
        discord_create_message(client, msg->channel_id, &params, NULL);
    }
    else {
        struct discord_attachment attachment = {
            .content = pathtmp,
            .size = fsize,
        };

        struct discord_create_message params = {
            .attachments = (struct discord_attachment *[]){ &attachment, NULL }
        };
        discord_create_message(client, msg->channel_id, &params, NULL);
    }

    pclose(fp);
    free(path);
    free(pathtmp);
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

    discord_set_prefix(client, "$");
    discord_set_on_command(client, NULL, &on_fallback);
    discord_set_on_command(client, "cd", &on_cd);
    discord_set_on_commands(client, &on_less_like, "less", "cat", "hexdump",
                            NULL);

    print_usage();
    do {
        printf("User ID to have sudo privileges\n");
        fscanf(stdin, "%" SCNu64, &g_sudo_id);
    } while (!g_sudo_id || errno == ERANGE);

    discord_run(client);

    discord_cleanup(client);
    ccord_global_cleanup();
}
