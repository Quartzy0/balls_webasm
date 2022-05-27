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
#include "vector.h"


//0.01f
#define POWERUP_CHANCE 0.02f
#define POWERUP_DURATION 5.f
#define START_DELAY 0.5f

clock_t last_spawn;
clock_t frame_time;

VECTOR_NEW(Circle, circles, MAX_CIRCLES);
VECTOR_NEW(Circle*, clearers, MAX_CIRCLES);

uint32_t mouseX, mouseY;

uint32_t dodges;
clock_t start_time;
clock_t spawn_time;
bool dead = true;
bool started = false;
CircleType active_powerup;
clock_t powerup_start_time;
char *powerup_names[] = {NULL, "Invincibility", NULL, "Remover"};

void
explode(Circle *c){
	c->a = 0;
	c->br = 1.f;
	c->bg = 0.f;
	c->bb = 0.f;
	c->ba = 1.f;
	c->border = 7.f;
	c->animation = EXPLOSION;
	c->animation_start = clock();
}

void
die(){
	dead = true;
	VECTOR_CLEAR(circles);
	VECTOR_CLEAR(clearers);
	active_powerup = ENEMY;
	EM_ASM({
		let highscore = window.localStorage.getItem("highscore");
		if(!highscore || highscore<$0){
			highscore = $0;
			window.localStorage.setItem("highscore", $0);
		}
		document.getElementById("highscore").innerText = "High score: " + highscore;
		document.getElementById("gameover").style.display = "block";
	}, dodges);
}

void
game_loop(){
	if(dead) return;
	if(!started){
		started = ((float) ((float) ((float) clock())-start_time) / CLOCKS_PER_SEC) >= START_DELAY;
		if(started){
			start_time = clock();
			printf("Started\n");
		}
	}
	const float delta_time = ((float) (clock()-frame_time))/((float) CLOCKS_PER_SEC);
	if(active_powerup!=ENEMY && clock()-powerup_start_time>=POWERUP_DURATION*CLOCKS_PER_SEC){
		active_powerup = ENEMY;
	}
    if (clock()-last_spawn>=spawn_time && VECTOR_COUNT(circles)<MAX_CIRCLES && started){
        Circle *c = NULL;
        VECTOR_ADD(circles, c, c->index);
		c->valid = 1;

		float powerup_chance = (rand()/((float) RAND_MAX));
		if (powerup_chance<=POWERUP_CHANCE) {
			float type_chance = rand()/((float) RAND_MAX);
			CircleType type = type_chance*(TYPE_COUNT-1) + 1;
			c->type = type;
			switch (type) {
				case POWERUP_CLEAR: {
					c->r = 0.f;
					c->g = 0.f;
					c->b = 1.f;
					break;
				}
				case POWERUP_INVINCIBILITY: {
					c->r = 1.f;
					c->g = 0.f;
					c->b = 1.f;
					break;
				}
				case POWERUP_REMOVER: {
					c->r = 1.f;
					c->g = 0.f;
					c->b = 0.f;
					break;
				}
			}

		}else{
			c->type = ENEMY;
			c->r = 1.f;
			c->g = 1.f;
			c->b = 1.f;
		}
		c->border = 3.f;
		c->ba = 1.f;
		c->a = 1.f;

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

		GLfloat speed = (((float) rand())/((float) RAND_MAX)) * 400.0f + 150.0f;
		c->velX = dirX*speed;
		c->velY = dirY*speed;

		float secondsSinceStart = ((float) clock()-start_time)/((float) CLOCKS_PER_SEC);
		//float multiplier = fmaxf(powf(1 - (secondsSinceStart/MAX_DIFFICULTY), 5), 0.f);
		float multiplier = expf(-powf(secondsSinceStart, 0.7f));
		spawn_time = multiplier * CLOCKS_PER_SEC;
        last_spawn = clock();
	}
	clear_buffer();
    for (int i = 0; i < MAX_CIRCLES; ++i) {
        if (circles[i].valid!=1)continue;

		switch (circles[i].animation) {
			case EXPLOSION: {
				circles[i].radius += 10;
				if(circles[i].radius>=400.f){
					VECTOR_REMOVE(clearers, circles[i].clearer_index);
					goto circle_remove;
				}
				goto loop_end_add;
			}
			case NONE: {
				break;
			}
		}

		if(circles[i].type!=POWERUP_CLEAR){
			for (uint16_t j = 0; j < MAX_CIRCLES; j++) {
				if(!clearers[j]) continue;

				if(fabsf(circles[i].x-clearers[j]->x)<circles[i].radius+clearers[j]->radius && fabsf(circles[i].y-clearers[j]->y)<circles[i].radius+clearers[j]->radius){
					dodges++;
					goto circle_remove;
				}
			}
		}

		if(circles[i].x + circles[i].radius < 0 || circles[i].y + circles[i].radius < 0 || circles[i].x - circles[i].radius > width || circles[i].y - circles[i].radius > height){
			dodges++;
		circle_remove:
			VECTOR_REMOVE(circles, i);
			continue;
		}

		if(fabsf(circles[i].x-mouseX)<=circles[i].radius && fabsf(circles[i].y-mouseY)<=circles[i].radius){
			switch (circles[i].type) {
				case ENEMY: {
					switch (active_powerup) {
						case POWERUP_REMOVER: {
							VECTOR_REMOVE(circles, i);
							dodges++;
							continue;
						}
						case POWERUP_INVINCIBILITY: {
							break;
						}
						default: {
							die();
							return;
						}
					}
					break;
				}
				case POWERUP_CLEAR: {
					explode(&circles[i]);
					circles[i].velX = 0.f;
					circles[i].velY = 0.f;
					VECTOR_ADD_A(clearers, &circles[i], circles[i].clearer_index);
					break;
				}
				case POWERUP_REMOVER: {
					active_powerup = POWERUP_REMOVER;
					powerup_start_time = clock();
					VECTOR_REMOVE(circles, i);
					continue;
				}
				case POWERUP_INVINCIBILITY: {
					active_powerup = POWERUP_INVINCIBILITY;
					powerup_start_time = clock();
					VECTOR_REMOVE(circles, i);
					continue;
				}
			}
		}

        circles[i].x += circles[i].velX * delta_time;
        circles[i].y += circles[i].velY * delta_time;

		loop_end_add:
		add_circle(&circles[i]);
    }
loop_end:
	if(powerup_names[active_powerup]){
		EM_ASM({
				document.getElementById("dodges").innerText = "Dodges: " + $0;
				document.getElementById("timer").innerText = $1 + "s";
				document.getElementById("powerup").innerText = "Powerup: " + UTF8ToString($2) + "(" + $3 + " s)";
		}, dodges, (clock()-start_time)/CLOCKS_PER_SEC, powerup_names[active_powerup], POWERUP_DURATION-(clock()-powerup_start_time)/CLOCKS_PER_SEC);
	}else {
		EM_ASM({
				document.getElementById("dodges").innerText = "Dodges: " + $0;
				document.getElementById("timer").innerText = $1 + "s";
				document.getElementById("powerup").innerText = "";
		}, dodges, (clock()-start_time)/CLOCKS_PER_SEC);
	}
    draw();
	frame_time = clock();
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
	srand((unsigned int) clock());
	active_powerup = ENEMY;
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
	start_time = clock();
	active_powerup = ENEMY;
	started = false;
	dodges = 0;
	dead = false;
}
