#pragma once
#include <pthread.h>
#include "types.h"

// Carga los datos de configuracion desde un archivo .cfg
int cargar_configuracion(const char *ruta, Configuracion *cfg);

// Libera la memoria asociada a la configuracion
void liberar_configuracion(Configuracion *cfg);
