#include <stdio.h>
#include <stdlib.h>

#define ALI_IMPLEMENTATION
#define ALI_REMOVE_PREFIX
#include "ali.h"

typedef enum {
	LOG_INFO = 0,
	LOG_WARN,
	LOG_ERROR,

	LOG_COUNT_
}LogLevel;

static_assert(LOG_COUNT_ == 3, "Log level was added");
const char* loglevel_to_str[LOG_COUNT_] = {
	[LOG_INFO] = "INFO",
	[LOG_WARN] = "WARN",
	[LOG_ERROR] = "ERROR",
};

FILE* ali_global_logfile = NULL;
LogLevel ali_global_loglevel = LOG_INFO;

void log_log(LogLevel level, const char* fmt, ...) {
	ALI_ASSERT(ali_global_logfile != NULL);

	va_list args;
	va_start(args, fmt);

	if (ali_global_loglevel >= level) {
		fprintf(ali_global_logfile, "[%s] ", loglevel_to_str[level]);
		vfprintf(ali_global_logfile, fmt, args);
		fprintf(ali_global_logfile, "\n");
	}

	va_end(args);
}

int main(void) {
	log_log(LOG_INFO, "Hello, World");
	return 0;
}

