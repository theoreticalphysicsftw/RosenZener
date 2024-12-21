#include <Core.hpp>
#include <OS/Window.hpp>
#include <GUI/Basic.hpp>
#include <Math/Algebra/Matrix.hpp>
#include <Image/RawCPUImage.hpp>
#include <RZSimulator.hpp>
#include <Rendering/Plot.hpp>

int main()
{
    GThreadPool::Init();
    Window::Init("RosenZener", 1024, 512, false);
    GUI::Init();

    RawCPUImage img(1024, 512, EFormat::RGBA8, true);
    SmoothPlot2D
    (
        img,
        Vector2(0, 0.3),
        Vector2(1, 0.5),
        Vector2(0, -1),
        Vector2(1, 1),
        [](F32 t) -> F32 { return Sin(200 * t) + Sin(500 * t + 200); },
        Color4(0, 1, 1, 1),
        1.f
    );
    Window::AddToDrawAfterClear(img);

    Window::Loop();
    GThreadPool::ShutDown();
}
