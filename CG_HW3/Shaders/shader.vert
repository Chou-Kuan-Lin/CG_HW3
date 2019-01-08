//vertex
#version 400

layout(location = 0) in vec3 ModelPosition;
layout(location = 1) in vec3 ModelNormal;
layout(location = 2) in vec2 ModelTexcoord;

uniform mat4 ProjectionMatrix;
uniform mat4 ModelMatrix;
uniform mat4 ModelViewMatrix;

//model Information
out vec2 modelTexcoord;
out vec3 modelPosition;

// Rim Effect
out vec3 rimPosition;
out vec3 rimNormal;

void main()
{
	// Model Coordinates * Model Matrix = World Coordinates
	// World Coordinates * View Matrix(MV=Camera LookAt) = Camera Coordinates
	// Camera Coordinates * Projection Matrix = Projection Coordinates
	gl_Position = ProjectionMatrix * ModelViewMatrix * vec4( ModelPosition, 1 );

	// texture
	modelTexcoord = ModelTexcoord;
	modelPosition = ModelPosition;
	
	// Rim Effect
	mat3 ModelMatrix3 = mat3( transpose(inverse( ModelMatrix )) );
	rimPosition = ModelMatrix3 * ModelPosition;
	rimNormal = normalize( ModelMatrix3 * ModelNormal );
}