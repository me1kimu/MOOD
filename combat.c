#include <stdio.h>
#include "combat.h"

// distancia Manhattan
static int distancia_manhattan(Coordenada a, Coordenada b) {
    int dx = a.x - b.x; if (dx < 0) dx = -dx;
    int dy = a.y - b.y; if (dy < 0) dy = -dy;
    return dx + dy;
}

void resolver_ataques(EstadoMundo *mundo, int atacante_idx) {
    if (!mundo) return;
    if (atacante_idx < 0 || atacante_idx >= mundo->cantidad_entidades) return;

    EstadoEntidad *atacante = &mundo->entidades[atacante_idx];
    if (!atacante->viva) return;

    TipoEntidad tipo_objetivo = (atacante->tipo == ENTIDAD_HEROE) ? ENTIDAD_MONSTRUO : ENTIDAD_HEROE;

    for (int j = 0; j < mundo->cantidad_entidades; j++) {
        if (j == atacante_idx) continue;
        EstadoEntidad *victima = &mundo->entidades[j];
        if (!victima->viva) continue;
        if (victima->tipo != tipo_objetivo) continue;

        int dist = distancia_manhattan(atacante->posicion, victima->posicion);
        if (dist <= atacante->rango_ataque) {
            victima->hp -= atacante->dano_ataque;
            printf("[COMBATE] atacante=%s id=%d ataca a id=%d dano=%d hp_victima=%d\n",
                   (atacante->tipo==ENTIDAD_HEROE) ? "HEROE" : "MONSTRUO",
                   atacante->id,
                   victima->id,
                   atacante->dano_ataque,
                   victima->hp);
            if (victima->hp <= 0) {
                victima->viva = 0;
                printf("[COMBATE] entidad id=%d muere\n", victima->id);
            }
            return;
        }
    }
}
