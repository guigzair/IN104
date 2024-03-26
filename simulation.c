#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h> 

#define PI 3.1415926
#define IMAGE_WIDTH 1200
#define IMAGE_HEIGHT 800
#define RADIUS 8
#define NUM_BODIES 300
#define MAX_VELOCITY 15
#define DT 0.1
#define R_repulsion 15
#define R_orientation 64
#define R_attraction 100

#define MAX_TURNING_RATE 40
#define ANGLE_PERCEPTION 200

// Simulation
typedef  struct bodies{
    double x, y;    // Position
    double vx, vy;  // Velocity
    double angle;
} Body;

void update_position(Body *body) {
    body->x += body->vx * DT;
    body->y += body->vy * DT;
}

void update_angle(Body *body) {
    double angle = atan2(body->vy, body->vx) * (double)180/(double)PI;
    body->angle = angle;
}

double generate_normal(double mean, double stddev) {
    // Box-Muller method
    double u1 = rand()/(RAND_MAX+1.0);
    double u2 = rand()/(RAND_MAX+1.0);

    double z0 = sqrt(-2.0 * log(u1)) * cos(2.0 * PI * u2);
    return mean + stddev * z0;
}

void wiggle(Body *body) {
    double wiggle = generate_normal(0., 5.5);
    body -> angle = body -> angle + wiggle;
    double V = sqrt(body->vx * body->vx + body->vy * body->vy);
    body->vx = cos((body -> angle) * (double)PI/(double)180) * V;
    body->vy = sin((body -> angle) * (double)PI/(double)180) * V;
}

void handle_collision(Body *body1, Body *body2) {
    double dx = body2->x - body1->x;
    double dy = body2->y - body1->y;
    double distance = sqrt(dx * dx + dy * dy);
    double overlap = 2.0 * RADIUS - distance;
    if (overlap > 0) {
        // Calculate normal and tangential components
        double nx = dx / distance;
        double ny = dy / distance;
        double v1x = body1->vx;
        double v1y = body1->vy;
        double v2x = body2->vx;
        double v2y = body2->vy;


        // Update particle velocities
        body1->vx = v2x;
        body1->vy = v2y;
        body2->vx = v1x;
        body2->vy = v1y;
        body1->angle = body2->angle;
        body2->angle = body1->angle;

        // Separate particles to avoid overlap
        double move_x = 0.5 * overlap * nx;
        double move_y = 0.5 * overlap * ny;
        body1->x -= move_x;
        body1->y -= move_y;
        body2->x += move_x;
        body2->y += move_y;
    }
}

void handle_wall_periodic(Body *body) {
    if (body->x > IMAGE_WIDTH){
        body->x -= IMAGE_WIDTH;
    } else if (body->x < 0) {
        body->x += IMAGE_WIDTH;
    }
    if (body->y > IMAGE_HEIGHT){
        body->y -= IMAGE_HEIGHT;
    } else if (body->y < 0) {
        body->y += IMAGE_HEIGHT;
    }
}

void handle_wall(Body *body) {
    if (body->x > IMAGE_WIDTH){
        body->vx = - body->vx;
    } else if (body->x < 0) {
        body->vx = - body->vx;
    }
    if (body->y > IMAGE_HEIGHT){
        body->vy = - body->vy;
    } else if (body->y < 0) {
        body->vy = - body->vy;
    }
}

double angle_difference(double angle1, double angle2) {
    double diff = fmod((angle1 - angle2 + 180.0), 360.0) - 180.0;
    return (diff < -180.0) ? diff + 360.0 : diff;
}
// Window
void render(SDL_Renderer *renderer, Body *bodies, SDL_Texture **texture) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    for (int i = 0; i < NUM_BODIES; ++i) {
        // SDL_Rect rect = { (int)bodies[i].x - 5, (int)bodies[i].y - 5, 10, 10 };
        // SDL_RenderFillRect(renderer, &rect);
        SDL_Rect destRect = {(int)bodies[i].x - 5, (int)bodies[i].y - 5, RADIUS * 2, RADIUS * 2}; // Adjust the dimensions as needed
        double angle = atan2((double)bodies[i].vy, (double)bodies[i].vx) * 180/PI; 
        // SDL_RenderCopy(renderer, *texture, NULL, &destRect);
        SDL_RenderCopyEx(renderer, *texture, NULL, &destRect, angle, NULL, SDL_FLIP_NONE);
    }

    SDL_RenderPresent(renderer);
}

