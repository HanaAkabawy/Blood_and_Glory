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
    float attenuation =1.0;
    vec3 light_dir;
    if(light_type==0) //directional
    {
    light_dir = normalize(-light_direction); //for lighting math we need the direction FROM THE SURFACE TO THE LIGHT, so we reverse the direction
    } else if(light_type==1) //POINT
    {
        vec3 to_light = light_position - fs_in.world_position;
        float distance = length(to_light);
        light_dir = normalize(to_light);
        attenuation = 1.0/(light_attenuation.x+light_attenuation.y*distance + light_attenuation.z*distance*distance);
    } else if(light_type==2){
                vec3 to_light = light_position - fs_in.world_position;
        float distance = length(to_light);
        light_dir = normalize(to_light);
        attenuation = 1.0/(light_attenuation.x+light_attenuation.y*distance + light_attenuation.z*distance*distance);
        vec3 spot_dir = normalize(-light_direction);
        float cos_angle = dot(light_dir, spot_dir);
        float cos_inner = cos(light_cone.x);
        float cos_outer = cos(light_cone.y);
// Smooth falloff between inner and outer cone
        float spot_effect = smoothstep(cos_outer, cos_inner, cos_angle);
        attenuation *= spot_effect;
    }
    vec3 ambient = ambient_light * base_color.rgb;
    float diff = max(dot(normal, light_dir), 0.0);
    vec3 diffuse =  attenuation* diff*light_color*base_color.rgb;
    vec3 view_dir = normalize(camera_position - fs_in.world_position);
    vec3 halfway_dir = normalize(light_dir + view_dir);
    float spec = pow(max(dot(normal, halfway_dir), 0.0), shininess);
    vec3 specular = attenuation* spec * light_color;
                                  
    // 7. Combine
    vec3 result = ambient + diffuse + specular;
    
    // 8. Output
    frag_color = vec4(result, base_color.a);
}