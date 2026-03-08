/**
 * @file   crt_entry.c
 * @brief  Minimal CRT entry point for freestanding Windows executable
 * @detail Calls main() and exits with its return code via ExitProcess.
 */

__declspec(dllimport) void __stdcall ExitProcess(unsigned int uExitCode);

extern int main(void);

void mainCRTStartup(void)
{
    int ret = main();
    ExitProcess((unsigned int)ret);
}
