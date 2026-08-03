#include "CLI/CLIHandler.hpp"
#include "gui/MainWindow.hpp"

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        return RunGUI(argc, argv);
    }
    return RunCLI(argc, argv);
}