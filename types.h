#pragma once
#include <pthread.h>

// Estructura para representar una coordenada en la grilla
typedef struct {
    int x;
    int y;
} Coordenada;

// Estructura de configuracion general del archivo .cfg
typedef struct {
    int ancho_grilla;
    int alto_grilla;

    int heroe_hp;
    int heroe_dano_ataque;
    int heroe_rango_ataque;
    Coordenada heroe_inicio;

    Coordenada *ruta_heroe;
    int largo_ruta_heroe;

    int cantidad_monstruos;
    int *monstruo_hp;
    int *monstruo_dano_ataque;
    int *monstruo_rango_vision;
    int *monstruo_rango_ataque;
    Coordenada *monstruo_inicio;
} Configuracion;

// Tipo de entidad en el mundo
typedef enum { ENTIDAD_HEROE, ENTIDAD_MONSTRUO } TipoEntidad;

// Estado de una entidad viva en la simulacion
typedef struct {
    TipoEntidad tipo;
    int id;
    int hp;
    int dano_ataque;
    int rango_ataque;
    int rango_vision;
    Coordenada posicion;
    int viva;
} EstadoEntidad;

// Estado global del mundo en cada tick
typedef struct {
    int ancho_grilla;
    int alto_grilla;
    int cantidad_entidades;
    EstadoEntidad *entidades;
} EstadoMundo;

// Implementacion manual de barrera
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int contador;
    int total;
} Barrera;

// Estructuras de sincronizacion global
typedef struct {
    pthread_mutex_t bloqueo_mundo;
    Barrera barrera_tick;
} ObjetosSincronizacion;
