# DOOM Simulation

Este proyecto es una simulación del clásico videojuego DOOM implementada con programación concurrente usando hilos (threads) en C. La simulación incluye héroes que se mueven por un grid siguiendo rutas predefinidas y monstruos que detectan, persiguen y atacan a los héroes.

## Requisitos

- Sistema operativo UNIX/Linux
- Compilador GCC
- Biblioteca pthread (usualmente incluida en sistemas UNIX)

## Compilación

Para compilar el proyecto, use el Makefile incluido:

```bash
make
```

Esto generará el ejecutable `doom_simulation`.

## Ejecución

Para ejecutar la simulación, debe proporcionar un archivo de configuración:

```bash
./doom_simulation config_example.txt
```

O use el comando make:

```bash
make run
```

### Archivos de Configuración Incluidos

El repositorio incluye tres archivos de configuración de ejemplo:

1. **config_example.txt** - Escenario básico con 1 héroe y 3 monstruos (del PDF)
2. **config_multi_hero.txt** - Escenario avanzado con 2 héroes y 5 monstruos (extensión de 15 puntos)
3. **config_hard.txt** - Escenario difícil donde el héroe puede morir enfrentando 4 monstruos

Puede ejecutar cualquiera de ellos:
```bash
./doom_simulation config_example.txt
./doom_simulation config_multi_hero.txt
./doom_simulation config_hard.txt
```

## Formato del Archivo de Configuración

El archivo de configuración debe seguir este formato:

### Para Un Solo Héroe (Formato Original - 45 puntos):
```
GRID_SIZE <ancho> <alto>
HERO_HP <vida_inicial>
HERO_ATTACK_DAMAGE <daño_ataque>
HERO_ATTACK_RANGE <rango_ataque>
HERO_START <x> <y>
HERO_PATH (x1,y1) (x2,y2) (x3,y3) ...
MONSTER_COUNT <número_monstruos>
MONSTER_<id>_HP <vida>
MONSTER_<id>_ATTACK_DAMAGE <daño>
MONSTER_<id>_VISION_RANGE <rango_visión>
MONSTER_<id>_ATTACK_RANGE <rango_ataque>
MONSTER_<id>_COORDS <x> <y>
```

### Para Múltiples Héroes (Extensión - 15 puntos):
```
GRID_SIZE <ancho> <alto>
HERO_COUNT <número_héroes>
HERO_<id>_HP <vida_inicial>
HERO_<id>_ATTACK_DAMAGE <daño_ataque>
HERO_<id>_ATTACK_RANGE <rango_ataque>
HERO_<id>_START <x> <y>
HERO_<id>_PATH (x1,y1) (x2,y2) (x3,y3) ...
MONSTER_COUNT <número_monstruos>
MONSTER_<id>_HP <vida>
MONSTER_<id>_ATTACK_DAMAGE <daño>
MONSTER_<id>_VISION_RANGE <rango_visión>
MONSTER_<id>_ATTACK_RANGE <rango_ataque>
MONSTER_<id>_COORDS <x> <y>
```

### Ejemplo:

```
GRID_SIZE 30 20
HERO_HP 150
HERO_ATTACK_DAMAGE 20
HERO_ATTACK_RANGE 3
HERO_START 2 2
HERO_PATH (3,2) (4,2) (5,2) (5,3) (5,4) (6,4)
MONSTER_COUNT 3
MONSTER_1_HP 50
MONSTER_1_ATTACK_DAMAGE 10
MONSTER_1_VISION_RANGE 5
MONSTER_1_ATTACK_RANGE 1
MONSTER_1_COORDS 8 4
```

## Funcionamiento

### Héroe
- Comienza en la posición `HERO_START`
- Sigue la ruta definida en `HERO_PATH`
- Se detiene cuando hay monstruos en su rango de ataque
- Ataca a todos los monstruos en su rango
- Puede morir si su HP llega a 0

### Monstruos
- Comienzan en modo pasivo en sus coordenadas iniciales
- Se activan cuando detectan un héroe en su rango de visión
- Al activarse, alertan a otros monstruos en su rango de visión
- Persiguen al héroe más cercano usando la distancia de Manhattan
- Atacan cuando el héroe está en su rango de ataque

### Concurrencia
- Cada héroe corre en un thread independiente
- Cada monstruo corre en un thread independiente
- Se usan mutexes para sincronizar el acceso al estado compartido
- La simulación termina cuando:
  - Todos los héroes mueren
  - Todos los héroes llegan al final de su ruta
  - Todos los monstruos mueren

## Soporte para Múltiples Héroes

El programa soporta múltiples héroes (extensión de 15 puntos). Para agregar más héroes al archivo de configuración, simplemente agregue más secciones de héroe con sus respectivas rutas.

## Limpieza

Para eliminar el ejecutable compilado:

```bash
make clean
```

## Arquitectura del Código

El código está estructurado de la siguiente manera:

- **Estructuras de datos**: `Hero`, `Monster`, `SimulationState`
- **Funciones de parsing**: `parse_config_file()`
- **Funciones de movimiento**: `move_hero()`, `move_monster_towards_hero()`
- **Funciones de combate**: `hero_attack_monsters()`, `monster_attack_hero()`
- **Funciones de IA de monstruos**: `check_monster_sees_hero()`, `alert_nearby_monsters()`
- **Threads**: `hero_thread_function()`, `monster_thread_function()`
- **Main loop**: Monitorea el estado de la simulación y coordina los threads

## Notas de Implementación

- La distancia de Manhattan se usa para el pathfinding de los monstruos
- Los delays de 1 segundo entre acciones hacen la simulación más visible
- Los mutexes previenen condiciones de carrera en el acceso al estado compartido
- Los monstruos priorizan moverse en la dirección que más reduce la distancia Manhattan

## Autor

Desarrollado para el curso de Sistemas Operativos - Universidad Diego Portales
