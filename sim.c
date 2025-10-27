#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "sim.h"
#include "hero.h"
#include "monster.h"
#include "sync.h"

// Funcion principal que coordina la simulacion completa
int ejecutar_simulacion(const Configuracion *cfg) {
    // Crear estructura base del mundo
    EstadoMundo mundo = {0};
    mundo.ancho_grilla = cfg->ancho_grilla;
    mundo.alto_grilla = cfg->alto_grilla;
    mundo.cantidad_entidades = 1 + cfg->cantidad_monstruos;
    mundo.entidades = calloc(mundo.cantidad_entidades, sizeof(EstadoEntidad));
    if (!mundo.entidades) {
        fprintf(stderr, "Error: no hay memoria para las entidades\n");
        return 1;
    }

    // Configurar el heroe
    mundo.entidades[0].tipo = ENTIDAD_HEROE;
    mundo.entidades[0].id = 0;
    mundo.entidades[0].hp = cfg->heroe_hp;
    mundo.entidades[0].dano_ataque = cfg->heroe_dano_ataque;
    mundo.entidades[0].rango_ataque = cfg->heroe_rango_ataque;
    mundo.entidades[0].posicion = cfg->heroe_inicio;
    mundo.entidades[0].viva = 1;

    // Configurar monstruos
    for (int i = 0; i < cfg->cantidad_monstruos; i++) {
        int indice = 1 + i;
        mundo.entidades[indice].tipo = ENTIDAD_MONSTRUO;
        mundo.entidades[indice].id = i;
        mundo.entidades[indice].hp = cfg->monstruo_hp[i];
        mundo.entidades[indice].dano_ataque = cfg->monstruo_dano_ataque[i];
        mundo.entidades[indice].rango_ataque = cfg->monstruo_rango_ataque[i];
        mundo.entidades[indice].rango_vision = cfg->monstruo_rango_vision[i];
        mundo.entidades[indice].posicion = cfg->monstruo_inicio[i];
        mundo.entidades[indice].viva = 1;
    }

    // Inicializar sincronizacion (heroe + todos los monstruos)
    ObjetosSincronizacion sync;
    int total_hilos = 1 + cfg->cantidad_monstruos;
    if (iniciar_sync(&sync, total_hilos) != 0) {
        fprintf(stderr, "Error iniciando sincronizacion\n");
        free(mundo.entidades);
        return 1;
    }

    // Crear hilo del heroe
    pthread_t hilo_heroe_id;
    ArgumentosHeroe args_heroe = {
        .sync = &sync,
        .mundo = &mundo,
        .cfg = cfg
    };

    if (pthread_create(&hilo_heroe_id, NULL, hilo_heroe, &args_heroe) != 0) {
        fprintf(stderr, "Error creando el hilo del heroe\n");
        destruir_sync(&sync);
        free(mundo.entidades);
        return 1;
    }

    // Crear hilos de monstruos
    pthread_t *hilos_monstruos = calloc(cfg->cantidad_monstruos, sizeof(pthread_t));
    ArgumentosMonstruo *args_monstruos = calloc(cfg->cantidad_monstruos, sizeof(ArgumentosMonstruo));
    if (!hilos_monstruos || !args_monstruos) {
        fprintf(stderr, "Error: no hay memoria para hilos de monstruos\n");
        destruir_sync(&sync);
        free(mundo.entidades);
        return 1;
    }

    for (int i = 0; i < cfg->cantidad_monstruos; i++) {
        args_monstruos[i].indice_monstruo = i;
        args_monstruos[i].sync = &sync;
        args_monstruos[i].mundo = &mundo;
        args_monstruos[i].cfg = cfg;

        if (pthread_create(&hilos_monstruos[i], NULL, hilo_monstruo, &args_monstruos[i]) != 0) {
            fprintf(stderr, "Error creando hilo del monstruo %d\n", i + 1);
        }
    }

    // Esperar que todos los hilos terminen
    pthread_join(hilo_heroe_id, NULL);
    for (int i = 0; i < cfg->cantidad_monstruos; i++) {
        pthread_join(hilos_monstruos[i], NULL);
    }

    // Liberar memoria y destruir sincronizacion
    free(hilos_monstruos);
    free(args_monstruos);
    destruir_sync(&sync);
    free(mundo.entidades);

    return 0;
}
