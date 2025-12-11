#version 330

uniform sampler2D tex;
in vec2 tex_coord;
out vec4 frag_color;

// Simple sepia tone postprocess
void main(){
    vec4 c = texture(tex, tex_coord);
    float r = c.r;
    float g = c.g;
    float b = c.b;
    // Sepia conversion matrix
    float sr = clamp((r * 0.393) + (g * 0.769) + (b * 0.189), 0.0, 1.0);
    float sg = clamp((r * 0.349) + (g * 0.686) + (b * 0.168), 0.0, 1.0);
    float sb = clamp((r * 0.272) + (g * 0.534) + (b * 0.131), 0.0, 1.0);
    frag_color = vec4(sr, sg, sb, c.a);
}
