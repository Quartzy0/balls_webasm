"#version 300 es\n\
\
layout(location=0) in vec2 position;\
layout(location=1) in float radius;\
layout(location=2) in vec2 center;\
layout(location=3) in vec3 color;\
layout(location=4) in float border;\
\
flat out float v_radius;\
flat out vec2 v_center;\
flat out float v_height;\
flat out vec3 v_color;\
flat out float v_border;\
\
uniform mat4 proj;\
uniform float screenHeight;\
\
void main()\
{\
    gl_Position = proj * vec4(position, 0.0, 1.0);\
    v_radius = radius;\
    v_center = center;\
    v_height = screenHeight;\
	v_color = color;\
	v_border = border;\
}\
"
