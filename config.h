#ifndef CONFIG_H
#define CONFIG_H

typedef struct config config;

struct config {
  int thread_count;
  int initial_depth;
  int load_form_file_id;
};

void load_config(config *cfg, const char *filename);

#endif
