#include "pch-il2cpp.h"
#include "Output.h"

#include <Windows.h>
#include <thread>
#include <cstdarg>
#include <cstdio>
#include <stdlib.h>

static void _PrintTime() {
	SYSTEMTIME st;
	GetLocalTime(&st);
	printf("[%02d:%02d:%02d.%03d] ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
}

void Output::Log(const char* format, ...) {
	_PrintTime();
	printf("[INFO] ");
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("\n");
}
void Output::Warn(const char* format, ...) {
	_PrintTime();
	printf("[WARNING] ");
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("\n");
}
void Output::Error(const char* format, ...) {
	_PrintTime();
	printf("[ERROR] ");
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("\n");
}
void Output::Fatal(const char* format, int code, ...) {
	_PrintTime();
	printf("[FATAL ERROR] ");
	va_list args;
	va_start(args, code);
	vprintf(format, args);
	va_end(args);
	printf("\n");
	std::this_thread::sleep_for(std::chrono::seconds(5));
	exit(code);
}
