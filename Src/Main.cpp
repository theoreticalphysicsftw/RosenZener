#include <Core.hpp>
#include <OS/Window.hpp>
#include <OS/Thread.hpp>
#include <GUI/Basic.hpp>
#include <GUI/BasicWidgets.hpp>
#include <Math/Algebra/Matrix.hpp>
#include <Image/RawCPUImage.hpp>
#include <RZSimulator.hpp>
#include <Rendering/Plot.hpp>

using RZSim = RZSimulator<F64>;
using Scalar = RZSim::Scalar;
using RealScalar = RZSim::RealScalar;
using State = RZSim::State;
using Observable = RZSim::Observable;

static constexpr U32 initDisplayW = 1024;
static constexpr U32 initDisplayH = 512;

GUITexture gGraph;
RawCPUImage gGraphRaw;

static Atomic<ColorU32> gE0Color = ColorU32(0xFFFFFF00);
static Atomic<ColorU32> gE1Color = ColorU32(0xFFFF00FF);

static constexpr Color4 cLightBackground = Color4(1, 252.f/255.f, 249.f/255.f, 1);
static constexpr Color4 cDarkBackground = Color4(0, 0, 0, 1);

static Bool gIsLightTheme = false;

static Atomic<Bool> gReplotNeeded = false;
static Atomic<Bool> gPlotAnalythic = false;


RZSimulator<F64> gSimulator
(
    16.0,
    0.5,
    2.0,
    State(Scalar(1.0), Scalar(0.0)),
    -10,
    10,
    1.0,
    4096
);

Void DisplayState()
{
    auto& io = ImGui::GetIO();
    auto bgDrawList = ImGui::GetBackgroundDrawList();
    auto cfg = gSimulator.currentCfg.Load();

    auto rfVal = Format("{} = {:.2f}", "\xCE\xA9\xE2\x82\x80", cfg.rabiFreq);
    auto dVal = Format("{} = {:.2f}", "\xCE\x94\xE2\x82\x80", cfg.detuning);
    auto pwVal = Format("{} = {:.2f}", "T", cfg.pulseWidth);
    auto textColor = gIsLightTheme ? ColorU32(cDarkBackground) : ColorU32(cLightBackground);
    bgDrawList->AddText(ImVec2(io.DisplaySize.x * 0.75f, 0.025f * io.DisplaySize.y), textColor, rfVal.ToCStr());
    bgDrawList->AddText(ImVec2(io.DisplaySize.x * 0.75f, 0.025f * io.DisplaySize.y + 20), textColor, dVal.ToCStr());
    bgDrawList->AddText(ImVec2(io.DisplaySize.x * 0.75f, 0.025f * io.DisplaySize.y + 40), textColor, pwVal.ToCStr());
}

