#version 430

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

uniform layout(binding = 0) sampler2D s_Image;

uniform float u_PixelSize;

#include "../../fragments/frame_uniforms.glsl"

void main() {

        float Pixels = u_PixelSize;
        float dx = 15.0 * (1.0 / Pixels);
        float dy = 10.0 * (1.0 / Pixels);
        vec2 Coord = vec2(dx * floor(inUV.x / dx),
                          dy * floor(inUV.y / dy));
        outColor = texture(s_Image, Coord);

}
