#include <Core.hpp>
#include <OS/Window.hpp>
#include <GUI/Basic.hpp>
#include <Math/Algebra/Matrix.hpp>
#include <Image/RawCPUImage.hpp>


int main()
{
    GThreadPool::Init();
    Window::Init("RosenZener", 1024, 512, false);
    GUI::Init();
    Window::Loop();
    GThreadPool::ShutDown();
}
