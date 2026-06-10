#include "params.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_PARAMS 64
#define MAX_KEY_LEN 32
#define MAX_VAL_LEN 64

typedef struct {
    char key[MAX_KEY_LEN];
    char val[MAX_VAL_LEN];
    bool used;
} ParamEntry;

static ParamEntry g_params[MAX_PARAMS];
static int g_count = 0;

static int find_param(const char* key) {
    for (int i = 0; i < g_count; i++)
        if (g_params[i].used && strcmp(g_params[i].key, key) == 0) return i;
    return -1;
}

void params_clear(void) {
    g_count = 0;
    memset(g_params, 0, sizeof(g_params));
}

void params_load(const char* filepath) {
    params_clear();
    FILE* f = fopen(filepath, "r");
    if (!f) return;

    char line[128];
    while (fgets(line, sizeof(line), f) && g_count < MAX_PARAMS) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\0') continue;
        
        // Supporta sia "chiave valore" che "chiave=valore"
        for (int i = 0; line[i]; i++) if (line[i] == '=') line[i] = ' ';

        char key[MAX_KEY_LEN], val[MAX_VAL_LEN];
        if (sscanf(line, "%31s %63s", key, val) == 2) {
            strncpy(g_params[g_count].key, key, MAX_KEY_LEN - 1);
            strncpy(g_params[g_count].val, val, MAX_VAL_LEN - 1);
            g_params[g_count].used = true;
            g_count++;
        }
    }
    fclose(f);
}

void params_save(const char* filepath) {
    FILE* f = fopen(filepath, "w");
    if (!f) return;
    for (int i = 0; i < g_count; i++) {
        if (g_params[i].used)
            fprintf(f, "%s %s\n", g_params[i].key, g_params[i].val);
    }
    fclose(f);
}

// GETTERS
int params_get_int(const char* key, int def) {
    int idx = find_param(key);
    return (idx == -1) ? def : atoi(g_params[idx].val);
}

float params_get_float(const char* key, float def) {
    int idx = find_param(key);
    return (idx == -1) ? def : (float)atof(g_params[idx].val);
}

bool params_get_bool(const char* key, bool def) {
    int idx = find_param(key);
    if (idx == -1) return def;
    const char* v = g_params[idx].val;
    return (strcmp(v, "1") == 0 || strcmp(v, "true") == 0 || strcmp(v, "yes") == 0);
}

const char* params_get_string(const char* key, const char* def) {
    int idx = find_param(key);
    return (idx == -1) ? def : g_params[idx].val;
}

// SETTERS (interni)
static void set_param_str(const char* key, const char* val) {
    int idx = find_param(key);
    if (idx != -1) {
        strncpy(g_params[idx].val, val, MAX_VAL_LEN - 1);
    } else if (g_count < MAX_PARAMS) {
        strncpy(g_params[g_count].key, key, MAX_KEY_LEN - 1);
        strncpy(g_params[g_count].val, val, MAX_VAL_LEN - 1);
        g_params[g_count].used = true;
        g_count++;
    }
}

void params_set_int(const char* key, int val) {
    char buf[32]; snprintf(buf, sizeof(buf), "%d", val); set_param_str(key, buf);
}
void params_set_float(const char* key, float val) {
    char buf[32]; snprintf(buf, sizeof(buf), "%.6f", val); set_param_str(key, buf);
}
void params_set_bool(const char* key, bool val) {
    set_param_str(key, val ? "1" : "0");
}
void params_set_string(const char* key, const char* val) {
    set_param_str(key, val);
}