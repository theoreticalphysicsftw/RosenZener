#include <Core.hpp>
#include <OS/Window.hpp>
#include <OS/Thread.hpp>
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
    Thread plottingThread
    (
        [&]() -> Void
        {
            SmoothParametricPlot2D
            (
                img,
                Vector<F64, 2>(0, 0.2),
                Vector<F64, 2>(1, 0.8),
                Vector<F64, 3>(0, -1, -1),
                Vector<F64, 3>(6.28, 1, 1),
                [](F64 t) -> Vector<F64, 2>
                {
                    return Vector<F64, 2>
                    (
                        Sin(Sin(15 * t)) * Cos(0.01 * t) * Cos(10 * t) * Pow(Cos(20 * t), 2.0),
                        Cos(Cos(17 * t)) * Sin(20 * t) * Pow(Sin(Sin(50 * t)), 4.0)
                    );
                },
                Color4(0, 1, 1, 1),
                1.0
            );
        }
    );
    Window::AddToDrawAfterClear(img);

    Window::Loop();
    GThreadPool::ShutDown();
}
