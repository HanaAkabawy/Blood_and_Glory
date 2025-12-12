#version 330 core

in vec2 tex_coord;

out vec4 frag_color;
uniform sampler2D tex;

void main(){

    vec4 color = texture(tex, tex_coord);
    
    float r = color.r * 0.393 + color.g * 0.769 + color.b * 0.189;
    float g = color.r * 0.349 + color.g * 0.686 + color.b * 0.168;
    float b = color.r * 0.272 + color.g * 0.534 + color.b * 0.131;
    
    frag_color = vec4(r, g, b, color.a);
}