#include <GLES2/gl2.h>
#include <emscripten.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "context.h"
#include "timer.h"

#define DIFFICULTY_TIME_MULTIPLIER 0.1f

clock_t last_spawn;
clock_t frame_time;

Circle circles[MAX_CIRCLES];
uint16_t circle_count_;
uint16_t free_indexes_[MAX_CIRCLES];
uint16_t last_index_ = -1;

uint32_t mouseX, mouseY;

uint32_t dodges;
clock_t start_time;
clock_t spawn_time;
bool dead;

void
get_circle(Circle **out){
    if (last_index_ == ((uint16_t) - 1)){
        (*out) = &circles[circle_count_];
        (*out)->index = circle_count_;
    }else{
        (*out) = &circles[free_indexes_[last_index_]];
        (*out)->index = free_indexes_[last_index_];
        last_index_--;
    }
    (*out)->valid = 1;
    circle_count_++;
}

void
remove_circle_l(uint16_t index){
	last_index_++;
    free_indexes_[last_index_] = index;
    circle_count_--;
	circles[index].valid = 0;

    memset(&circles[index], 0, sizeof(Circle));
}

void
die(){
	dead = true;
	memset(circles, 0, MAX_CIRCLES * sizeof(Circle));
	EM_ASM(
		document.getElementById("gameover").style.display = "block";
	);
}

void
game_loop(){
	if(dead) return;
    if (((float) clock()-last_spawn)/((float) CLOCKS_PER_SEC)>=(spawn_time/((float) CLOCKS_PER_SEC)) && circle_count_<MAX_CIRCLES){
        Circle *c = NULL;
        get_circle(&c);
        c->radius = (((float) rand())/((float) RAND_MAX)) * 60 + 10;
		do{
			c->x = (((float) rand())/((float) RAND_MAX)) * (width-c->radius*2);
		}while(fabsf(mouseX - (c->x + c->radius)) < c->radius + 100);

		do{
			c->y = (((float) rand())/((float) RAND_MAX)) * (height-c->radius*2);
		}while(fabsf(mouseY - (c->y + c->radius)) < c->radius + 100);

		GLfloat velX = mouseX - c->x, velY = mouseY - c->y;

		GLfloat norm = hypotf(velX, velY);
		GLfloat dirX = velX/norm, dirY = velY/norm;

		GLfloat speed = (((float) rand())/((float) RAND_MAX)) * 6.0f + 2.0f;
		c->velX = dirX*speed;
		c->velY = dirY*speed;

		clock_t time_add = 1000.f - ((float) (clock()-start_time)/((float) CLOCKS_PER_SEC))*10.f;
		spawn_time = (((float) time_add)/1000.f) * CLOCKS_PER_SEC;
        last_spawn = clock();
	}
	clear_buffer();
    for (int i = 0; i < MAX_CIRCLES; ++i) {
        if (circles[i].valid!=1)continue;

		if(circles[i].x + circles[i].radius*2 < 0 || circles[i].y + circles[i].radius*2 < 0 || circles[i].x > width || circles[i].y > height){
			dodges++;
			remove_circle_l(i);
			continue;
		}

		if(fabsf((circles[i].x+circles[i].radius)-mouseX)<=circles[i].radius && fabsf((circles[i].y+circles[i].radius)-mouseY)<=circles[i].radius){
			//remove_circle_l(i);
			die();
			return;
		}

        circles[i].x += circles[i].velX;
        circles[i].y += circles[i].velY;

		add_circle(&circles[i]);
    }
	EM_ASM({
			document.getElementById("dodges").innerText = "Dodges: " + $0;
			document.getElementById("timer").innerText = $1 + "s";
	}, dodges, (clock()-start_time)/CLOCKS_PER_SEC);
    draw();
}

int main(int argc, char const *argv[]) {
    printf("Loaded\n");

    EM_ASM(
    if (typeof window!="undefined") {
        window.dispatchEvent(new CustomEvent("wasmLoaded"))
    } else {
        global.onWASMLoaded && global.onWASMLoaded()
    }
	);

	srand(clock());

    return 0;
}

int
mouse_move_callback(int type, const EmscriptenMouseEvent *event, void *user_data){
	switch (type) {
		case EMSCRIPTEN_EVENT_MOUSEMOVE: {
			mouseX = event->clientX;
			mouseY = event->clientY;
		}
	}
	return 0;
}

EMSCRIPTEN_KEEPALIVE
void clearContext (void) {
    destroy_context();
    printf("Delete context\n");
}

EMSCRIPTEN_KEEPALIVE
void createContext (int widthIn, int heightIn) {
    width = widthIn;
    height = heightIn;
    create_context();
    printf("Created context\n");
    last_spawn = clock();
	start_time = clock();
	emscripten_set_mousemove_callback("#ctxCanvas", NULL, 0, mouse_move_callback);
	emscripten_set_main_loop(game_loop, 0, false);
}

EMSCRIPTEN_KEEPALIVE
void resize(int widthIn, int heightIn) {
    width = widthIn;
    height = heightIn;
}

EMSCRIPTEN_KEEPALIVE
void undead() {
	dead = false;
	start_time = clock();
	dodges = 0;
}
