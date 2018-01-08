#version 440 core

uniform sampler2D tree_sprite_tx2D;

layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 out_color;

void main()
{
    /*
     * TODO (zu Teilaufgabe 2.4): Verwenden Sie die uebergebenen Texturcoordinaten uv
	 * um auf die Texture tree_sprite_tx2D zuzugreifen. Weisen Sie den ausgelesenen
	 * Wert out_color zu.
	 * Wenn der Wert des Alpha-Kanals unter eines gewissen Grenzwertes faellt, verwerfen
	 * Sie das Fragment mit dem discard Befehl. Experimentieren Sie mit unterschiedlichen
	 * Grenzwerten bis Sie ein visuell zufriedenstellendes Ergebnis erhalten.
     */
	out_color = texture2D(tree_sprite_tx2D, uv).bgra;
	if (out_color[3] <= 0.5f){
		discard;
	}
}
