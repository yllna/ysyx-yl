#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...)
{
	va_list ap;
	size_t out_idx = 0;
	int d;
	// char c;
	char *s;
	int counter = 0;
	va_start(ap, fmt);
	while (*fmt)
	{
		if ((*fmt) == '%')
		{
			fmt++;
			switch (*fmt)
			{
			case 's': /* string */
				s = va_arg(ap, char *);
				while (s)
				{
					out[out_idx++] = *s++;
				}
				counter++;
				break;
			case 'd': /* int */
				d = va_arg(ap, int);
				char number[15] = {0};
				int number_idx = 0;
				if (d == 0)
				{
					out[out_idx++] = '0';
				}
				else if((unsigned)d == 0x80000000){
					out[out_idx++] = '-';
					unsigned d_ = d;
					while (d_)
					{
						number[number_idx++] = (d_ % 10) + '0';
						d_ /= 10;
					}
					while (number_idx)
					{
						out[out_idx++] = number[number_idx--];
					}
				}
				else
				{
					if (d < 0)
					{
						out[out_idx++] = '-';
						d = -d;
					}
					while (d)
					{
						number[number_idx++] = (d % 10) + '0';
						d /= 10;
					}
					while (number_idx)
					{
						out[out_idx++] = number[number_idx--];
					}
				}
				counter++;
				break;
				// case 'c': /* char */
				// 	/* need a cast here since va_arg only
				//           takes fully promoted types */
				// 	c = (char)va_arg(ap, int);
				// 	break;
			default:
				out[out_idx++] = *fmt;
			}
			fmt++;
		}
		else
		{
			out[out_idx++] = *fmt;
			fmt++;
		}
	}
	va_end(ap);
	return counter;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
