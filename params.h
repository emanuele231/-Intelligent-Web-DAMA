#ifndef PARAMS_H
#define PARAMS_H

#include <stdbool.h>

// Gestione ciclo di vita
void params_load(const char* filepath);
void params_save(const char* filepath);
void params_clear(void);

// Lettura con fallback su valore default
int         params_get_int(const char* key, int def);
float       params_get_float(const char* key, float def);
bool        params_get_bool(const char* key, bool def);
const char* params_get_string(const char* key, const char* def);

// Scrittura runtime
void params_set_int(const char* key, int val);
void params_set_float(const char* key, float val);
void params_set_bool(const char* key, bool val);
void params_set_string(const char* key, const char* val);

#endif // PARAMS_H