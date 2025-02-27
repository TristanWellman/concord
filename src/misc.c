#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "discord.h"
#include "discord-internal.h"
#include "cog-utils.h"

void
discord_embed_set_footer(struct discord_embed *embed,
                         char text[],
                         char icon_url[],
                         char proxy_icon_url[])
{
    if (IS_EMPTY_STRING(text)) {
        log_error("Missing 'text'");
        return;
    }

    if (embed->footer)
        discord_embed_footer_cleanup(embed->footer);
    else
        embed->footer = malloc(sizeof *embed->footer);
    discord_embed_footer_init(embed->footer);

    if (text) cog_strndup(text, strlen(text), &embed->footer->text);
    if (icon_url)
        cog_strndup(icon_url, strlen(icon_url), &embed->footer->icon_url);
    if (proxy_icon_url)
        cog_strndup(proxy_icon_url, strlen(proxy_icon_url),
                    &embed->footer->proxy_icon_url);
}

void
discord_embed_set_title(struct discord_embed *embed, char format[], ...)
{
    char buf[2048];
    size_t len;
    va_list args;

    va_start(args, format);

    len = vsnprintf(buf, sizeof(buf), format, args);
    ASSERT_S(len < sizeof(buf), "Out of bounds write attempt");

    if (embed->title) free(embed->title);
    cog_strndup(buf, len, &embed->title);

    va_end(args);
}

void
discord_embed_set_description(struct discord_embed *embed, char format[], ...)
{
    char buf[2048];
    size_t len;
    va_list args;

    va_start(args, format);

    len = vsnprintf(buf, sizeof(buf), format, args);
    ASSERT_S(len < sizeof(buf), "Out of bounds write attempt");

    if (embed->description) free(embed->description);
    cog_strndup(buf, len, &embed->description);

    va_end(args);
}

void
discord_embed_set_url(struct discord_embed *embed, char format[], ...)
{
    char buf[2048];
    size_t len;
    va_list args;

    va_start(args, format);

    len = vsnprintf(buf, sizeof(buf), format, args);
    ASSERT_S(len < sizeof(buf), "Out of bounds write attempt");

    if (embed->url) free(embed->url);
    cog_strndup(buf, len, &embed->url);

    va_end(args);
}

void
discord_embed_set_thumbnail(struct discord_embed *embed,
                            char url[],
                            char proxy_url[],
                            int height,
                            int width)
{
    if (embed->thumbnail)
        discord_embed_thumbnail_cleanup(embed->thumbnail);
    else
        embed->thumbnail = malloc(sizeof *embed->thumbnail);
    discord_embed_thumbnail_init(embed->thumbnail);

    if (url) cog_strndup(url, strlen(url), &embed->thumbnail->url);
    if (proxy_url)
        cog_strndup(proxy_url, strlen(proxy_url),
                    &embed->thumbnail->proxy_url);
    if (height) embed->thumbnail->height = height;
    if (width) embed->thumbnail->width = width;
}

void
discord_embed_set_image(struct discord_embed *embed,
                        char url[],
                        char proxy_url[],
                        int height,
                        int width)
{
    if (embed->image)
        discord_embed_image_cleanup(embed->image);
    else
        embed->image = malloc(sizeof *embed->image);
    discord_embed_image_init(embed->image);

    if (url) cog_strndup(url, strlen(url), &embed->image->url);
    if (proxy_url)
        cog_strndup(proxy_url, strlen(proxy_url), &embed->image->proxy_url);
    if (height) embed->image->height = height;
    if (width) embed->image->width = width;
}

void
discord_embed_set_video(struct discord_embed *embed,
                        char url[],
                        char proxy_url[],
                        int height,
                        int width)
{
    if (embed->video)
        discord_embed_video_cleanup(embed->video);
    else
        embed->video = malloc(sizeof *embed->video);
    discord_embed_video_init(embed->video);

    if (url) cog_strndup(url, strlen(url), &embed->video->url);
    if (proxy_url)
        cog_strndup(proxy_url, strlen(proxy_url), &embed->video->proxy_url);
    if (height) embed->video->height = height;
    if (width) embed->video->width = width;
}

void
discord_embed_set_provider(struct discord_embed *embed,
                           char name[],
                           char url[])
{
    if (embed->provider)
        discord_embed_provider_cleanup(embed->provider);
    else
        embed->provider = malloc(sizeof *embed->provider);
    discord_embed_provider_init(embed->provider);

    if (name) cog_strndup(name, strlen(name), &embed->provider->name);
    if (url) cog_strndup(url, strlen(url), &embed->provider->url);
}

void
discord_embed_set_author(struct discord_embed *embed,
                         char name[],
                         char url[],
                         char icon_url[],
                         char proxy_icon_url[])
{
    if (embed->author)
        discord_embed_author_cleanup(embed->author);
    else
        embed->author = malloc(sizeof *embed->author);
    discord_embed_author_init(embed->author);

    if (name) cog_strndup(name, strlen(name), &embed->author->name);
    if (url) cog_strndup(url, strlen(url), &embed->author->url);
    if (icon_url)
        cog_strndup(icon_url, strlen(icon_url), &embed->author->icon_url);
    if (proxy_icon_url)
        cog_strndup(proxy_icon_url, strlen(proxy_icon_url),
                    &embed->author->proxy_icon_url);
}

void
discord_embed_add_field(struct discord_embed *embed,
                        char name[],
                        char value[],
                        bool Inline)
{
    struct discord_embed_field field = { 0 };

    field.Inline = Inline;

    if (name) cog_strndup(name, strlen(name), &field.name);
    if (value) cog_strndup(value, strlen(value), &field.value);

    ntl_append2((ntl_t *)&embed->fields, sizeof(struct discord_embed_field),
                &field);
}

void
discord_overwrite_append(struct discord_overwrite ***permission_overwrites,
                         u64_snowflake_t id,
                         int type,
                         u64_bitmask_t allow,
                         u64_bitmask_t deny)
{
    struct discord_overwrite new_overwrite = { 0 };

    new_overwrite.id = id;
    new_overwrite.type = type;
    new_overwrite.allow = allow;
    new_overwrite.deny = deny;

    ntl_append2((ntl_t *)permission_overwrites,
                sizeof(struct discord_overwrite), &new_overwrite);
}

void
discord_presence_add_activity(struct discord_presence_status *presence,
                              struct discord_activity *activity)
{
    ntl_append2((ntl_t *)&presence->activities,
                sizeof(struct discord_activity), activity);
}
