#include <stdbool.h>
#include <wchar.h>

size_t wcslen(const wchar_t* str)
{
	size_t len = 0;
	while (*(str++))
		++len;
	return len;
}

wchar_t* wcscpy(wchar_t* dest, const wchar_t* src)
{
	wchar_t* originalDest = dest;
	while ((*dest++ = *src++) != '\0')
		;
	return originalDest;
}

wchar_t* wcsncpy(wchar_t* dest, const wchar_t* src, size_t num)
{
	wchar_t* originalDest = dest;
	while (((*dest++ = *src++) != '\0') && ((size_t)(dest - originalDest) < num))
		;
	return originalDest;
}

int wcscmp(const wchar_t* s1, const wchar_t* s2)
{
	while (*s1 == *s2++)
		if (*s1++ == 0)
			return 0;
	return *(const wchar_t*)s1 - *(const wchar_t*)--s2;
}

wchar_t* wcschr(const wchar_t* str, int c)
{
	wchar_t ch = c;
	for (;; ++str)
	{
		if (*str == ch)
			return (wchar_t*)str;
		if (!*str)
			return NULL;
	}
}

const wchar_t* wcsrchr(const wchar_t* str, wchar_t wc)
{
	wchar_t* last = NULL;
	wchar_t c;
	for (; (c = *str); ++str)
	{
		if (c == wc)
			last = (wchar_t*)str;
	}
	return last;
}

wchar_t* wcscat(wchar_t* dest, const wchar_t* src)
{
	size_t dest_length = wcslen(dest);
	size_t i;
	for (i = 0; src[i] != '\0'; i++)
		dest[dest_length + i] = src[i];
	dest[dest_length + i] = '\0';
	return dest;
}

wchar_t* wcstok(wchar_t* str, const wchar_t* delim, wchar_t** ptr)
{
	wchar_t* used_str = str;
	if (!used_str)
	{
		used_str = *ptr;
	}

	size_t token_start = 0;
	size_t token_end = 0;
	size_t str_len = wcslen(used_str);
	size_t delim_len = wcslen(delim);

	for (size_t i = 0; i < str_len; ++i)
	{
		bool is_proper_delim = false;

		for (size_t j = 0; j < delim_len; ++j)
		{
			if (used_str[i] == delim[j])
			{
				// Skip beginning delimiters
				if (token_end - token_start == 0)
				{
					++token_start;
					break;
				}

				is_proper_delim = true;
			}
		}

		++token_end;
		if (is_proper_delim && token_end > 0)
		{
			--token_end;
			break;
		}
	}

	if (used_str[token_start] == '\0')
		return NULL;

	if (token_end == 0)
	{
		return &used_str[token_start];
	}

	used_str[token_end] = '\0';
	return &used_str[token_start];
}

wchar_t* wcsncat(wchar_t* dest, const wchar_t* src, size_t n)
{
	size_t dest_length = wcslen(dest);
	size_t i;
	for (i = 0; i < n && src[i] != '\0'; i++)
		dest[dest_length + i] = src[i];
	dest[dest_length + i] = '\0';
	return dest;
}
