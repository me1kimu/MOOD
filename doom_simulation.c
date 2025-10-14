#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <limits.h>

#define MAX_PATH_LENGTH 100
#define MAX_MONSTERS 50
#define MAX_HEROES 10
#define MAX_LINE_LENGTH 256

// Structure for coordinates
typedef struct {
    int x;
    int y;
} Coords;

// Structure for Hero
typedef struct {
    int id;
    int hp;
    int attack_damage;
    int attack_range;
    Coords start;
    Coords current;
    Coords path[MAX_PATH_LENGTH];
    int path_length;
    int path_index;
    bool is_alive;
    bool in_combat;
    pthread_t thread;
} Hero;

// Structure for Monster
typedef struct {
    int id;
    int hp;
    int attack_damage;
    int vision_range;
    int attack_range;
    Coords start;
    Coords current;
    bool is_alive;
    bool is_alerted;
    pthread_t thread;
} Monster;

// Global simulation state
typedef struct {
    int grid_width;
    int grid_height;
    Hero heroes[MAX_HEROES];
    int hero_count;
    Monster monsters[MAX_MONSTERS];
    int monster_count;
    bool simulation_running;
    pthread_mutex_t simulation_mutex;
} SimulationState;

SimulationState sim_state;

// Function prototypes
int manhattan_distance(Coords a, Coords b);
void parse_config_file(const char *filename);
void* hero_thread_function(void* arg);
void* monster_thread_function(void* arg);
void print_simulation_state();
void move_hero(Hero *hero);
void move_monster_towards_hero(Monster *monster, Coords hero_pos);
void hero_attack_monsters(Hero *hero);
void monster_attack_hero(Monster *monster, Hero *hero);
void alert_nearby_monsters(Monster *alerting_monster);
bool check_monster_sees_hero(Monster *monster);
Coords get_closest_hero_position(Monster *monster);
Hero* get_closest_hero(Monster *monster);

// Calculate Manhattan distance between two points
int manhattan_distance(Coords a, Coords b) {
    return abs(a.x - b.x) + abs(a.y - b.y);
}

