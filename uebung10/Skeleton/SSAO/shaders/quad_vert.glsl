#version 440 core

// Patch filling the entire window
vec2 vertices[] = {vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(-1.0, 1.0),
				   vec2(1.0, -1.0),  vec2(1.0, 1.0),  vec2(-1.0, 1.0)};

void main() {
	gl_Position = vec4(vertices[gl_VertexID], 0.0, 1.0);
}
