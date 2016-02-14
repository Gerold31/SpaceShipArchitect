#version 330

in vec3 fragColor;

layout(location = 0) out vec4 pixelColor;

void main() {
	pixelColor = vec4(fragColor, 1.0); // RGBA
}