// Parse the configuration file
void parse_config_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open configuration file %s\n", filename);
        exit(1);
    }

    char line[MAX_LINE_LENGTH];
    sim_state.hero_count = 0;
    sim_state.monster_count = 0;

    while (fgets(line, sizeof(line), file)) {
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        if (strncmp(line, "GRID_SIZE", 9) == 0) {
            if (sscanf(line, "GRID_SIZE %d %d", &sim_state.grid_width, &sim_state.grid_height) != 2) { fprintf(stderr, "Error: Malformed GRID_SIZE line.\n"); exit(1); }
        }
        else if (strncmp(line, "HERO_COUNT", 10) == 0) {
            // Just for information, we'll count as we parse
        }
        // Handle single hero format (HERO_HP, HERO_START, etc.)
        else if (strncmp(line, "HERO_HP", 7) == 0 && line[7] == ' ') {
            sscanf(line, "HERO_HP %d", &sim_state.heroes[0].hp);
        }
        else if (strncmp(line, "HERO_ATTACK_DAMAGE", 18) == 0 && line[18] == ' ') {
            sscanf(line, "HERO_ATTACK_DAMAGE %d", &sim_state.heroes[0].attack_damage);
        }
        else if (strncmp(line, "HERO_ATTACK_RANGE", 17) == 0 && line[17] == ' ') {
            sscanf(line, "HERO_ATTACK_RANGE %d", &sim_state.heroes[0].attack_range);
        }
        else if (strncmp(line, "HERO_START", 10) == 0 && line[10] == ' ') {
            sscanf(line, "HERO_START %d %d", 
                   &sim_state.heroes[0].start.x,
                   &sim_state.heroes[0].start.y);
            sim_state.heroes[0].current = sim_state.heroes[0].start;
        }
        else if (strncmp(line, "HERO_PATH", 9) == 0 && line[9] == ' ') {
            Hero *hero = &sim_state.heroes[0];
            hero->path_length = 0;
            hero->path_index = 0;
            
            char *ptr = strchr(line, '(');
            while (ptr != NULL) {
                int x, y;
                if (sscanf(ptr, "(%d,%d)", &x, &y) == 2) {
                    hero->path[hero->path_length].x = x;
                    hero->path[hero->path_length].y = y;
                    hero->path_length++;
                }
                ptr = strchr(ptr + 1, '(');
            }
            
            hero->id = 0;
            hero->is_alive = true;
            hero->in_combat = false;
            if (sim_state.hero_count == 0) sim_state.hero_count = 1;
        }
        // Handle multi-hero format (HERO_1_HP, HERO_2_HP, etc.)
        else if (strstr(line, "HERO_") != NULL && strstr(line, "_HP") != NULL) {
            int hero_id;
            int hp;
            sscanf(line, "HERO_%d_HP %d", &hero_id, &hp);
            
            int idx = hero_id - 1;
            if (idx < 0 || idx >= MAX_HEROES) {
                fprintf(stderr, "Error: HERO_ID %d out of bounds (max %d heroes)\n", hero_id, MAX_HEROES);
                fclose(file);
                exit(1);
            }
            if (idx >= sim_state.hero_count) {
                sim_state.hero_count = idx + 1;
            }
            sim_state.heroes[idx].id = hero_id;
            sim_state.heroes[idx].hp = hp;
            sim_state.heroes[idx].is_alive = true;
            sim_state.heroes[idx].in_combat = false;
        }
        else if (strstr(line, "HERO_") != NULL && strstr(line, "_ATTACK_DAMAGE") != NULL) {
            int hero_id, damage;
            sscanf(line, "HERO_%d_ATTACK_DAMAGE %d", &hero_id, &damage);
            int idx = hero_id - 1;
            if (idx >= 0 && idx < MAX_HEROES) {
                sim_state.heroes[idx].attack_damage = damage;
            }
        }
        else if (strstr(line, "HERO_") != NULL && strstr(line, "_ATTACK_RANGE") != NULL) {
            int hero_id, range;
            sscanf(line, "HERO_%d_ATTACK_RANGE %d", &hero_id, &range);
            int idx = hero_id - 1;
            if (idx >= 0 && idx < MAX_HEROES) {
                sim_state.heroes[idx].attack_range = range;
            }
        }
        else if (strstr(line, "HERO_") != NULL && strstr(line, "_START") != NULL) {
            int hero_id, x, y;
            sscanf(line, "HERO_%d_START %d %d", &hero_id, &x, &y);
            int idx = hero_id - 1;
            if (idx >= 0 && idx < MAX_HEROES) {
                sim_state.heroes[idx].start.x = x;
                sim_state.heroes[idx].start.y = y;
                sim_state.heroes[idx].current.x = x;
                sim_state.heroes[idx].current.y = y;
            }
        }
        else if (strstr(line, "HERO_") != NULL && strstr(line, "_PATH") != NULL) {
            int hero_id;
            sscanf(line, "HERO_%d_PATH", &hero_id);
            int idx = hero_id - 1;
            
            if (idx >= 0 && idx < MAX_HEROES) {
                Hero *hero = &sim_state.heroes[idx];
                hero->path_length = 0;
                hero->path_index = 0;
                
                char *ptr = strchr(line, '(');
                while (ptr != NULL) {
                    int x, y;
                    if (sscanf(ptr, "(%d,%d)", &x, &y) == 2) {
                        hero->path[hero->path_length].x = x;
                        hero->path[hero->path_length].y = y;
                        hero->path_length++;
                    }
                    ptr = strchr(ptr + 1, '(');
                }
            }
        }
        else if (strncmp(line, "MONSTER_COUNT", 13) == 0) {
            int count;
            sscanf(line, "MONSTER_COUNT %d", &count);
            // Just for information, we'll count as we parse
        }
        else if (strstr(line, "MONSTER_") != NULL && strstr(line, "_HP") != NULL) {
            int monster_id;
            int hp;
            sscanf(line, "MONSTER_%d_HP %d", &monster_id, &hp);
            
            // Initialize monster if needed
            int idx = monster_id - 1;
            if (idx < 0 || idx >= MAX_MONSTERS) {
                fprintf(stderr, "Error: MONSTER_ID %d out of bounds (max %d monsters)\n", monster_id, MAX_MONSTERS);
                fclose(file);
                exit(1);
            }
            if (idx >= sim_state.monster_count) {
                sim_state.monster_count = idx + 1;
            }
            sim_state.monsters[idx].id = monster_id;
            sim_state.monsters[idx].hp = hp;
            sim_state.monsters[idx].is_alive = true;
            sim_state.monsters[idx].is_alerted = false;
        }
        else if (strstr(line, "MONSTER_") != NULL && strstr(line, "_ATTACK_DAMAGE") != NULL) {
            int monster_id, damage;
            sscanf(line, "MONSTER_%d_ATTACK_DAMAGE %d", &monster_id, &damage);
            int idx = monster_id - 1;
            if (idx >= 0 && idx < MAX_MONSTERS) {
                sim_state.monsters[idx].attack_damage = damage;
            }
        }
        else if (strstr(line, "MONSTER_") != NULL && strstr(line, "_VISION_RANGE") != NULL) {
            int monster_id, range;
            sscanf(line, "MONSTER_%d_VISION_RANGE %d", &monster_id, &range);
            int idx = monster_id - 1;
            if (idx >= 0 && idx < MAX_MONSTERS) {
                sim_state.monsters[idx].vision_range = range;
            }
        }
        else if (strstr(line, "MONSTER_") != NULL && strstr(line, "_ATTACK_RANGE") != NULL) {
            int monster_id, range;
            sscanf(line, "MONSTER_%d_ATTACK_RANGE %d", &monster_id, &range);
            int idx = monster_id - 1;
            if (idx >= 0 && idx < MAX_MONSTERS) {
                sim_state.monsters[idx].attack_range = range;
            }
        }
        else if (strstr(line, "MONSTER_") != NULL && strstr(line, "_COORDS") != NULL) {
            int monster_id, x, y;
            sscanf(line, "MONSTER_%d_COORDS %d %d", &monster_id, &x, &y);
            int idx = monster_id - 1;
            if (idx >= 0 && idx < MAX_MONSTERS) {
                sim_state.monsters[idx].start.x = x;
                sim_state.monsters[idx].start.y = y;
                sim_state.monsters[idx].current.x = x;
                sim_state.monsters[idx].current.y = y;
            }
        }
    }

    fclose(file);
}

