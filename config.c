#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "config.h"

// Inicializa en cero
static void limpiar_config(Configuracion *c) {
    memset(c, 0, sizeof(*c));
}

// Lee dos enteros de una linea
static int leer_dos_enteros(const char *s, int *a, int *b) {
    return sscanf(s, " %d %d", a, b) == 2 ? 0 : -1;
}

// Lee una lista de tuplas (x,y)
static int leer_lista_tuplas(const char *s, Coordenada **out, int *largo) {
    int capacidad = 8, n = 0;
    Coordenada *arr = malloc(sizeof(Coordenada) * capacidad);
    if (!arr) return -1;

    const char *p = s;
    while (*p) {
        while (isspace((unsigned char)*p)) p++;
        if (*p == '\0') break;
        if (*p != '(') break;

        int x, y; char c1, c2, c3;
        int m = sscanf(p, " %c %d %c %d %c", &c1, &x, &c2, &y, &c3);
        if (m != 5 || c1 != '(' || c2 != ',' || c3 != ')') {
            free(arr);
            return -1;
        }

        if (n == capacidad) {
            capacidad *= 2;
            Coordenada *tmp = realloc(arr, sizeof(Coordenada) * capacidad);
            if (!tmp) { free(arr); return -1; }
            arr = tmp;
        }

        arr[n++] = (Coordenada){x, y};

        const char *q = strchr(p, ')');
        if (!q) break;
        p = q + 1;
    }

    if (n == 0) { free(arr); return -1; }
    *out = arr;
    *largo = n;
    return 0;
}

int cargar_configuracion(const char *ruta, Configuracion *cfg) {
    limpiar_config(cfg);

    FILE *f = fopen(ruta, "r");
    if (!f) return 1;

    char linea[4096];
    while (fgets(linea, sizeof(linea), f)) {
        char *comentario = strchr(linea, '#');
        if (comentario) *comentario = '\0';

        char clave[128];
        if (sscanf(linea, " %127s", clave) != 1) continue;

        if (strcmp(clave, "GRID_SIZE") == 0) {
            leer_dos_enteros(linea + strlen(clave), &cfg->ancho_grilla, &cfg->alto_grilla);
        } 
        else if (strcmp(clave, "HERO_HP") == 0) {
            sscanf(linea, " %*s %d", &cfg->heroe_hp);
        } 
        else if (strcmp(clave, "HERO_ATTACK_DAMAGE") == 0) {
            sscanf(linea, " %*s %d", &cfg->heroe_dano_ataque);
        } 
        else if (strcmp(clave, "HERO_ATTACK_RANGE") == 0) {
            sscanf(linea, " %*s %d", &cfg->heroe_rango_ataque);
        } 
        else if (strcmp(clave, "HERO_START") == 0) {
            leer_dos_enteros(linea + strlen(clave), &cfg->heroe_inicio.x, &cfg->heroe_inicio.y);
        } 
        else if (strcmp(clave, "HERO_PATH") == 0) {
            char *resto = strchr(linea, ' ');
            leer_lista_tuplas(resto, &cfg->ruta_heroe, &cfg->largo_ruta_heroe);
        } 
        else if (strcmp(clave, "MONSTER_COUNT") == 0) {
            sscanf(linea, " %*s %d", &cfg->cantidad_monstruos);
            int n = cfg->cantidad_monstruos;
            cfg->monstruo_hp = calloc(n, sizeof(int));
            cfg->monstruo_dano_ataque = calloc(n, sizeof(int));
            cfg->monstruo_rango_vision = calloc(n, sizeof(int));
            cfg->monstruo_rango_ataque = calloc(n, sizeof(int));
            cfg->monstruo_inicio = calloc(n, sizeof(Coordenada));
        } 
        else if (strncmp(clave, "MONSTER_", 8) == 0) {
            int indice;
            char resto[64];
            if (sscanf(clave, "MONSTER_%d_%63s", &indice, resto) == 2 && indice >= 1) {
                int i = indice - 1;
                if (strcmp(resto, "HP") == 0)
                    sscanf(linea, " %*s %d", &cfg->monstruo_hp[i]);
                else if (strcmp(resto, "ATTACK_DAMAGE") == 0)
                    sscanf(linea, " %*s %d", &cfg->monstruo_dano_ataque[i]);
                else if (strcmp(resto, "VISION_RANGE") == 0)
                    sscanf(linea, " %*s %d", &cfg->monstruo_rango_vision[i]);
                else if (strcmp(resto, "ATTACK_RANGE") == 0)
                    sscanf(linea, " %*s %d", &cfg->monstruo_rango_ataque[i]);
                else if (strcmp(resto, "COORDS") == 0)
                    leer_dos_enteros(linea + strlen(clave), &cfg->monstruo_inicio[i].x, &cfg->monstruo_inicio[i].y);
            }
        }
    }

    fclose(f);
    return 0;
}

void liberar_configuracion(Configuracion *cfg) {
    free(cfg->ruta_heroe);
    free(cfg->monstruo_hp);
    free(cfg->monstruo_dano_ataque);
    free(cfg->monstruo_rango_vision);
    free(cfg->monstruo_rango_ataque);
    free(cfg->monstruo_inicio);
    memset(cfg, 0, sizeof(*cfg));
}
