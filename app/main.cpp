#include "CLI/CLIHandler.hpp"
#include "gui/MainWindow.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char **argv)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    std::ios::sync_with_stdio(false);

    if (argc == 1)
    {
        return RunGUI(argc, argv);
    }
    return RunCLI(argc, argv);
}