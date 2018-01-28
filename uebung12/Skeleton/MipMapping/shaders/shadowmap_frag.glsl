#version 440 core

layout(location = 0) out float out_depth;

void main() { out_depth = exp((gl_FragCoord.z * 2.0 - 1.0) * 96); }
