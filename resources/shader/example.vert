#version 330

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertColor;
layout(location = 2) in vec2 vertTexCord;

out vec3 fragColor;
out vec2 fragTexCord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
	fragColor = vertColor;
	fragTexCord = vertTexCord;
	gl_Position = proj * view * model * vec4(vertPos, 1.0);
}
