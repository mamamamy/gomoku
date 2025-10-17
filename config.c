#include "config.h"
#include "cjson.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *read_config_file(const char *filename) {
  FILE* f = fopen(filename, "rb");
  if (f == NULL) {
    fprintf(stderr, "Failed to open the file: %s\n", filename);
    exit(EXIT_FAILURE);
  }
  fseek(f, 0, SEEK_END);
  long file_size = ftell(f);
  if (file_size < 0) {
    fprintf(stderr, "Failed to get file size\n");
    exit(EXIT_FAILURE);
  }
  rewind(f);
  char* buf = malloc(file_size + 1);
  size_t rn = fread(buf, 1, file_size, f);
  if (rn != (size_t)file_size) {
    fprintf(stderr, "Failed to read file\n");
    exit(EXIT_FAILURE);
  }
  buf[file_size] = '\0';
  fclose(f);
  return buf;
}

void load_config(config *cfg, const char *filename) {
  cfg->thread_count = 0;
  cfg->initial_depth = 0;
  cfg->load_form_file_id = 0;
  char *json_text = read_config_file(filename);
  cj_value *obj = cj_parse(json_text, NULL);
  free(json_text);
  cj_value *member = obj->value.members;
  while (member != NULL) {
    char *name = member->name->data;
    if (!strcmp(name, "thread_count")) {
      cfg->thread_count = member->value.number;
    } else if (!strcmp(name, "initial_depth")) {
      cfg->initial_depth = member->value.number;
    } else if (!strcmp(name, "load_form_file_id")) {
      cfg->load_form_file_id = member->value.number;
    }
    member = member->next;
  }
  cj_clean(obj);
}
