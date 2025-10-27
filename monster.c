#include <stdio.h>
#include <stdlib.h>
#include "monster.h"
#include "sync.h"
#include "combat.h"

static int distancia_manhattan(Coordenada a, Coordenada b) {
    int dx = abs(a.x - b.x);
    int dy = abs(a.y - b.y);
    return dx + dy;
}

static Coordenada mover_hacia(Coordenada desde, Coordenada hasta) {
    Coordenada r = desde;
    if (desde.x < hasta.x) r.x++;
    else if (desde.x > hasta.x) r.x--;
    else if (desde.y < hasta.y) r.y++;
    else if (desde.y > hasta.y) r.y--;
    return r;
}

void *hilo_monstruo(void *arg) {
    ArgumentosMonstruo *ctx = (ArgumentosMonstruo*)arg;
    EstadoMundo *mundo = ctx->mundo;
    const Configuracion *cfg = ctx->cfg;

    int self = 1 + ctx->indice_monstruo;
    int total_ticks = cfg->largo_ruta_heroe + 20;

    for (int t = 0; t < total_ticks; t++) {

        pthread_mutex_lock(&ctx->sync->bloqueo_mundo);

        EstadoEntidad *yo = &mundo->entidades[self];
        EstadoEntidad *heroe = &mundo->entidades[0];

        int dist = distancia_manhattan(yo->posicion, heroe->posicion);

        if (dist <= yo->rango_vision) {
            yo->posicion = mover_hacia(yo->posicion, heroe->posicion);
        }

        // ataque del monstruo
        resolver_ataques(mundo, self);

        printf("[MONSTRUO %d] tick=%d pos=(%d,%d) dist=%d\n",
               ctx->indice_monstruo + 1,
               t,
               yo->posicion.x,
               yo->posicion.y,
               dist
        );

        pthread_mutex_unlock(&ctx->sync->bloqueo_mundo);

        // sincronizar con los demas hilos
        esperar_barrera(ctx->sync);
    }

    return NULL;
}