Void GuiAccumulator()
{
    static Bool screenShotFrame = false;

    auto& io = ImGui::GetIO();
    auto aspectRatio = F32(io.DisplaySize.x) / F32(io.DisplaySize.y);

    const auto windowFlags =
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoMove;
    const auto windowOffset = io.DisplaySize.x * 0.025f;
    static Bool open = false;

    auto cfg = gSimulator.currentCfg.Load();

    if (!screenShotFrame)
    {
        ImGui::Begin("Options", &open, windowFlags);
        ImGui::SetWindowPos(ImVec2(windowOffset, windowOffset));

        if (ImGui::Checkbox("Light theme", &gIsLightTheme))
        {
            gReplotNeeded = true;
        }
        if (gIsLightTheme)
        {
            ImGui::StyleColorsLight(&ImGui::GetStyle());
            Window::SetClearColor(cLightBackground);
        }
        else
        {
            ImGui::StyleColorsDark(&ImGui::GetStyle());
            Window::SetClearColor(cDarkBackground);
        }

        //ImGui::Text("%.1f ms/frame (%.1f FPS)", 1000.f / io.Framerate, io.Framerate);

        ImGui::PushItemWidth(io.DisplaySize.x * 0.14f);
        ImGui::InputDouble("\xCE\xA9\xE2\x82\x80 - Rabi frequency", &cfg.rabiFreq, 0.1);
        ImGui::InputDouble("\xCE\x94\xE2\x82\x80 - detuning", &cfg.detuning, 0.1);
        ImGui::InputDouble("T - pulse width", &cfg.pulseWidth, 0.1);
        ImGui::InputDouble("## Iitial State 0.0", (F64*)&cfg.initialState[0], 0.1);
        ImGui::SameLine();
        ImGui::InputDouble("## Initial State 0.1", (F64*)&cfg.initialState[0] + 1, 0.1);
        ImGui::SameLine();
        ImGui::InputDouble("## Initial State 1.0", (F64*)&cfg.initialState[1], 0.1);
        ImGui::SameLine();
        ImGui::InputDouble("\xCE\xA8 - initial state", (F64*)&cfg.initialState[1] + 1, 0.1);

        ImGui::InputDouble("t\xE2\x82\x80 - start time", &cfg.timeStart, 0.1);
        ImGui::InputDouble("t\xE2\x82\x81 - end time", &cfg.timeEnd, 0.1);
        ImGui::PopItemWidth();

        static Bool analythicModel = gPlotAnalythic.Load();
        if (ImGui::Checkbox("Use analythic model", &analythicModel))
        {
            gPlotAnalythic = analythicModel;
            gReplotNeeded = true;
        }

        if (ImGui::Button("Take screenshot"))
        {
            screenShotFrame = true;
        }

        ImGui::End();
    }
    else
    {
        screenShotFrame = false;
        Window::takeScreenShotThisFrame = true;
    }

    auto bgDrawList = ImGui::GetBackgroundDrawList();
    gGraph.UpdateFromLebesgueRGBA8(gGraphRaw);
    bgDrawList->AddImage(gGraph.id, ImVec2(0, 0), ImVec2(gGraph.width, gGraph.height));

    DisplayState();

    PlotEuclideanCoordinateFrame2D
    (
        Vector2(1 * cfg.timeStart, 0),
        Vector2(1 * cfg.timeEnd, 1),
        Vector2(0 * io.DisplaySize.x, 0.8 * io.DisplaySize.y),
        Vector2(1 * io.DisplaySize.x, 0.2 * io.DisplaySize.y),
        Vector2((cfg.timeEnd + cfg.timeStart) / 2, 0),
        "",
        "",
        "t",
        "",
        10,
        10,
        gIsLightTheme? ColorU32(cDarkBackground) : ColorU32(cLightBackground),
        2.0
    );

    gSimulator.currentCfg = cfg;


    ImGui::Begin("## Legend", nullptr, windowFlags | ImGuiWindowFlags_NoDecoration);
    ImGui::SetWindowPos(ImVec2(windowOffset, 0.85 * io.DisplaySize.y));
    ImColor c0 = As<ImColor>(gE0Color.Load().operator Color4());
    ImColor c1 = As<ImColor>(gE1Color.Load().operator Color4());
    if (ImGui::ColorEdit3("probability of measuring the first energy level", As<F32*>(&c0), ImGuiColorEditFlags_NoInputs))
    {
        gReplotNeeded = true;
    }
    if (ImGui::ColorEdit3("probability of measuring the second energy level", As<F32*>(&c1), ImGuiColorEditFlags_NoInputs))
    {
        gReplotNeeded = true;
    }
    gE0Color = ColorU32((U32)c0);
    gE1Color = ColorU32((U32)c1);

    ImGui::End();
}

int main()
{
    GThreadPool::Init();
    Window::Init("RosenZener", initDisplayW, initDisplayH, true);
    GUI::Init();
    GUI::AddCmdAccumulator(GuiAccumulator);

    Thread rzThread
    (
        [&]() -> Void
        {
            while (!Window::isClosed)
            {
                gSimulator.Solve();
            }
        }
    );

    gGraphRaw.Init(initDisplayW, initDisplayH, EFormat::RGBA8, true);
    gGraph.Init(initDisplayW, initDisplayH);
    Thread plottingThread
    (
        [&]() -> Void
        {
            while (!Window::isClosed)
            {
                if (gSimulator.HasNewSolution() || gReplotNeeded.Load())
                {
                    gReplotNeeded = false;
                    auto solution = gSimulator.GetSolution();
                    gGraphRaw.Clear<U32>(gIsLightTheme? ColorU32(cLightBackground) : ColorU32(cDarkBackground));
                    SmoothPlot2D
                    (
                        gGraphRaw,
                        Vector<F64, 2>(0, 0.2),
                        Vector<F64, 2>(1, 0.8),
                        Vector<F64, 2>(gSimulator.currentCfg.Load().timeStart, 0),
                        Vector<F64, 2>(gSimulator.currentCfg.Load().timeEnd, 1),
                        [&](F64 t) -> F64
                        {
                            auto val = gPlotAnalythic.Load()? gSimulator.GetAnalythicSolutionAtTime(t) : gSimulator.GetSolutionAtTime(t);
                            auto probability = GetNorm(val[0]);
                            return probability;
                        },
                        gE0Color.Load().operator Color4(),
                        1.0
                    );
                    SmoothPlot2D
                    (
                        gGraphRaw,
                        Vector<F64, 2>(0, 0.2),
                        Vector<F64, 2>(1, 0.8),
                        Vector<F64, 2>(gSimulator.currentCfg.Load().timeStart, 0),
                        Vector<F64, 2>(gSimulator.currentCfg.Load().timeEnd, 1),
                        [&](F64 t) -> F64
                        {
                            auto val = gPlotAnalythic.Load() ? gSimulator.GetAnalythicSolutionAtTime(t) : gSimulator.GetSolutionAtTime(t);
                            auto probability = GetNorm(val[1]);
                            return probability;
                        },
                        gE1Color.Load().operator Color4(),
                        1.0
                    );
                }
            }
        }
    );

    Window::Loop();
    GThreadPool::ShutDown();
}
