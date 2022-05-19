//
// Created by quartzy on 4/13/22.
//

#include <GLES2/gl2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "context.h"

uint32_t width, height;

GLuint programId;
GLuint vertexShader;
GLuint fragmentShader;

int context;

const char *vertex_source =

#include "shaders/vertex.glsl"
        ;

const char *fragment_source =

#include "shaders/fragment.glsl"
        ;

GLfloat proj_matrix[16] = {-999.0f, 0.0f, 0.0f, 0.0f,
                           0.0f, -999.0f, 0.0f, 0.0f,
                           0.0f, 0.0f, 0.0f, 0.0f,
                           -1.0f, 1.0f, 0.0f, 1.0f};

GLint heightLoc;
GLint proj_loc;

GLfloat vertices[MAX_CIRCLES * CIRCLE_FLOAT_SIZE];
uint32_t circle_count;

GLuint vertexObject;

GLuint compile_shader(GLenum type, const char *source) {
    GLuint shader = glCreateShader(type);

    if (!shader) {
        return 0;
    }

    // Assign and compile the source to the shader object
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    // Check if there were errors
    int infoLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

    if (infoLen > 1) {

        char infoLog[infoLen];

        // And print them out
        glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
        printf("Shader error: %s\n", infoLog);
    }

    return shader;
}

void
create_context() {
    // Context configurations
    EmscriptenWebGLContextAttributes attrs;
    attrs.explicitSwapControl = 0;
    attrs.depth = 1;
    attrs.stencil = 1;
    attrs.antialias = 1;
    attrs.majorVersion = 3;
    attrs.minorVersion = 0;

    context = emscripten_webgl_create_context("#ctxCanvas", &attrs);
    emscripten_webgl_make_context_current(context);

    // Compile shaders
    fragmentShader = compile_shader(GL_FRAGMENT_SHADER, fragment_source);
    vertexShader = compile_shader(GL_VERTEX_SHADER, vertex_source);
    // fragmentShader = 1;
    // vertexShader = 0;

    // Build program
    programId = glCreateProgram();

    glAttachShader(programId, vertexShader);
    glAttachShader(programId, fragmentShader);

    glBindAttribLocation(programId, 0, "position");
    glBindAttribLocation(programId, 1, "radius");
    glBindAttribLocation(programId, 2, "center");
	glBindAttribLocation(programId, 3, "color");
	glBindAttribLocation(programId, 4, "border");
	glBindAttribLocation(programId, 5, "border_color");

    glLinkProgram(programId);
    glValidateProgram(programId);

    // Check if there were errors
    int infoLen = 0;
    glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLen);

    if (infoLen > 1) {

        char infoLog[infoLen];

        // And print them out
        glGetProgramInfoLog(programId, infoLen, NULL, infoLog);
        fprintf(stdout, "Program error: %s\n", infoLog);
        return;
    }

    glGenBuffers(1, &vertexObject);
	glUseProgram(programId);
    proj_loc = glGetUniformLocation(programId, "proj");
    heightLoc = glGetUniformLocation(programId, "screenHeight");
}

void
destroy_context() {
    emscripten_webgl_destroy_context(context);
}

void
draw() {
    // Make the context current and use the program
    emscripten_webgl_make_context_current(context);
    glUseProgram(programId);

    proj_matrix[0] = 2.0f / ((float) width);
    proj_matrix[5] = -2.0f / ((float) height);


    glBindBuffer(GL_ARRAY_BUFFER, vertexObject);
    glBufferData(GL_ARRAY_BUFFER, circle_count * CIRCLE_FLOAT_SIZE * sizeof(*vertices), vertices, GL_DYNAMIC_DRAW);


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(200.f/255.f, 200.f/255.f, 200.f/255.f, 1.0);

    // Set the viewport
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    // Load and enable the vertex position and texture coordinates
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), 0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (const void *) (2 * sizeof(GLfloat)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (const void *) (3 * sizeof(GLfloat)));
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (const void *) (5 * sizeof(GLfloat)));
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (const void *) (9 * sizeof(GLfloat)));
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (const void *) (10 * sizeof(GLfloat)));

    glBindBuffer(GL_ARRAY_BUFFER, vertexObject);
    glUniformMatrix4fv(proj_loc, 1, GL_FALSE, proj_matrix);
    glUniform1f(heightLoc, (GLfloat) height);
    // Draw
    glDrawArrays(GL_TRIANGLES, 0, circle_count * 6);
}

void
clear_buffer(){
	memset(vertices, 0, MAX_CIRCLES * CIRCLE_FLOAT_SIZE);
	circle_count = 0;
}

