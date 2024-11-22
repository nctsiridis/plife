
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL2/SDL.h>

#define WIDTH 800
#define HEIGHT 600
#define MAX_PARTICLES 10000
#define MAX_TYPES 100
#define COLLISION_RADIUS 5.0f
#define COLLISION_STRENGTH 0.5f

typedef struct {
    float x, y;
    float vx, vy;
    float ax, ay; // Added accelerations
    int type;
} Particle;

typedef struct {
    int num_particles;
    Particle *particles;
    int num_types;
    int *quantities;
    float **attraction;
    SDL_Color *colors;
} Simulation;

int read_input_file(const char *filename, int *num_epochs, int **quantities, int *num_types, float ***attraction_matrix);
void initialize_particles(Simulation *sim);
void compute_forces(Simulation *sim);
void update_particles(Simulation *sim);
void render_particles(SDL_Renderer *renderer, Simulation *sim);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s input_file\n", argv[0]);
        return 1;
    }
    // Read input file
    int num_epochs;
    int *quantities;
    int num_types;
    float **attraction_matrix;

    if (!read_input_file(argv[1], &num_epochs, &quantities, &num_types, &attraction_matrix)) {
        printf("Failed to read input file.\n");
        return 1;
    }

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL Init Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("Particle Life Simulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (!win) {
        printf("SDL CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_DestroyWindow(win);
        printf("SDL CreateRenderer Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Initialize simulation
    Simulation sim;
    sim.num_types = num_types;
    sim.quantities = quantities;
    sim.attraction = attraction_matrix;
    initialize_particles(&sim);

    // Simulation loop
    for (int epoch = 0; epoch < num_epochs; ++epoch) {
        // Handle events
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                epoch = num_epochs; // Exit simulation
            }
        }

        // Compute forces
        compute_forces(&sim);

        // Update particles
        update_particles(&sim);

        // Render particles
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Clear screen with black
        SDL_RenderClear(renderer);
        render_particles(renderer, &sim);
        SDL_RenderPresent(renderer);

        SDL_Delay(16); // Delay to control frame rate (approx 60 FPS)
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();

    // Free memory
    for (int i = 0; i < num_types; ++i) {
        free(attraction_matrix[i]);
    }
    free(attraction_matrix);
    free(quantities);
    free(sim.particles);
    free(sim.colors);

    return 0;
}

int read_input_file(const char *filename, int *num_epochs, int **quantities, int *num_types, float ***attraction_matrix) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Cannot open input file %s\n", filename);
        return 0;
    }

    char line[256];
    int state = 0;
    *num_types = 0;
    int max_types = MAX_TYPES;

    *quantities = (int *)malloc(sizeof(int) * max_types);
    memset(*quantities, 0, sizeof(int) * max_types);

    float **attraction = NULL;
    int row = 0;

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#') {
            if (strstr(line, "Epochs")) {
                state = 1;
            } else if (strstr(line, "Quantities")) {
                state = 2;
            } else if (strstr(line, "Attraction")) {
                state = 3;
                row = 0;
            } else {
                state = 0;
            }
            continue;
        }

        if (state == 1) {
            // Read number of epochs
            sscanf(line, "%d", num_epochs);
            state = 0;
        } else if (state == 2) {
            // Read quantities
            char *token = strtok(line, " ");
            int idx = 0;
            while (token) {
                if (idx >= max_types) {
                    max_types *= 2;
                    *quantities = (int *)realloc(*quantities, sizeof(int) * max_types);
                }
                (*quantities)[idx++] = atoi(token);
                token = strtok(NULL, " ");
            }
            *num_types = idx;
            state = 0;
        } else if (state == 3) {
            // Read attraction matrix
            if (!attraction) {
                // Allocate attraction matrix
                attraction = (float **)malloc(sizeof(float *) * (*num_types));
                for (int i = 0; i < *num_types; ++i) {
                    attraction[i] = (float *)malloc(sizeof(float) * (*num_types));
                }
            }
            char *token = strtok(line, " ");
            int idx = 0;
            while (token) {
                attraction[row][idx++] = atof(token);
                token = strtok(NULL, " ");
            }
            row++;
            if (row >= *num_types) {
                state = 0;
            }
        }
    }

    fclose(file);

    *attraction_matrix = attraction;

    return 1;
}

