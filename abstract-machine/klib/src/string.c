#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s)
{
	size_t i = 0;
	while (*s)
	{
		i++, s++;
	}
	return i;
}

char *strcpy(char *dst, const char *src)
{
	for (size_t i = 0; src[i]; i++)
	{
		dst[i] = src[i];
	}
	return dst;
}

char *strncpy(char *dst, const char *src, size_t n)
{
	size_t i;

	for (i = 0; i < n && src[i] != '\0'; i++)
		dst[i] = src[i];
	for (; i < n; i++)
		dst[i] = '\0';
	return dst;
}

char *strcat(char *dst, const char *src)
{
	size_t dst_len = strlen(dst);
	size_t i;
	for (i = 0; src[i] != '\0'; i++)
		dst[dst_len + i] = src[i];
	
	dst[dst_len + i] = '\0';

	return dst;
}

int strcmp(const char *s1, const char *s2)
{
	for(size_t i = 0; s1[i] && s2[i] ;i++){
		if(s1[i] != s2[i]){
			return s1[i] - s2[i];
		}
	}
	return 0;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	for(size_t i = 0; i < n ;i++){
		if(s1[i] != s2[i]){
			return s1[i] - s2[i];
		}
	}
	return 0;
}

void *memset(void *s, int c, size_t n)
{
	
	for(size_t i = 0; i < n; i++){
		*(((uint8_t *)s) + i) = (unsigned char )c;
	}
	return s;
}

void *memmove(void *dst, const void *src, size_t n)
{
	uint8_t temp[n];
	for(size_t i = 0; i < n; i++){
		temp[i] = 0;
	}
	for(size_t i = 0; i < n; i++){
		temp[i] = *(((uint8_t *) src) + i);
	} 
	for(size_t i = 0; i < n; i++){
		*(((uint8_t *)dst) + i) = temp[i];
	}
	return dst;
}

void *memcpy(void *out, const void *in, size_t n)
{
	for(size_t i = 0; i < n; i++){
		*(((uint8_t *)out) + i) = *(((uint8_t *)in) + i);
	}
	return out;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
	for(size_t i = 0; i < n ;i++){
		if(*(((uint8_t *)s1) + i) != *(((uint8_t *)s2)+ i)){
			return *(((uint8_t *)s1) + i) - *(((uint8_t *)s2)+ i);
		}
	}
	return 0;
}

#endif
