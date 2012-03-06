/* prototypes for the mini printf, copied from pptp */

#include <stdarg.h>

int pp_vprintf(const char *fmt, va_list args);
int pp_printf(const char *fmt, ...);
int pp_vsprintf(char *buf, const char *fmt, va_list args);

#define pp_puts dbgu_print
