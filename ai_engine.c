#include "ai_engine.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_AI_ENGINES 16

static const AIEngineDef* registry[MAX_AI_ENGINES];
static int registry_count = 0;

void ai_register(const AIEngineDef* def) {
    if (!def || !def->id || !def->create || !def->get_move || !def->destroy) {
        fprintf(stderr, "ERRORE: Registrazione IA fallita (campi obbligatori mancanti)\n");
        return;
    }
    if (registry_count >= MAX_AI_ENGINES) {
        fprintf(stderr, "ERRORE: Registry piena (%d max). Aumenta MAX_AI_ENGINES.\n", MAX_AI_ENGINES);
        return;
    }
    for (int i = 0; i < registry_count; i++) {
        if (strcmp(registry[i]->id, def->id) == 0) {
            fprintf(stderr, "ERRORE: IA con ID '%s' già registrata.\n", def->id);
            return;
        }
    }
    registry[registry_count++] = def;
}

const AIEngineDef* ai_find(const char* id) {
    if (!id) return NULL;
    for (int i = 0; i < registry_count; i++) {
        if (strcmp(registry[i]->id, id) == 0) return registry[i];
    }
    return NULL;
}

int ai_count(void) {
    return registry_count;
}

const AIEngineDef** ai_list_all(void) {
    return (const AIEngineDef**)registry;
}