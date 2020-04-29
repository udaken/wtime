#include <windows.h>
#include <stdio.h>
#include <corecrt_startup.h>

#define FT_MILISEC ((ULONGLONG)10000)
#define FT_SECOND ((ULONGLONG)10000000)
#define FT_MINUTE (60 * FT_SECOND)

static ULONGLONG ft2ull(const FILETIME * pft)
{
	ULARGE_INTEGER a = { pft->dwLowDateTime, pft->dwHighDateTime };
	return a.QuadPart;
}

static ULONGLONG subft(const FILETIME *lhs, const FILETIME *rhs)
{
	ULARGE_INTEGER a = { lhs->dwLowDateTime, lhs->dwHighDateTime };
	ULARGE_INTEGER b = { rhs->dwLowDateTime, rhs->dwHighDateTime };
	// ULARGE_INTEGER c{};
	// c.QuadPart = a.QuadPart - b.QuadPart;
	// return FILETIME{c.LowPart, c.HighPart};
	return a.QuadPart - b.QuadPart;
}

int wmain(int argc, wchar_t* argv[])
{
	wchar_t* lpCmdLine = _get_wide_winmain_command_line();

	if (argc < 2)
	{
		fprintf(stderr, "usage %S command [...]\n", argv[0]);
		return EXIT_FAILURE;
	}

	STARTUPINFO si = { sizeof(STARTUPINFO) };
	PROCESS_INFORMATION pi = {0};
	if (!CreateProcess(NULL, lpCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{
		fprintf(stderr, "%S: CreateProcess failed.(%u)\n", argv[0], GetLastError());
		return EXIT_FAILURE;
	}

	if (WAIT_FAILED == WaitForSingleObject(pi.hProcess, INFINITE))
	{
		fprintf(stderr, "%S: WaitForSingleObject failed.(%u)\n", argv[0], GetLastError());
	}

	FILETIME ct, et, kt, ut;
	if (!GetProcessTimes(pi.hProcess, &ct, &et, &kt, &ut))
	{
		fprintf(stderr, "%S: GetProcessTimes failed.(%u)\n", argv[0], GetLastError());
	}

	{
		ULONGLONG real = subft(&et,&ct);
		ULONGLONG real_in_ms = real / FT_MILISEC;
		ULONGLONG real_in_sec = real / FT_SECOND;
		ULONGLONG real_in_min = real / FT_MINUTE;
		fprintf(stderr, "real\t""%llu""m""%u"".""%03u""s\n",
			real_in_min, 
			(unsigned)(real_in_sec % 60),
			(unsigned)(real_in_ms % 1000));
	}
	{
		ULONGLONG k = ft2ull(&kt);
		ULONGLONG k_in_ms = k / FT_MILISEC;
		ULONGLONG k_in_sec = k / FT_SECOND;
		ULONGLONG k_in_min = k / FT_MINUTE;
		fprintf(stderr, "kernel\t""%llu""m""%u"".""%03u""s\n",
			k_in_min, 
			(unsigned)(k_in_sec % 60), 
			(unsigned)(k_in_ms % 1000));
	}
	{
		ULONGLONG u = ft2ull(&ut);
		ULONGLONG u_in_ms = u / FT_MILISEC;
		ULONGLONG u_in_sec = u / FT_SECOND;
		ULONGLONG u_in_min = u / FT_MINUTE;
		fprintf(stderr, "user\t""%llu""m""%u"".""%03u""s\n", 
			u_in_min, 
			(unsigned)(u_in_sec % 60), 
			(unsigned)(u_in_ms % 1000));
	}

	DWORD exitCode = EXIT_FAILURE;
	if (!GetExitCodeProcess(pi.hProcess, &exitCode))
	{
		fprintf(stderr, "%S: GetExitCodeProcess failed.(%u)\n", argv[0], GetLastError());
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return exitCode;
}
