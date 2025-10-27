#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "sim.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s archivo_configuracion\n", argv[0]);
        return 1;
    }

    Configuracion cfg;
    if (cargar_configuracion(argv[1], &cfg) != 0) {
        fprintf(stderr, "Error: no se pudo cargar el archivo '%s'\n", argv[1]);
        return 1;
    }

    int resultado = ejecutar_simulacion(&cfg);
    liberar_configuracion(&cfg);
    return resultado;
}
