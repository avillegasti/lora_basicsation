#include "tomlc17.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

char* allowed_devaddrs[MAX_ALLOWED];
int allowed_count = 0;

int load_allowed_devaddr(const char* filename) {
    toml_result_t conf = toml_parse_file_ex(filename);
    if (!conf.ok) {
        fprintf(stderr, "Error parsing %s: %s\n", filename, conf.errmsg);
        return -1;
    }

    toml_datum_t d_allowed = toml_seek(conf.toptab, "concentrator-sadf.allowed");
    if (d_allowed.type == TOML_ARRAY) {
        int n = d_allowed.u.arr.size;
        for (int i = 0; i < n && allowed_count < MAX_ALLOWED; i++) {
            toml_datum_t elem = d_allowed.u.arr.elem[i];
            if (elem.type == TOML_STRING) {
                allowed_devaddrs[allowed_count] = strdup(elem.u.s);
                allowed_count++;
            }
        }
    } else {
        fprintf(stderr, "[ERROR] No se encontró concentrator-sadf.allowed\n");
        toml_free(conf);
        return -1;
    }

    toml_free(conf);
    return 0;
}

int is_devaddr_allowed(const char* devaddr) {
    for (int i = 0; i < allowed_count; i++) {
        if (strcmp(devaddr, allowed_devaddrs[i]) == 0) {
            return 1; // permitido
        }
    }
    return 0; // no permitido
}
