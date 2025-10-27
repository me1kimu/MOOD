#pragma once
#include "types.h"

int iniciar_sync(ObjetosSincronizacion *sync, int total_hilos);
void destruir_sync(ObjetosSincronizacion *sync);
void esperar_barrera(ObjetosSincronizacion *sync);