void initialize_particles(Simulation *sim) {
    // Calculate total number of particles
    int total_particles = 0;
    for (int i = 0; i < sim->num_types; ++i) {
        total_particles += sim->quantities[i];
    }
    sim->num_particles = total_particles;
    sim->particles = (Particle *)malloc(sizeof(Particle) * total_particles);

    // Initialize particles
    int idx = 0;
    for (int type = 0; type < sim->num_types; ++type) {
        int quantity = sim->quantities[type];
        for (int i = 0; i < quantity; ++i) {
            sim->particles[idx].x = rand() % WIDTH;
            sim->particles[idx].y = rand() % HEIGHT;
            sim->particles[idx].vx = 0;
            sim->particles[idx].vy = 0;
            sim->particles[idx].ax = 0; // Initialize acceleration
            sim->particles[idx].ay = 0; // Initialize acceleration
            sim->particles[idx].type = type;
            idx++;
        }
    }

    // Initialize colors
    sim->colors = (SDL_Color *)malloc(sizeof(SDL_Color) * sim->num_types);
    for (int type = 0; type < sim->num_types; ++type) {
        float attraction_sum = 0.0f;
        for (int j = 0; j < sim->num_types; ++j) {
            attraction_sum += sim->attraction[type][j];
        }
        // Map attraction_sum to a color
        Uint8 r = (Uint8)((attraction_sum + type * 50) * 10) % 256;
        Uint8 g = (Uint8)((attraction_sum + type * 80) * 20) % 256;
        Uint8 b = (Uint8)((attraction_sum + type * 110) * 30) % 256;
        sim->colors[type].r = r;
        sim->colors[type].g = g;
        sim->colors[type].b = b;
        sim->colors[type].a = 255;
    }
}

void compute_forces(Simulation *sim) {
    int n = sim->num_particles;
    Particle *particles = sim->particles;
    float **attraction = sim->attraction;

    // Initialize accelerations to zero for this iteration
    for (int i = 0; i < n; ++i) {
        particles[i].ax = 0;
        particles[i].ay = 0;
    }

    // Compute forces between particles
    for (int i = 0; i < n; ++i) {
        Particle *p1 = &particles[i];
        for (int j = i + 1; j < n; ++j) {
            Particle *p2 = &particles[j];

            float dx = p2->x - p1->x;
            float dy = p2->y - p1->y;

            // Apply periodic boundary conditions
            if (dx > WIDTH / 2) dx -= WIDTH;
            if (dx < -WIDTH / 2) dx += WIDTH;
            if (dy > HEIGHT / 2) dy -= HEIGHT;
            if (dy < -HEIGHT / 2) dy += HEIGHT;

            float distance = sqrtf(dx * dx + dy * dy) + 0.1f; // Avoid division by zero

            // Compute attraction force
            float force = attraction[p1->type][p2->type] / distance;
            float fx = force * dx / distance;
            float fy = force * dy / distance;

            // Update accelerations (symmetrical)
            p1->ax += fx;
            p1->ay += fy;
            p2->ax -= fx;
            p2->ay -= fy;

            // Collision handling
            if (distance < COLLISION_RADIUS) {
                float collision_force = COLLISION_STRENGTH * (COLLISION_RADIUS - distance);
                float fx_collision = collision_force * dx / distance;
                float fy_collision = collision_force * dy / distance;

                // Apply collision force
                p1->ax -= fx_collision;
                p1->ay -= fy_collision;
                p2->ax += fx_collision;
                p2->ay += fy_collision;
            }
        }
    }
}

void update_particles(Simulation *sim) {
    int n = sim->num_particles;
    Particle *particles = sim->particles;

    for (int i = 0; i < n; ++i) {
        // Update velocities with accelerations
        particles[i].vx += particles[i].ax;
        particles[i].vy += particles[i].ay;

        // Apply damping to velocities
        float damping = 0.99f;
        particles[i].vx *= damping;
        particles[i].vy *= damping;

        // Update positions with velocities
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;

        // Apply periodic boundary conditions
        if (particles[i].x < 0) particles[i].x += WIDTH;
        if (particles[i].x >= WIDTH) particles[i].x -= WIDTH;
        if (particles[i].y < 0) particles[i].y += HEIGHT;
        if (particles[i].y >= HEIGHT) particles[i].y -= HEIGHT;
    }
}

void render_particles(SDL_Renderer *renderer, Simulation *sim) {
    int n = sim->num_particles;
    Particle *particles = sim->particles;
    SDL_Color *colors = sim->colors;

    for (int i = 0; i < n; ++i) {
        Particle *p = &particles[i];
        SDL_SetRenderDrawColor(renderer, colors[p->type].r, colors[p->type].g, colors[p->type].b, 255);
        SDL_Rect rect = {(int)p->x, (int)p->y, 3, 3};
        SDL_RenderFillRect(renderer, &rect);
    }
}
