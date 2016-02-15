#version 330

in vec3 fragColor;
in vec2 fragTexCord;

layout(location = 0) out vec4 pixelColor;

uniform sampler2D tex;

void main() {
	vec4 texColor = texture2D(tex, fragTexCord);
	pixelColor = texColor * vec4(fragColor, 1.0); // RGBA
}
