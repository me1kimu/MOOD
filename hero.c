#define _DEFAULT_SOURCE
#include <stdio.h>
#include <unistd.h>
#include "hero.h"
#include "sync.h"
#include "combat.h"

void *hilo_heroe(void *arg) {
    ArgumentosHeroe *ctx = (ArgumentosHeroe*)arg;
    const Configuracion *cfg = ctx->cfg;
    EstadoMundo *mundo = ctx->mundo;

    int paso = 0;
    int total_ticks = cfg->largo_ruta_heroe + 20;

    for (int t = 0; t < total_ticks; t++) {

        // Bloqueo mundo antes de leer / escribir
        pthread_mutex_lock(&ctx->sync->bloqueo_mundo);

        // Avanzar al heroe segun la ruta
        if (paso < cfg->largo_ruta_heroe) {
            mundo->entidades[0].posicion = cfg->ruta_heroe[paso];
            paso++;
        }

        // Resolver ataque del heroe (indice 0 en el arreglo de entidades)
        resolver_ataques(mundo, 0);

        // Log estado del heroe
        Coordenada pos_actual = mundo->entidades[0].posicion;
        printf("[HEROE] tick=%d pos=(%d,%d) hp=%d\n",
               t,
               pos_actual.x,
               pos_actual.y,
               mundo->entidades[0].hp
        );

        // Desbloquear mundo
        pthread_mutex_unlock(&ctx->sync->bloqueo_mundo);

        // Barrera de sincronizacion por tick
        esperar_barrera(ctx->sync);

        // Pausa para que la salida sea legible
        usleep(40000);
    }

    return NULL;
}
