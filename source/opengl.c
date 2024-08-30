void LoadOpenGLFunctions()
{
#define X(N,R,P) N = (GL_##N*)wglGetProcAddress(#N);
#include "third_party/ogl_api_funcs.h"
#undef X
}

u32 CreateTextureFromData(u8* data, i32 width, i32 height, i32 channels)
{
	u32 result;
	
	glGenTextures(1, &result);
	glBindTexture(GL_TEXTURE_2D, result);
	
	u32 mode = GL_RGB;
	if (channels >= 4)
		mode = GL_RGBA;
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, mode, GL_UNSIGNED_BYTE, data);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	return result;
}

u32 CreateTextureFromRGBA(v4 color)
{
	u32 result;
	u8 data[] = { color.r, color.g, color.b, color.a };
	result = CreateTextureFromData((u8*)data, 1, 1, 4);
	return result;
}

u32 LoadTexture(const char* path)
{
	u32 result = 0;
	i32 width, height, channels;
	
	u8* data = stbi_load(path, &width, &height, &channels, 0);
	if (!data)
		printf(FormatString("Failed to load texture. path: '%s'\n", path));
	
	result = CreateTextureFromData(data, width, height, channels);
	stbi_image_free(data);
	return result;
}

u32 LoadShader(const char* path)
{
	u32 result;
	u32 vertex;
	u32 fragment;
	b32 success;
	char errorLog[1024];
	
	char* shaderSource = ReadEntireFile(path);
	char* vertexSource = FormatString("#version 460 core\n#define VERTEX_SHADER 1\n%s", shaderSource);
	char* fragmentSource = FormatString("#version 460 core\n#define FRAGMENT_SHADER 1\n%s", shaderSource);
	
	printf("-- Compiling shader. path: '%s' --\n", path);
	
	// Create vertex shader.
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertexSource, NULL);
	glCompileShader(vertex);
	
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	glGetShaderInfoLog(vertex, 512, NULL, errorLog);
	if (!success)
		printf("Failed to compile vertex shader. Error log: %s\n", errorLog);
	
	// Create fragment shader.
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragmentSource, NULL);
	glCompileShader(fragment);
	
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	glGetShaderInfoLog(fragment, 512, NULL, errorLog);
	if (!success)
		printf("Failed to compile fragment shader. Error log: %s\n", errorLog);
	
	// Create and Link shader program.
	result = glCreateProgram();
	glAttachShader(result, vertex);
	glAttachShader(result, fragment);
	glLinkProgram(result);
	
	glGetProgramiv(result, GL_LINK_STATUS, &success);
	glGetProgramInfoLog(result, 512, NULL, errorLog);
	if (!success)
		printf("Failed to link shader program. Error log: %s\n", errorLog);
	
	// Delete unsused shaders.
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	
	free(vertexSource);
	free(fragmentSource);
	free(shaderSource);
	
	return result;
}

