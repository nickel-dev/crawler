inline char* ReadEntireFile(const char* path)
{
	FILE *f = fopen(path, "r");
	if (!f)
	{
		printf("-- Failed to read file. path: '%s' --\n", path);
		return NULL;
	}
	
	fseek(f, 0, SEEK_END);
	u64 size = ftell(f);
	fseek(f, 0, SEEK_SET);
	
	char* result = malloc(size + 1);
	fread(result, size, 1, f);
	fclose(f);
	
	result[size] = 0;
	return result;
}

inline char* FormatString(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);
	
	i32 len = _vscprintf(format, ap);
	if (len < 0)
		return NULL;
	
	char* buffer = (char*)malloc((u64)len + 1);
	
	if (stbsp_vsprintf(buffer, format, ap) < 0)
	{
		free((void*)buffer);
		return NULL;
	}
	
	va_end(ap);
	return buffer;
}