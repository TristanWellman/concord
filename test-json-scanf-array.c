#include "json-scanf.h"
#include <stdio.h>
#include <stdlib.h>
#include "jsmn.h"

static char * print_token(jsmntype_t t) {
  switch(t) {
    case JSMN_UNDEFINED: return "undefined";
    case JSMN_OBJECT: return "object";
    case JSMN_ARRAY: return "array";
    case JSMN_STRING: return "string";
    case JSMN_PRIMITIVE: return "primitive";
  }
}

char test_string [] =
"{\n"
  "|sha|: |9fb037999f264ba9a7fc6274d15fa3ae2ab98312|,\n"
  "|url|: |https://api.github.com/repos/octocat/Hello-World/trees/9fb037999f264ba9a7fc6274d15fa3ae2ab98312|,\n"
  "|tree|: [\n"
  "{"
  "  |path|: |file.rb|,\n"
  "  |mode|: |100644|,\n"
  "  |type|: |blob|,\n"
  "  |size|: 30,\n"
  "  |sha|: |44b4fc6d56897b048c772eb4087f854f46256132|,\n"
  "  |url|: |https://api.github.com/repos/octocat/Hello-World/git/blobs/44b4fc6d56897b048c772eb4087f854f46256132|\n"
  "},\n"
  "{\n"
  "  |path|: |subdir|,\n"
  "  |mode|: |040000|,\n"
  "  |type|: |tree|,\n"
  "  |sha|: |f484d249c660418515fb01c2b9662073663c242e|,\n"
  "  |url|: |https://api.github.com/repos/octocat/Hello-World/git/blobs/f484d249c660418515fb01c2b9662073663c242e|\n"
  "},\n"
  "{\n"
  "  |path|: |exec_file|,\n"
  "  |mode|: |100755|,\n"
  "  |type|: |blob|,\n"
  "  |size|: 75,\n"
  "  |sha|: |45b983be36b73c0788dc9cbcb76cbb80fc7bb057|,\n"
  "  |url|: |https://api.github.com/repos/octocat/Hello-World/git/blobs/45b983be36b73c0788dc9cbcb76cbb80fc7bb057|\n"
  "}\n"
  "],\n"
  "|truncated|: false\n"
"}";

int main () {
  char * json_str = NULL;
  int s = json_asprintf(&json_str, test_string);
  //printf("%s\n", json_str);
  struct json_token tok;
  json_scanf(json_str, s, "[tree]%T", &tok);
  printf ("%.*s\n", tok.length, tok.start);

  jsmn_parser parser;
  jsmn_init(&parser);
  jsmntok_t * t = NULL;
  int num_tok = jsmn_parse(&parser, tok.start, tok.length, NULL, 0);
  //printf ("%d\n", num_tok);

  t = malloc(sizeof(jsmntok_t) * num_tok);
  jsmn_init(&parser);
  num_tok = jsmn_parse(&parser, tok.start, tok.length, t, num_tok+1);

  int i;
  for (i = 0; i < num_tok; i++) {
    printf("[%d][size:%d]%s (%.*s)\n", i, t[i].size, print_token(t[i].type),
           t[i].end - t[i].start, tok.start + t[i].start);
  }
  return 0;
}

