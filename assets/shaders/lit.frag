#version 330 core

in Varyings {
    vec3 world_position;
    vec3 world_normal;
    vec2 tex_coord;
} fs_in;

out vec4 frag_color;

uniform sampler2D tex;
uniform vec4 tint;

uniform vec3 light_direction;
uniform vec3 light_color;
uniform vec3 ambient_light;
uniform vec3 camera_position;

void main(){
   vec4 base_color = texture(tex, fs_in.tex_coord)*tint;
   vec3 normal = normalize(fs_in.world_normal);
   vec3 light_dir = normalize(-light_direction); //for lighting math we need the direction FROM THE SURFACE TO THE LIGHT, so we reverse the direction
   vec3 ambient = ambient_light * base_color.rgb;
float diff = max(dot(normal, light_dir), 0.0);
vec3 difuse =  diff*light_color*base_color.rgb;
}