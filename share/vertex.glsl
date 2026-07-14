#version 330 core
#ifdef GLES
precision mediump float;
#endif

layout (location=0) in vec2 position;
layout (location=1) in vec3 vertex_color_cpu;
out vec3 vertex_color;
//out vec2 uv;
// out vec2 fragCoord;

void main(){
    vertex_color = vertex_color_cpu;
    //uv = position;
    gl_Position = vec4(position.x, position.y, 0.0, 0.5);
//     fragCoord = abs(position);
}
