#pragma once

namespace Output
{
	void Log(const char* format, ...);
	void Warn(const char* format, ...);
	void Error(const char* format, ...);
	void Fatal(const char* format, int code, ...);
}