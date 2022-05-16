//
// Created by quartzy on 4/13/22.
//

#ifndef BALLS_WEBASM_CONTEXT_H
#define BALLS_WEBASM_CONTEXT_H

#include <stdint.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include "emscripten/html5_webgl.h"

#define MAX_CIRCLES 300
#define CIRCLE_FLOAT_SIZE 84

typedef enum CircleType{
ENEMY,
POWERUP_INVINCIBILITY,
POWERUP_CLEAR,
POWERUP_REMOVER,
TYPE_COUNT
} CircleType;

typedef enum Animation{
NONE,
EXPLOSION
} Animation;

typedef struct Radius{
	GLfloat x, y, radius;
	uint8_t valid;
	uint16_t index;
} Radius;

typedef struct Circle{
    GLfloat x, y, radius, border;
	GLfloat r, g, b, a;
	GLfloat br, bg, bb, ba;
    GLfloat velX, velY;
    uint16_t index;
	CircleType type;
	uint16_t clearer_index;
	Animation animation;
	clock_t animation_start;
    uint8_t valid; // 1 if data is valid
} Circle;

extern uint32_t width, height;
extern uint32_t mouseX, mouseY;

extern GLuint programId;
extern GLuint vertexShader;
extern GLuint fragmentShader;

extern int context;

void draw();

void create_context();

void destroy_context();

void add_circle(Circle *circle);

void clear_buffer();

#endif //BALLS_WEBASM_CONTEXT_H
