#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define MAX_TYPES 10
#define MAX_PARTICLES 10000

typedef struct {
    float x, y;
    float vx, vy;
    int type;
} Particle;

int num_types;
int quantities[MAX_TYPES];
float attraction[MAX_TYPES][MAX_TYPES];
float viscosity;
float repulsion_strength;
float interaction_radius;
int epochs;

Particle particles[MAX_PARTICLES];
int total_particles = 0;

void read_input(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Could not open input file.\n");
        exit(EXIT_FAILURE);
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "#Epochs", 7) == 0) {
            fgets(line, sizeof(line), file);
            epochs = atoi(line);
        } else if (strncmp(line, "#Quantities", 11) == 0) {
            fgets(line, sizeof(line), file);
            char *token = strtok(line, " ");
            num_types = 0;
            while (token && num_types < MAX_TYPES) {
                quantities[num_types++] = atoi(token);
                token = strtok(NULL, " ");
            }
        } else if (strncmp(line, "#Attraction", 11) == 0) {
            for (int i = 0; i < num_types; i++) {
                fgets(line, sizeof(line), file);
                char *token = strtok(line, " ");
                for (int j = 0; j < num_types; j++) {
                    attraction[i][j] = atof(token);
                    token = strtok(NULL, " ");
                }
            }
        } else if (strncmp(line, "#Viscosity", 10) == 0) {
            fgets(line, sizeof(line), file);
            viscosity = atof(line);
        } else if (strncmp(line, "#RepulsionStrength", 18) == 0) {
            fgets(line, sizeof(line), file);
            repulsion_strength = atof(line);
        } else if (strncmp(line, "#Radius", 7) == 0) {
            fgets(line, sizeof(line), file);
            interaction_radius = atof(line);
        }
    }

    fclose(file);
}

void initialize_particles() {
    srand(SDL_GetTicks());
    total_particles = 0;
    for (int t = 0; t < num_types; t++) {
        for (int i = 0; i < quantities[t]; i++) {
            particles[total_particles].x = rand() % WINDOW_WIDTH;
            particles[total_particles].y = rand() % WINDOW_HEIGHT;
            particles[total_particles].vx = 0.0f;
            particles[total_particles].vy = 0.0f;
            particles[total_particles].type = t;
            total_particles++;
        }
    }
}

void update_particles() {
    float radius2 = interaction_radius * interaction_radius;
    float epsilon = 0.01f; // Small value to prevent singularity
    float max_force = 10.0f; // Maximum force cap
    float max_velocity = 5.0f; // Maximum velocity cap

    for (int i = 0; i < total_particles; i++) {
        Particle *p1 = &particles[i];
        float fx = 0.0f, fy = 0.0f;

        for (int j = 0; j < total_particles; j++) {
            if (i == j) continue;

            Particle *p2 = &particles[j];
            float dx = p2->x - p1->x;
            float dy = p2->y - p1->y;

            // Handle periodic boundary conditions
            if (dx > WINDOW_WIDTH / 2) dx -= WINDOW_WIDTH;
            if (dx < -WINDOW_WIDTH / 2) dx += WINDOW_WIDTH;
            if (dy > WINDOW_HEIGHT / 2) dy -= WINDOW_HEIGHT;
            if (dy < -WINDOW_HEIGHT / 2) dy += WINDOW_HEIGHT;

            float dist2 = dx * dx + dy * dy;
            if (dist2 < radius2 && dist2 > 0.0001f) {
                // Add epsilon to prevent division by zero
                float dist2_softened = dist2 + epsilon;
                float dist = sqrtf(dist2_softened);

                // Compute force with softened distance
                float force = (attraction[p1->type][p2->type]) / dist - (repulsion_strength / dist2_softened);

                // Cap the force magnitude
                if (force > max_force) force = max_force;
                if (force < -max_force) force = -max_force;

                fx += force * dx / dist;
                fy += force * dy / dist;
            }
        }

        // Apply viscosity
        p1->vx = (p1->vx + fx) * (1.0f - viscosity);
        p1->vy = (p1->vy + fy) * (1.0f - viscosity);

        // Clamp velocity
        float speed = sqrtf(p1->vx * p1->vx + p1->vy * p1->vy);
        if (speed > max_velocity) {
            p1->vx = (p1->vx / speed) * max_velocity;
            p1->vy = (p1->vy / speed) * max_velocity;
        }
    }

    // Update positions
    for (int i = 0; i < total_particles; i++) {
        Particle *p = &particles[i];
        p->x += p->vx;
        p->y += p->vy;

        // Wrap around screen edges
        if (p->x < 0) p->x += WINDOW_WIDTH;
        if (p->x >= WINDOW_WIDTH) p->x -= WINDOW_WIDTH;
        if (p->y < 0) p->y += WINDOW_HEIGHT;
        if (p->y >= WINDOW_HEIGHT) p->y -= WINDOW_HEIGHT;
    }
}

void render_particles(SDL_Renderer *renderer) {
    // Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
    SDL_RenderClear(renderer);

    // Define colors for each particle type
    SDL_Color colors[MAX_TYPES] = {
        {255, 0, 0, 255},     // Red
        {0, 255, 0, 255},     // Green
        {0, 0, 255, 255},     // Blue
        {255, 255, 0, 255},   // Yellow
        {255, 0, 255, 255},   // Magenta
        {0, 255, 255, 255},   // Cyan
        {255, 165, 0, 255},   // Orange
        {128, 0, 128, 255},   // Purple
        {0, 128, 128, 255},   // Teal
        {128, 128, 0, 255}    // Olive
    };

    for (int i = 0; i < total_particles; i++) {
        Particle *p = &particles[i];
        SDL_SetRenderDrawColor(renderer, colors[p->type % MAX_TYPES].r, colors[p->type % MAX_TYPES].g, colors[p->type % MAX_TYPES].b, 255);
        SDL_Rect rect = {(int)p->x, (int)p->y, 2, 2};
        SDL_RenderFillRect(renderer, &rect);
    }

    SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s input_file.txt\n", argv[0]);
        return EXIT_FAILURE;
    }

    read_input(argv[1]);
    initialize_particles();

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Could not initialize SDL2: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Window *window = SDL_CreateWindow("Particle Life Simulation",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT,
                                          SDL_WINDOW_SHOWN);

    if (!window) {
        fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!renderer) {
        fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    int running = 1;
    SDL_Event event;
    int frame = 0;

    // Simulation substeps for better stability
    int substeps = 4;
    int substep_delay = 4; // 16 ms / 4 substeps

    while (running && frame < epochs) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = 0;
        }

        // Perform substeps
        for (int s = 0; s < substeps; s++) {
            update_particles();
        }

        render_particles(renderer);

        SDL_Delay(substep_delay); // Adjusted delay for substeps
        frame++;
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
