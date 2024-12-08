#include <Core.hpp>
#include <OS/Window.hpp>
#include <GUI/Basic.hpp>

int main()
{
    Window::Init("RosenZener", 1024, 512, false);
    GUI::Init();
    Window::Loop();
}