void DrawTexture(State* state, u32 texture, v2 pos, v2 scale, f32 angle)
{
	if (!texture)
		texture = state->errorTexture;
	
	glUseProgram(state->defaultShader);
	glBindTexture(GL_TEXTURE_2D, texture);
	
	glUniform1f(glGetUniformLocation(state->defaultShader, "uTexture"), 0);
	glUniform1f(glGetUniformLocation(state->defaultShader, "uAngle"), DegToRad(angle));
	glUniform2f(glGetUniformLocation(state->defaultShader, "uPos"), pos.x, pos.y);
	glUniform2f(glGetUniformLocation(state->defaultShader, "uScale"), scale.x, scale.y);
	glUniform4f(glGetUniformLocation(state->defaultShader, "uTint"), 1.0f, 1.0f, 1.0f, 1.0f);
	glUniform2f(glGetUniformLocation(state->defaultShader, "uCameraPos"), state->cameraPos.x, state->cameraPos.y);
	glUniformMatrix4fv(glGetUniformLocation(state->defaultShader, "uProjection"),
					   1, 0, &state->projection.elements[0][0]);
	
	glBindVertexArray(state->vao);
	glBindBuffer(GL_ARRAY_BUFFER, state->vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->ebo);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void DrawTextureTinted(State* state, u32 texture, v2 pos, v2 scale, f32 angle, v4 tint)
{
	if (!texture)
		texture = state->errorTexture;
	
	glUseProgram(state->defaultShader);
	glBindTexture(GL_TEXTURE_2D, texture);
	
	glUniform1f(glGetUniformLocation(state->defaultShader, "uTexture"), 0);
	glUniform1f(glGetUniformLocation(state->defaultShader, "uAngle"), DegToRad(angle));
	glUniform2f(glGetUniformLocation(state->defaultShader, "uPos"), pos.x, pos.y);
	glUniform2f(glGetUniformLocation(state->defaultShader, "uScale"), scale.x, scale.y);
	glUniform4f(glGetUniformLocation(state->defaultShader, "uTint"), tint.x, tint.y, tint.z, tint.w);
	glUniform2f(glGetUniformLocation(state->defaultShader, "uCameraPos"), state->cameraPos.x, state->cameraPos.y);
	glUniformMatrix4fv(glGetUniformLocation(state->defaultShader, "uProjection"),
					   1, 0, &state->projection.elements[0][0]);
	
	glBindVertexArray(state->vao);
	glBindBuffer(GL_ARRAY_BUFFER, state->vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->ebo);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void DrawAABB(State* state, AABB aabb)
{
	DrawTextureTinted(state, 0, aabb.min, V2SubV2(aabb.max, aabb.min), 0.0f, V4(1.0f, 0.0f, 0.0f, 0.25f));
}

void DrawUiTexture(State* state, u32 texture, v2 pos, v2 scale, f32 angle, v4 tint)
{
	/*pos = V2MulV2(pos, state->windowSize);
	scale = V2MulV2(scale, state->windowSize);
	*/
	if (!texture)
		texture = state->errorTexture;
	
	glUseProgram(state->uiShader);
	glBindTexture(GL_TEXTURE_2D, texture);
	
	glUniform1f(glGetUniformLocation(state->uiShader, "uTexture"), 0);
	glUniform4f(glGetUniformLocation(state->uiShader, "uTint"), tint.x, tint.y, tint.z, tint.w);
	glUniform1f(glGetUniformLocation(state->uiShader, "uAngle"), DegToRad(angle));
	glUniform2f(glGetUniformLocation(state->uiShader, "uPos"), pos.x, pos.y);
	glUniform2f(glGetUniformLocation(state->uiShader, "uScale"), scale.x, scale.y);
	glUniformMatrix4fv(glGetUniformLocation(state->uiShader, "uProjection"),
					   1, 0, &state->uiProjection.elements[0][0]);
	
	glBindVertexArray(state->vao);
	glBindBuffer(GL_ARRAY_BUFFER, state->vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->ebo);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void DrawChar(State* state, v2 pos, v2 scale, i32 character, v4 tint)
{
	glUseProgram(state->fontShader);
	glBindTexture(GL_TEXTURE_2D, state->fontTexture);
	
	glUniform1f(glGetUniformLocation(state->fontShader, "uTexture"), 0);
	glUniform4f(glGetUniformLocation(state->fontShader, "uTint"), tint.x, tint.y, tint.z, tint.w);
	glUniform2f(glGetUniformLocation(state->fontShader, "uPos"), pos.x, pos.y);
	glUniform2f(glGetUniformLocation(state->fontShader, "uScale"), scale.x, scale.y);
	glUniform1f(glGetUniformLocation(state->fontShader, "uChar"), character - 32);
	glUniform2f(glGetUniformLocation(state->fontShader, "uCameraPos"), 0.0f, 0.0f);
	glUniformMatrix4fv(glGetUniformLocation(state->fontShader, "uProjection"),
					   1, 0, &state->uiProjection.elements[0][0]);
	
	glBindVertexArray(state->vao);
	glBindBuffer(GL_ARRAY_BUFFER, state->vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->ebo);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void DrawString(State* state, v2 pos, v2 scale, const char* string, v4 tint)
{
	u64 length = strlen(string);
	pos.x -= (f32)length / 2 * scale.x;
	
	for (u64 x = 0; x < length; ++x)
		DrawChar(state, V2AddV2(pos, V2(x * (scale.x * 0.75f), 0.0f)), scale, string[x], tint);
}

void DrawCharWorld(State* state, v2 pos, v2 scale, i32 character, v4 tint)
{
	glUseProgram(state->fontShader);
	glBindTexture(GL_TEXTURE_2D, state->fontTexture);
	
	glUniform1f(glGetUniformLocation(state->fontShader, "uTexture"), 0);
	glUniform4f(glGetUniformLocation(state->fontShader, "uTint"), tint.x, tint.y, tint.z, tint.w);
	glUniform2f(glGetUniformLocation(state->fontShader, "uPos"), pos.x, pos.y);
	glUniform2f(glGetUniformLocation(state->fontShader, "uScale"), scale.x, scale.y);
	glUniform1f(glGetUniformLocation(state->fontShader, "uChar"), character - 32);
	glUniform2f(glGetUniformLocation(state->fontShader, "uCameraPos"), state->cameraPos.x, state->cameraPos.y);
	glUniformMatrix4fv(glGetUniformLocation(state->fontShader, "uProjection"),
					   1, 0, &state->projection.elements[0][0]);
	
	glBindVertexArray(state->vao);
	glBindBuffer(GL_ARRAY_BUFFER, state->vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->ebo);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void StartRenderer(State* state)
{
	u32 indices[] = { 0, 1, 3, 1, 2, 3 };
	Vertex vertices[] =
	{
		{ V3(-.5f, 1.0f, 0.0f), V2(0.0f, 0.0f) },
		{ V3(0.5f, 1.0f, 0.0f), V2(1.0f, 0.0f) },
		{ V3(0.5f, 0.0f, 0.0f), V2(1.0f, 1.0f) },
		{ V3(-.5f, 0.0f, 0.0f), V2(0.0f, 1.0f) }
	};
	
	glGenVertexArrays(1, &state->vao);
	glGenBuffers(1, &state->vbo);
	glGenBuffers(1, &state->ebo);
	
	glBindVertexArray(state->vao);
	glBindBuffer(GL_ARRAY_BUFFER, state->vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->ebo);
	
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vertex), vertices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(u32), indices, GL_STATIC_DRAW);
	
	/// Position Vector
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), NULL);
	
	/// UV Coordinate
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(v3));
	
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	// Create error texture
	state->errorTexture = CreateTextureFromRGBA(V4(255, 0, 255, 255));
}