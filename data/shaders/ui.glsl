#if VERTEX_SHADER

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

uniform float uAngle;
uniform vec2 uPos;
uniform vec2 uScale;
uniform mat4 uProjection;

out vec2 texCoords;

vec2 rotate(vec2 v, float a) {
	float s = sin(a);
	float c = cos(a);
	mat2 m = mat2(c, s, -s, c);
	return m * v;
}

void main()
{
	vec2 pos = vec2(aPos.x, aPos.y - 0.5) * uScale;
	pos = rotate(pos, uAngle);
	pos += uPos;
	gl_Position = uProjection * vec4(pos, 0.0, 1.0);
	texCoords = vec2(aTexCoords.s, 1.0 - aTexCoords.t);
}

#elif FRAGMENT_SHADER

out vec4 color;

uniform sampler2D uTexture;
uniform vec4 uTint;
in vec2 texCoords;

void main()
{
	color = texture(uTexture, texCoords) * uTint;
}

#endif