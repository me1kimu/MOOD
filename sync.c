#include <pthread.h>
#include "sync.h"

// Inicializa la barrera manual
static void iniciar_barrera(Barrera *barrera, int total_hilos) {
    pthread_mutex_init(&barrera->mutex, NULL);
    pthread_cond_init(&barrera->cond, NULL);
    barrera->contador = 0;
    barrera->total = total_hilos;
}

// Espera en la barrera (reemplaza pthread_barrier_wait)
void esperar_barrera(ObjetosSincronizacion *sync) {
    Barrera *b = &sync->barrera_tick;

    pthread_mutex_lock(&b->mutex);
    b->contador++;

    if (b->contador == b->total) {
        b->contador = 0;
        pthread_cond_broadcast(&b->cond);
    } else {
        pthread_cond_wait(&b->cond, &b->mutex);
    }

    pthread_mutex_unlock(&b->mutex);
}

// Inicializa la estructura de sincronizacion
int iniciar_sync(ObjetosSincronizacion *sync, int total_hilos) {
    if (pthread_mutex_init(&sync->bloqueo_mundo, NULL) != 0)
        return 1;

    iniciar_barrera(&sync->barrera_tick, total_hilos);
    return 0;
}

// Destruye la estructura de sincronizacion
void destruir_sync(ObjetosSincronizacion *sync) {
    pthread_mutex_destroy(&sync->bloqueo_mundo);
    pthread_mutex_destroy(&sync->barrera_tick.mutex);
    pthread_cond_destroy(&sync->barrera_tick.cond);
}
