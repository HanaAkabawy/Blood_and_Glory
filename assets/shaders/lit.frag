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
uniform float shininess;
uniform int light_type;
uniform vec3 light_position;
uniform vec3 light_attenuation;
uniform vec2 light_cone;

void main(){
    vec4 base_color = texture(tex, fs_in.tex_coord)*tint;
    vec3 normal = normalize(fs_in.world_normal);
    vec3 light_dir = normalize(-light_direction); //for lighting math we need the direction FROM THE SURFACE TO THE LIGHT, so we reverse the direction
    vec3 ambient = ambient_light * base_color.rgb;
    float diff = max(dot(normal, light_dir), 0.0);
    vec3 diffuse =  diff*light_color*base_color.rgb;
    vec3 view_dir = normalize(camera_position - fs_in.world_position);
    vec3 halfway_dir = normalize(light_dir + view_dir);
    float spec = pow(max(dot(normal, halfway_dir), 0.0), shininess);
    vec3 specular = spec * light_color;
    
    // 7. Combine
    vec3 result = ambient + diffuse + specular;
    
    // 8. Output
    frag_color = vec4(result, base_color.a);
}