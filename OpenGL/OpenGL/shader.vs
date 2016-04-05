#version 330 //GLSL language version

layout (location = 0) in vec3 Position;

uniform mat4 gWorld;

out vec4 Colour;

void main()
{		
	gl_Position = gWorld * vec4(Position.x,
		Position.y,
		Position.z,
		1.0);

	//Colour = vec4(clamp(Position, 0.0, 1.0), 1.0);
	//gl_Position = gWorld * vec4(1.0, 1.0, 1.0, 1.0);
	Colour = vec4(1.0, 1.0, 1.0, 1.0);
}