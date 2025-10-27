#pragma once
#include "types.h"

// Resuelve el ataque de la entidad 'atacante_idx' contra objetivos en rango.
// Debe llamarse con el bloqueo del mundo (bloqueo_mundo) tomado.
void resolver_ataques(EstadoMundo *mundo, int atacante_idx);