void
add_circle(Circle *circle) {
    size_t vertex_index;
    vertex_index = circle_count * CIRCLE_FLOAT_SIZE;
    circle->index = circle_count;
    circle_count++;

    vertices[vertex_index++] = circle->x - circle->radius;
    vertices[vertex_index++] = circle->y - circle->radius;
    vertices[vertex_index++] = circle->radius;
    vertices[vertex_index++] = circle->x;
    vertices[vertex_index++] = circle->y;
	vertices[vertex_index++] = circle->r;
	vertices[vertex_index++] = circle->g;
	vertices[vertex_index++] = circle->b;
	vertices[vertex_index++] = circle->a;
	vertices[vertex_index++] = circle->border;
	vertices[vertex_index++] = circle->br;
	vertices[vertex_index++] = circle->bg;
	vertices[vertex_index++] = circle->bb;
	vertices[vertex_index++] = circle->ba;

    vertices[vertex_index++] = circle->x - circle->radius;
    vertices[vertex_index++] = circle->y + circle->radius;
    vertices[vertex_index++] = circle->radius;
    vertices[vertex_index++] = circle->x;
    vertices[vertex_index++] = circle->y;
	vertices[vertex_index++] = circle->r;
	vertices[vertex_index++] = circle->g;
	vertices[vertex_index++] = circle->b;
	vertices[vertex_index++] = circle->a;
	vertices[vertex_index++] = circle->border;
	vertices[vertex_index++] = circle->br;
	vertices[vertex_index++] = circle->bg;
	vertices[vertex_index++] = circle->bb;
	vertices[vertex_index++] = circle->ba;

    vertices[vertex_index++] = circle->x + circle->radius;
    vertices[vertex_index++] = circle->y + circle->radius;
    vertices[vertex_index++] = circle->radius;
    vertices[vertex_index++] = circle->x;
    vertices[vertex_index++] = circle->y;
	vertices[vertex_index++] = circle->r;
	vertices[vertex_index++] = circle->g;
	vertices[vertex_index++] = circle->b;
	vertices[vertex_index++] = circle->a;
	vertices[vertex_index++] = circle->border;
	vertices[vertex_index++] = circle->br;
	vertices[vertex_index++] = circle->bg;
	vertices[vertex_index++] = circle->bb;
	vertices[vertex_index++] = circle->ba;

    vertices[vertex_index++] = circle->x - circle->radius;
    vertices[vertex_index++] = circle->y - circle->radius;
    vertices[vertex_index++] = circle->radius;
    vertices[vertex_index++] = circle->x;
    vertices[vertex_index++] = circle->y;
	vertices[vertex_index++] = circle->r;
	vertices[vertex_index++] = circle->g;
	vertices[vertex_index++] = circle->b;
	vertices[vertex_index++] = circle->a;
	vertices[vertex_index++] = circle->border;
	vertices[vertex_index++] = circle->br;
	vertices[vertex_index++] = circle->bg;
	vertices[vertex_index++] = circle->bb;
	vertices[vertex_index++] = circle->ba;

    vertices[vertex_index++] = circle->x + circle->radius;
    vertices[vertex_index++] = circle->y + circle->radius;
    vertices[vertex_index++] = circle->radius;
    vertices[vertex_index++] = circle->x;
    vertices[vertex_index++] = circle->y;
	vertices[vertex_index++] = circle->r;
	vertices[vertex_index++] = circle->g;
	vertices[vertex_index++] = circle->b;
	vertices[vertex_index++] = circle->a;
	vertices[vertex_index++] = circle->border;
	vertices[vertex_index++] = circle->br;
	vertices[vertex_index++] = circle->bg;
	vertices[vertex_index++] = circle->bb;
	vertices[vertex_index++] = circle->ba;

    vertices[vertex_index++] = circle->x + circle->radius;
    vertices[vertex_index++] = circle->y - circle->radius;
    vertices[vertex_index++] = circle->radius;
    vertices[vertex_index++] = circle->x;
    vertices[vertex_index++] = circle->y;
	vertices[vertex_index++] = circle->r;
	vertices[vertex_index++] = circle->g;
	vertices[vertex_index++] = circle->b;
	vertices[vertex_index++] = circle->a;
	vertices[vertex_index++] = circle->border;
	vertices[vertex_index++] = circle->br;
	vertices[vertex_index++] = circle->bg;
	vertices[vertex_index++] = circle->bb;
	vertices[vertex_index++] = circle->ba;
}
