#pragma once
#include "types.h"

typedef struct {
    ObjetosSincronizacion *sync;
    EstadoMundo *mundo;
    const Configuracion *cfg;
} ArgumentosHeroe;

void *hilo_heroe(void *arg);
