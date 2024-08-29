#if VERTEX_SHADER

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

uniform float uAngle;
uniform vec2 uPos;
uniform vec2 uScale;
uniform vec2 uCameraPos;
uniform mat4 uProjection;

out vec2 texCoords;
out vec2 vertexCoord;

vec2 rotate(vec2 v, float a) {
	float s = sin(a);
	float c = cos(a);
	mat2 m = mat2(c, s, -s, c);
	return m * v;
}

void main()
{
	vec2 pos = aPos.xy * uScale;
	pos = rotate(pos, uAngle);
	pos += uPos - uCameraPos;
	gl_Position = uProjection * vec4(pos, 0.0f, 1.0);
	vertexCoord = gl_Position.xy;
	texCoords = aTexCoords;
}

#elif FRAGMENT_SHADER

out vec4 color;

uniform sampler2D uTexture;
uniform vec4 uTint;

in vec2 texCoords;
in vec2 vertexCoord;

float n2ddistance(vec2 first_point, vec2 second_point)
{
	float x = first_point.x-second_point.x;
	float y = first_point.y-second_point.y;
	float val = x*x + y*y;
	return sqrt(val);
}

void main()
{
	vec2 lightPos = vec2(0.0f, 0.0f);
	//vec3 diffuse = vec3(1.0f, 0.9f, 0.8f);
	vec3 diffuse = vec3(1.0f);
	float dst = n2ddistance(lightPos, vertexCoord);
	float intensity = clamp(1.0 - dst / 1.5f, 0.0, 1.0);
	color = texture(uTexture, texCoords) * uTint * (vec4(diffuse.xyz, 1.0) * intensity * 1.5); 
}

#endif