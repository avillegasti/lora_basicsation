#ifndef CONFIG_H
#define CONFIG_H

#define MAX_ALLOWED 100

extern char* allowed_devaddrs[MAX_ALLOWED];
extern int allowed_count;

int load_allowed_devaddr(const char* filename);
void print_allowed_devaddrs(void);

#endif
