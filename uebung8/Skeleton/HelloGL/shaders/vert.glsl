#version 400 core

uniform mat4 transform;

in vec3 in_pos;

void main()
{
    /*
     * TODO (zu Teilaufgabe 2): Setzen Sie die Vertexposition mit der transformierten in_pos.
     */
    //gl_Position = vec4(-0.8f + 0.8f * gl_VertexID, gl_VertexID == 1 ? 0.7f : -0.6f, 0.f, 5.0f);
	gl_Position = transform * vec4(in_pos, 1.0f);
}