void loadTexture(SDL_Renderer *renderer, SDL_Texture **texture) {
    SDL_Surface *surface = IMG_Load( "/mnt/c/Users/guill/workspace/IN104/fish_bitmap.png");
    if (surface == NULL) {
        fprintf(stderr, "Failed to load image: %s\n", IMG_GetError());
        SDL_Quit();
        exit(1);
    }
    *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (*texture == NULL) {
        fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    if (!((IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) & IMG_INIT_PNG) | IMG_INIT_JPG)) {
        printf("SDL_image could not initialize! SDL_Image_Error: %s\n", IMG_GetError());
    }

    SDL_Window *window = SDL_CreateWindow("N-Body Simulation", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, IMAGE_WIDTH, IMAGE_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        fprintf(stderr, "Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_Texture *texture;
    loadTexture(renderer, &texture); // Replace "particle.png" with your image file

    Body *bodies;

    bodies = (Body*) malloc(NUM_BODIES * sizeof(Body));

    // Use current time as seed for random generator 
    srand(time(0)); 
    for (int i = 0; i < NUM_BODIES; ++i) {
        bodies[i].x = rand() * ((float)IMAGE_WIDTH) / (float)RAND_MAX;
        bodies[i].y = rand() * ((float)IMAGE_HEIGHT) / (float)RAND_MAX;
        // bodies[i].vx = (rand() / (float)RAND_MAX - (float)0.5 ) * ((float)MAX_VELOCITY);
        // bodies[i].vy = (rand() / (float)RAND_MAX - (float)0.5 ) * ((float)MAX_VELOCITY);
        bodies[i].angle = rand() * ((float)360) / (float)RAND_MAX;
        bodies[i].vx = cos(bodies[i].angle * (double)PI/(double)180) * MAX_VELOCITY;
        bodies[i].vy = sin(bodies[i].angle * (double)PI/(double)180) * MAX_VELOCITY;
        // double angle = atan2((double)bodies[i].vy, (double)bodies[i].vx) * 180/PI;
        // bodies[i].angle = angle;
    }

    SDL_Event event;
    int quit = 0;
    while (!quit) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            }
        }
        

        // Update positions
        for (int i = 0; i < NUM_BODIES; ++i) {
            double Dr[2] = {0,0}; // for repulsion
            double Do[2] = {0,0}; // for orientation
            double Da[2] = {0,0}; // for attraction
            for (int j = 0; j < NUM_BODIES; ++j) {
                if (i != j) {
                    double dx = bodies[j].x - bodies[i].x;
                    double dy = bodies[j].y - bodies[i].y;
                    double distance = sqrt(dx * dx + dy * dy);
                    // Calculate normal and tangential components
                    double nx = dx / distance;
                    double ny = dy / distance;
                    double angle_perception = atan2(ny, nx) * (double)180/(double)PI;
                    if (fabs(angle_difference(bodies[i].angle, angle_perception)) < (double)ANGLE_PERCEPTION / 2 ) {
                        if (distance < R_repulsion) {
                            Dr[0] += dx/distance;
                            Dr[1] += dy/distance;
                        } 
                        if ((distance <= R_orientation) && (distance > R_repulsion)) {
                            double V = sqrt(bodies[j].vx * bodies[j].vx + bodies[j].vy * bodies[j].vy);
                            Do[0] -= bodies[j].vx/ V;
                            Do[1] -= bodies[j].vy/ V;
                        }
                        if ((distance <= R_attraction) && (distance > R_orientation)) {
                            Da[0] -= dx/distance;
                            Da[1] -= dy/distance;
                        }
                    }

                }
            }
            // Mise à jour de svitesses en fonction des regles mises en places
            if (Dr[0] != 0 || Dr[1] != 0) {
                
                double new_angle = atan2(Dr[1], Dr[0]) * (double)180/(double)PI;
                double* ptheta, theta;
                theta = 0;
                ptheta = &theta;
                if  (fabs(angle_difference(bodies[i].angle, new_angle)) < (double)MAX_TURNING_RATE * (double)DT ) {
                    *ptheta = new_angle * (double)PI/(double)180;
                } else {
                    if (angle_difference(bodies[i].angle, new_angle) < 0) {
                        *ptheta = (bodies[i].angle - (double)MAX_TURNING_RATE * (double)DT) * (double)PI/(double)180;
                    } else {
                        *ptheta = (bodies[i].angle + (double)MAX_TURNING_RATE * (double)DT) * (double)PI/(double)180;
                    }
                    // printf("angle diff %f \n", theta);

                }
                double V = sqrt(bodies[i].vx * bodies[i].vx + bodies[i].vy * bodies[i].vy);
                bodies[i].vx = cos(theta) * V;
                bodies[i].vy = sin(theta) * V;
            } else {
                double force[2] = {0,0};
                if (Do[0] + Da[0] != 0 || Do[1] + Da[1] != 0){
                    if (Do[0] == 0 && Do[1] == 0) {
                        force[0] =  Da[0];
                        force[1] =  Da[1];
                    } else if (Da[0] == 0 && Da[1] == 0) {
                        force[0] =  Do[0];
                        force[1] =  Do[1];
                    } else {
                        force[0] = 0.5 * (Do[0] + Da[0]);
                        force[1] = 0.5 * (Do[1] + Da[1]);
                    }
                    
                    double new_angle = atan2(force[1], force[0])* (double)180/(double)PI;
                    double* ptheta, theta;
                    theta = 0;
                    ptheta = &theta;
                    if  (fabs(angle_difference(bodies[i].angle, new_angle)) < (double)MAX_TURNING_RATE * (double)DT ) {
                        *ptheta = new_angle * (double)PI/(double)180;
                        // bodies[i].vx = force[0];
                        // bodies[i].vy = force[1];
                    } else {
                        if (angle_difference(bodies[i].angle, new_angle) < 0) {
                            *ptheta = (bodies[i].angle - (double)MAX_TURNING_RATE * (double)DT) * (double)PI/(double)180;
                        } else {
                            *ptheta = (bodies[i].angle + (double)MAX_TURNING_RATE * (double)DT) * (double)PI/(double)180;
                        }
                    }
                    double V = sqrt(bodies[i].vx * bodies[i].vx + bodies[i].vy * bodies[i].vy);
                    bodies[i].vx = cos(theta) * V;
                    bodies[i].vy = sin(theta) * V;
                }
            }

            update_position(&bodies[i]);
            handle_wall_periodic(&bodies[i]);
            for (int j = 0; j < NUM_BODIES; ++j) {
                if (i != j) {
                    handle_collision(&bodies[i], &bodies[j]);
                }
            }
            update_angle(&bodies[i]);
            wiggle(&bodies[i]);
        }


        // Render the updated positions
        render(renderer, bodies, &texture);

        // Delay to control the frame rate
        // SDL_Delay(1);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