// Get the closest hero position to a monster
Coords get_closest_hero_position(Monster *monster) {
    Coords closest = sim_state.heroes[0].current;
    int min_distance = manhattan_distance(monster->current, sim_state.heroes[0].current);
    
    for (int i = 1; i < sim_state.hero_count; i++) {
        if (sim_state.heroes[i].is_alive) {
            int dist = manhattan_distance(monster->current, sim_state.heroes[i].current);
            if (dist < min_distance) {
                min_distance = dist;
                closest = sim_state.heroes[i].current;
            }
        }
    }
    
    return closest;
}

// Get the closest hero pointer to a monster (returns NULL if no heroes alive)
Hero* get_closest_hero(Monster *monster) {
    Hero *closest = NULL;
    int min_distance = INT_MAX;
    
    for (int i = 0; i < sim_state.hero_count; i++) {
        if (sim_state.heroes[i].is_alive) {
            int dist = manhattan_distance(monster->current, sim_state.heroes[i].current);
            if (dist < min_distance) {
                min_distance = dist;
                closest = &sim_state.heroes[i];
            }
        }
    }
    
    return closest;
}

// Check if monster can see any hero
bool check_monster_sees_hero(Monster *monster) {
    for (int i = 0; i < sim_state.hero_count; i++) {
        if (sim_state.heroes[i].is_alive) {
            int dist = manhattan_distance(monster->current, sim_state.heroes[i].current);
            if (dist <= monster->vision_range) {
                return true;
            }
        }
    }
    return false;
}

// Alert nearby monsters
void alert_nearby_monsters(Monster *alerting_monster) {
    pthread_mutex_lock(&sim_state.simulation_mutex);
    
    for (int i = 0; i < sim_state.monster_count; i++) {
        if (sim_state.monsters[i].is_alive && !sim_state.monsters[i].is_alerted && 
            sim_state.monsters[i].id != alerting_monster->id) {
            
            int dist = manhattan_distance(alerting_monster->current, sim_state.monsters[i].current);
            if (dist <= alerting_monster->vision_range) {
                sim_state.monsters[i].is_alerted = true;
                printf("Monster %d alerted by Monster %d!\n", 
                       sim_state.monsters[i].id, alerting_monster->id);
            }
        }
    }
    
    pthread_mutex_unlock(&sim_state.simulation_mutex);
}

// Move monster towards closest hero using Manhattan distance
void move_monster_towards_hero(Monster *monster, Coords target) {
    Coords next = monster->current;
    
    int dx = target.x - monster->current.x;
    int dy = target.y - monster->current.y;
    
    // Move in the direction that reduces Manhattan distance most
    if (abs(dx) > abs(dy)) {
        if (dx > 0) next.x++;
        else if (dx < 0) next.x--;
    } else {
        if (dy > 0) next.y++;
        else if (dy < 0) next.y--;
    }
    
    // Check bounds
    if (next.x >= 0 && next.x < sim_state.grid_width &&
        next.y >= 0 && next.y < sim_state.grid_height) {
        monster->current = next;
    }
}

