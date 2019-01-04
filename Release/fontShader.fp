#version 130

varying vec4 outColor;
uniform vec4 fontColor;
uniform sampler2D _texture;

void main(void)
{ 
	//vec4 color = outColor;
	vec4 color = texture2D(_texture,gl_TexCoord[0].st);
	
	if(color == vec4(0,0,0,0))
		gl_FragColor = vec4(0,0,0,0);
	else
		gl_FragColor = fontColor;//vec4(0,1,1,1);
}