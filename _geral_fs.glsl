#version 410

in vec2 texture_coords;
uniform sampler2D sprite;
uniform float offsetx;
uniform float offsety;
uniform float weight;
uniform vec4 highlight_color; // Nova variável para escolher a cor!

out vec4 frag_color; 

void main () {
    vec4 tex_base = texture(sprite, vec2(texture_coords.x + offsetx, texture_coords.y + offsety));
    
    // Mistura a cor da textura com a cor de destaque (azul ou verde)
    vec4 texel = mix(tex_base, highlight_color, weight);
    
    if(texel.a < 0.1) {
        discard;
    }
    
    frag_color = texel;
}