// Move hero along predefined path
void move_hero(Hero *hero) {
    if (hero->path_index < hero->path_length) {
        hero->current = hero->path[hero->path_index];
        hero->path_index++;
        printf("Hero %d moved to (%d, %d)\n", hero->id, hero->current.x, hero->current.y);
    }
}

// Hero attacks monsters in range (assumes mutex is already locked)
void hero_attack_monsters(Hero *hero) {
    bool monsters_in_range = false;
    
    for (int i = 0; i < sim_state.monster_count; i++) {
        if (sim_state.monsters[i].is_alive) {
            int dist = manhattan_distance(hero->current, sim_state.monsters[i].current);
            if (dist <= hero->attack_range) {
                monsters_in_range = true;
                sim_state.monsters[i].hp -= hero->attack_damage;
                printf("Hero %d attacks Monster %d for %d damage! (Monster HP: %d)\n",
                       hero->id, sim_state.monsters[i].id, hero->attack_damage, sim_state.monsters[i].hp);
                
                if (sim_state.monsters[i].hp <= 0) {
                    sim_state.monsters[i].is_alive = false;
                    printf("Monster %d has been defeated!\n", sim_state.monsters[i].id);
                }
            }
        }
    }
    
    hero->in_combat = monsters_in_range;
}

// Monster attacks closest hero (assumes mutex is already locked)
void monster_attack_hero(Monster *monster, Hero *target_hero) {
    if (target_hero->is_alive) {
        int dist = manhattan_distance(monster->current, target_hero->current);
        if (dist <= monster->attack_range) {
            target_hero->hp -= monster->attack_damage;
            printf("Monster %d attacks Hero %d for %d damage! (Hero HP: %d)\n",
                   monster->id, target_hero->id, monster->attack_damage, target_hero->hp);
            
            if (target_hero->hp <= 0) {
                target_hero->is_alive = false;
                printf("Hero %d has died!\n", target_hero->id);
            }
        }
    }
}

// Hero thread function
void* hero_thread_function(void* arg) {
    Hero *hero = (Hero *)arg;
    
    printf("Hero %d starting at (%d, %d) with %d HP\n", 
           hero->id, hero->current.x, hero->current.y, hero->hp);
    
    bool running = true;
    while (running && hero->is_alive) {
        pthread_mutex_lock(&sim_state.simulation_mutex);
        
        // Check simulation_running flag while holding mutex
        running = sim_state.simulation_running;
        if (!running) {
            pthread_mutex_unlock(&sim_state.simulation_mutex);
            break;
        }
        
        // Check if there are monsters in attack range
        bool monsters_nearby = false;
        for (int i = 0; i < sim_state.monster_count; i++) {
            if (sim_state.monsters[i].is_alive) {
                int dist = manhattan_distance(hero->current, sim_state.monsters[i].current);
                if (dist <= hero->attack_range) {
                    monsters_nearby = true;
                    break;
                }
            }
        }
        
        if (monsters_nearby) {
            hero->in_combat = true;
            hero_attack_monsters(hero);
            pthread_mutex_unlock(&sim_state.simulation_mutex);
        } else {
            hero->in_combat = false;
            
            // Move along path if not at end
            if (hero->path_index < hero->path_length) {
                move_hero(hero);
                pthread_mutex_unlock(&sim_state.simulation_mutex);
            } else {
                pthread_mutex_unlock(&sim_state.simulation_mutex);
                // Reached end of path
                printf("Hero %d reached the end of the path!\n", hero->id);
                break;
            }
        }
        
        sleep(1); // 1 second delay
    }
    
    if (!hero->is_alive) {
        printf("Hero %d thread ending - hero died\n", hero->id);
    }
    
    return NULL;
}

// Monster thread function
void* monster_thread_function(void* arg) {
    Monster *monster = (Monster *)arg;
    
    printf("Monster %d starting at (%d, %d) with %d HP\n",
           monster->id, monster->current.x, monster->current.y, monster->hp);
    
    bool running = true;
    while (running && monster->is_alive) {
        pthread_mutex_lock(&sim_state.simulation_mutex);
        
        // Check simulation_running flag while holding mutex
        running = sim_state.simulation_running;
        if (!running) {
            pthread_mutex_unlock(&sim_state.simulation_mutex);
            break;
        }
        
        // Check if hero is in vision range or if alerted
        bool hero_detected = check_monster_sees_hero(monster);
        
        if (hero_detected && !monster->is_alerted) {
            monster->is_alerted = true;
            printf("Monster %d spotted a hero!\n", monster->id);
            pthread_mutex_unlock(&sim_state.simulation_mutex);
            alert_nearby_monsters(monster);
            pthread_mutex_lock(&sim_state.simulation_mutex);
        }
        
        if (monster->is_alerted) {
            // Find closest hero using helper function
            Hero *closest_hero = get_closest_hero(monster);
            
            if (closest_hero != NULL) {
                int min_dist = manhattan_distance(monster->current, closest_hero->current);
                
                // Attack if in range, otherwise move closer
                if (min_dist <= monster->attack_range) {
                    monster_attack_hero(monster, closest_hero);
                } else {
                    Coords target = closest_hero->current;
                    move_monster_towards_hero(monster, target);
                    printf("Monster %d moved to (%d, %d) (chasing hero)\n",
                           monster->id, monster->current.x, monster->current.y);
                }
            }
        }
        
        pthread_mutex_unlock(&sim_state.simulation_mutex);
        sleep(1); // 1 second delay
    }
    
    printf("Monster %d thread ending\n", monster->id);
    return NULL;
}

