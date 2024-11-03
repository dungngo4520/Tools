#include "../Utils/windows/WindowsUtils.h"
#include "../Utils/windows/zw.h"
#include "../Utils/cli/CLI11.hpp"

int wmain(int argc, wchar_t** argv)
{
    DWORD pid = 0;

    CLI::App app("List process handle");
    app.add_option("--pid", pid, "Target process id")->required();
    CLI11_PARSE(app, argc, argv);

    if (library::ntdll::Load() != 0) {
        printf("Load library failed\n");
        return 1;
    }

    return 0;
}
