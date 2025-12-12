#version 330 core

in Varyings {
    vec3 world_position;
    vec3 world_normal;
    vec2 tex_coord;
} fs_in;
#define MAX_LIGHTS 16
out vec4 frag_color;

uniform sampler2D tex;
uniform vec4 tint;
struct light{
    int type;
    vec3 direction;
    vec3 color;
    vec3 position;
    vec3 attenuation;
    vec2 cone;
};
uniform light lights[MAX_LIGHTS];
uniform int light_count;
uniform vec3 ambient_light;
uniform vec3 camera_position;
uniform float shininess;
uniform sampler2D specular_tex;
uniform sampler2D emissive_tex;
uniform bool has_specular_map;
uniform bool has_emissive_map;


void main(){
    vec4 base_color = texture(tex, fs_in.tex_coord)*tint;
    vec3 normal = normalize(fs_in.world_normal);
   
    vec3 light_dir;
    vec3 tot_diff = vec3(0.0);
    vec3 tot_spec = vec3(0.0);
    for(int i=0;i<min(light_count, MAX_LIGHTS);i++){
        light curr = lights[i];
         float attenuation =1.0;
    if(curr.type==0) //directional
    {
    light_dir = normalize(-curr.direction); //for lighting math we need the direction FROM THE SURFACE TO THE LIGHT, so we reverse the direction
    } else if(curr.type==1) //POINT
    {
        vec3 to_light = curr.position - fs_in.world_position;
        float distance = length(to_light);
        light_dir = normalize(to_light);
        attenuation = 1.0/(curr.attenuation.x+curr.attenuation.y*distance + curr.attenuation.z*distance*distance);
    } else if(curr.type==2){
                vec3 to_light = curr.position - fs_in.world_position;
        float distance = length(to_light);
        light_dir = normalize(to_light);
        attenuation = 1.0/(curr.attenuation.x+curr.attenuation.y*distance + curr.attenuation.z*distance*distance);
        vec3 spot_dir = normalize(-curr.direction);
        float cos_angle = dot(light_dir, spot_dir);
        float cos_inner = cos(curr.cone.x);
        float cos_outer = cos(curr.cone.y);
// Smooth falloff between inner and outer cone
        float spot_effect = smoothstep(cos_outer, cos_inner, cos_angle);
        attenuation *= spot_effect;
    }
    
    float diff = max(dot(normal, light_dir), 0.0);
    tot_diff +=  attenuation* diff*curr.color*base_color.rgb;
    vec3 view_dir = normalize(camera_position - fs_in.world_position);
    vec3 halfway_dir = normalize(light_dir + view_dir);
    float spec = pow(max(dot(normal, halfway_dir), 0.0), shininess);
    vec3 spec_map_value = vec3(1.0);
    if(has_specular_map){
    spec_map_value = texture(specular_tex, fs_in.tex_coord).rgb;
    }
    vec3 specular = attenuation* spec * curr.color*spec_map_value;
    tot_spec += specular;
    }
    vec3 ambient = ambient_light * base_color.rgb;
    vec3 emissive = vec3(0.0);
    if(has_emissive_map){
    emissive = texture(emissive_tex, fs_in.tex_coord).rgb;
}    
    // 7. Combine
    vec3 result = ambient + tot_diff + emissive + tot_spec;
    
    // 8. Output
    frag_color = vec4(result, base_color.a);
}