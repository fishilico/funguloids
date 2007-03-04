uniform sampler2D tex0;

varying vec2 texCoord[5];

float luminance(vec4 c)
{
	return dot( c, vec4(0.3, 0.59, 0.11, 0.0) );
}

void main()
{
	vec4 col = texture2D(tex0, texCoord[0]);
	vec4 sum = luminance(col) * 3.5 * col;

	col = texture2D(tex0, texCoord[1]);
	sum += luminance(col) * 1.5 * col;

	col = texture2D(tex0, texCoord[2]);
	sum += luminance(col) * 1.5 * col;

	col = texture2D(tex0, texCoord[3]);
	sum += luminance(col) * 1.5 * col;

	col = texture2D(tex0, texCoord[4]);
	sum += luminance(col) * 1.5 * col;

/*	vec4 sum = texture2D(tex0, texCoord[0]) +
			   texture2D(tex0, texCoord[1]) +
			   texture2D(tex0, texCoord[2]) +
			   texture2D(tex0, texCoord[3]) +
			   texture2D(tex0, texCoord[4]);
*/
	gl_FragColor = sum / 5.0;
}

