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
layout(location = 1) in vec3 in_uv; // uv coordinates for texture mapping

layout(location = 1) out vec2 uv;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main() {
    /*
     * TODO (zu Teilaufgabe 2.2): Kopieren Sie ihre Loesung von Teilaufgabe 1.3 und 1.4
     */
	mat4 model_view = per_frame.view * trees.model[gl_InstanceID];

     /*
     * TODO (zu Teilaufgabe 2.5): Verwenden Sie die Hilfsfunktion rand um eine
	 * Zufallszahl zwischen 0.7 und 1.0 als Wert fuer scale_factor zu berechnen.
	 * Verwenden sie die xz Position der jeweiligen Instanz als Eingabe für rand.
     */	
    float scale_factor = rand(in_pos.xz) * 0.3f + 0.7f;
    vec3 scaled_pos = in_pos * scale_factor;
	
    // output
    gl_Position = per_frame.proj *(model_view * vec4(0.f, 0.f, 0.f, 1.f) + vec4(in_pos.x, in_pos.y, 0.f, 0.f));
    uv = in_uv.rg;
}