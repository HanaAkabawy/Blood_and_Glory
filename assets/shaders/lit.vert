#version 330 core

// Inputs from mesh
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 tex_coord;
layout(location = 3) in vec3 normal;

// Outputs to fragment shader
out Varyings {
    vec3 world_position;
    vec3 world_normal;
    vec2 tex_coord;
} vs_out;

// === UNIFORMS (set by C++ code) ===
uniform mat4 model;                    // Model space → World space
uniform mat4 view_projection;          // World space → Screen space
uniform mat4 model_inverse_transpose;  // For correct 

void main(){
vec4 world_pos = model*vec4(position,1.0);
vs_out.world_position = world_pos.xyz; //we only need xyz not w for fragment shader
vs_out.world_normal = normalize((model_inverse_transpose*vec4(normal,0.0)).xyz);
vs_out.tex_coord = tex_coord;
gl_Position = view_projection*world_pos; //transforms world pos into the clip space which is the coordinate syustem the gpu uses for rendering

}