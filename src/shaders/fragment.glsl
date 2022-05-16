"#version 300 es\n\
\n\
precision mediump float;\n\
\n\
flat in float v_radius;\n\
flat in vec2 v_center;\n\
flat in float v_height;\n\
flat in vec4 v_color;\n\
flat in float v_border;\n\
flat in vec4 v_border_color;\n\
\n\
out vec4 fragColor;\n\
\n\
void main()\n\
{\n\
    float dist = distance(v_center, vec2(gl_FragCoord.x, v_height-gl_FragCoord.y));\n\
    if(dist==0.0) dist=1.0;\n\
    float d = dist / v_radius;\n\
	float border = v_border/v_radius;\n\
    if(d <= 1.0 - border) fragColor = v_color;\n\
	else if(d <= 1.0) fragColor = v_border_color;\n\
    else fragColor = vec4(0.0);\n\
\n\
}\
"
