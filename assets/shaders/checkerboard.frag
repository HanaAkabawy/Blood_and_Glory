#version 330 core

out vec4 frag_color;

// In this shader, we want to draw a checkboard where the size of each tile is (size x size).
// The color of the top-left most tile should be "colors[0]" and the 2 tiles adjacent to it
// should have the color "colors[1]".

//TODO: (Req 1) Finish this shader.

uniform int size = 32;
uniform vec3 colors[2];

void main(){
    // Get the pixel coordinates
    ivec2 pixel = ivec2(floor(gl_FragCoord.xy));
    
    // Divide by size to get tile coordinates
    ivec2 tile = pixel / size;
    
    // Calculate checkerboard pattern: sum of tile coordinates
    // If even, use colors[0], if odd use colors[1]
    int index = (tile.x + tile.y) % 2;
    
    frag_color = vec4(colors[index], 1.0);
}