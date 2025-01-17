#version 440 core

layout(binding = 0) uniform PerFrame {
    mat4 view; // world coords -> camera coords
    mat4 proj; // camera coords -> clip coords
}
per_frame;

layout(binding = 1) uniform Trees {
    mat4 model[192]; // object coords -> world coords
}
trees;

layout(location = 0) in vec3 in_pos; // position in object coords
layout(location = 1) in vec3 in_color;

layout(location = 1) out vec3 frag_color;

void main() {
    /*
     * TODO (zu Teilaufgabe 1.3): Modifizieren Sie den Code, sodass die Model-Matrizen korrekt
     * verwendet werden.
     *
     * TODO (zu Teilaufgabe 1.4): Implementieren Sie den Billboard-Effekt.
     */
	 
    mat4 model_view = per_frame.view * trees.model[gl_InstanceID];
    // output
    gl_Position = per_frame.proj * (model_view * vec4(0.f, 0.f, 0.f, 1.f) + vec4(in_pos.x, in_pos.y, 0.f, 0.f));
    frag_color = in_color;
}