// Print current simulation state
void print_simulation_state() {
    printf("\n=== Simulation State ===\n");
    printf("Grid: %dx%d\n", sim_state.grid_width, sim_state.grid_height);
    
    for (int i = 0; i < sim_state.hero_count; i++) {
        Hero *h = &sim_state.heroes[i];
        printf("Hero %d: HP=%d, Pos=(%d,%d), Alive=%s\n",
               h->id, h->hp, h->current.x, h->current.y, h->is_alive ? "Yes" : "No");
    }
    
    for (int i = 0; i < sim_state.monster_count; i++) {
        Monster *m = &sim_state.monsters[i];
        printf("Monster %d: HP=%d, Pos=(%d,%d), Alerted=%s, Alive=%s\n",
               m->id, m->hp, m->current.x, m->current.y,
               m->is_alerted ? "Yes" : "No", m->is_alive ? "Yes" : "No");
    }
    printf("========================\n\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <config_file>\n", argv[0]);
        return 1;
    }

    // Initialize mutex
    pthread_mutex_init(&sim_state.simulation_mutex, NULL);
    sim_state.simulation_running = true;

    // Parse configuration
    printf("Parsing configuration file: %s\n", argv[1]);
    parse_config_file(argv[1]);
    
    print_simulation_state();

    // Create hero threads
    for (int i = 0; i < sim_state.hero_count; i++) {
        if (pthread_create(&sim_state.heroes[i].thread, NULL, hero_thread_function, &sim_state.heroes[i]) != 0) { fprintf(stderr, "Error creating hero thread %d\n", i); exit(1); }
    }

    // Create monster threads
    for (int i = 0; i < sim_state.monster_count; i++) {
        pthread_create(&sim_state.monsters[i].thread, NULL, monster_thread_function, &sim_state.monsters[i]);
    }

    // Monitor simulation
    bool all_done = false;
    while (!all_done && sim_state.simulation_running) {
        sleep(2);
        
        pthread_mutex_lock(&sim_state.simulation_mutex);
        
        // Check if all heroes are dead or reached goal
        bool any_hero_active = false;
        for (int i = 0; i < sim_state.hero_count; i++) {
            if (sim_state.heroes[i].is_alive && 
                sim_state.heroes[i].path_index < sim_state.heroes[i].path_length) {
                any_hero_active = true;
                break;
            }
        }
        
        // Check if all monsters are dead
        bool any_monster_alive = false;
        for (int i = 0; i < sim_state.monster_count; i++) {
            if (sim_state.monsters[i].is_alive) {
                any_monster_alive = true;
                break;
            }
        }
        
        if (!any_hero_active || !any_monster_alive) {
            all_done = true;
        }
        
        pthread_mutex_unlock(&sim_state.simulation_mutex);
    }

    // Stop simulation
    sim_state.simulation_running = false;
    sleep(1);

    // Wait for all threads to finish
    for (int i = 0; i < sim_state.hero_count; i++) {
        pthread_join(sim_state.heroes[i].thread, NULL);
    }

    for (int i = 0; i < sim_state.monster_count; i++) {
        pthread_join(sim_state.monsters[i].thread, NULL);
    }

    print_simulation_state();
    
    printf("\n=== Simulation Complete ===\n");
    
    // Check results
    bool any_hero_survived = false;
    for (int i = 0; i < sim_state.hero_count; i++) {
        if (sim_state.heroes[i].is_alive) {
            any_hero_survived = true;
            printf("Hero %d survived!\n", sim_state.heroes[i].id);
        }
    }
    
    if (!any_hero_survived) {
        printf("All heroes died!\n");
    }

    pthread_mutex_destroy(&sim_state.simulation_mutex);
    
    return 0;
}
