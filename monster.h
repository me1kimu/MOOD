#pragma once
#include "types.h"

typedef struct {
    int indice_monstruo;
    ObjetosSincronizacion *sync;
    EstadoMundo *mundo;
    const Configuracion *cfg;
} ArgumentosMonstruo;

void *hilo_monstruo(void *arg);
