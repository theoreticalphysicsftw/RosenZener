#include <Core.hpp>
#include <OS/Window.hpp>
#include <OS/Thread.hpp>
#include <GUI/Basic.hpp>
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

RZSimulator<F64> gSimulator
(
    16.0,
    2.0,
    2.0,
    State(Scalar(1.0), Scalar(0.0)),
    -10.0,
    10.0,
    1.0,
    4096
);

Void GuiAccumulator()
{
	auto& io = ImGui::GetIO();
	auto aspectRatio = F32(io.DisplaySize.x) / F32(io.DisplaySize.y);

	const auto windowFlags =
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoMove;
	const auto windowOffset = io.DisplaySize.x * 0.025f;

    ImGui::Begin("Options", nullptr, windowFlags);
	ImGui::SetWindowPos(ImVec2(windowOffset, windowOffset));

    ImGui::Text("%.1f ms/frame (%.1f FPS)", 1000.f / io.Framerate, io.Framerate);

    auto cfg = gSimulator.currentCfg.Load();

    ImGui::PushItemWidth(io.DisplaySize.x * 0.14f);
    ImGui::InputDouble("Rabi Frequency", &cfg.rabiFreq, 0.1);
    ImGui::InputDouble("Detuning", &cfg.detuning, 0.1);
    ImGui::InputDouble("Pulse Width", &cfg.pulseWidth, 0.1);
    ImGui::InputDouble("## Initial State 0.0", (F64*) &cfg.initialState[0], 0.1);
    ImGui::SameLine();
    ImGui::InputDouble("## Initial State 0.1", (F64*)&cfg.initialState[0] + 1, 0.1);
    ImGui::SameLine();
    ImGui::InputDouble("## Initial State 1.0", (F64*)&cfg.initialState[1], 0.1);
    ImGui::SameLine();
    ImGui::InputDouble("Initial State", (F64*)&cfg.initialState[1] + 1, 0.1);

    ImGui::InputDouble("Time Start", &cfg.timeStart, 0.1);
    ImGui::InputDouble("Time End", &cfg.timeEnd, 0.1);
    ImGui::PopItemWidth();

    auto bgDrawList = ImGui::GetBackgroundDrawList();
    gGraph.UpdateFromLebesgueRGBA8(gGraphRaw);
    bgDrawList->AddImage(gGraph.id, ImVec2(0, 0), ImVec2(gGraph.width, gGraph.height));

    gSimulator.currentCfg = cfg;

    ImGui::End();
}

int main()
{
    GThreadPool::Init();
    Window::Init("RosenZener", initDisplayW, initDisplayH, false);
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
                if (gSimulator.HasNewSolution())
                {
                    auto solution = gSimulator.GetSolution();
                    gGraphRaw.Clear<U32>();
                    SmoothPlot2D
                    (
                        gGraphRaw,
                        Vector<F64, 2>(0, 0.2),
                        Vector<F64, 2>(1, 0.8),
                        Vector<F64, 2>(gSimulator.currentCfg.Load().timeStart, 0),
                        Vector<F64, 2>(gSimulator.currentCfg.Load().timeEnd, 1),
                        [&](F64 t) -> F64
                        {
                            if (t < (*solution)[0].first)
                            {
                                return GetNorm((*solution)[0].second[0]);
                            }
                            if (t > solution->GetBack().first)
                            {
                                return GetNorm(solution->GetBack().second[0]);
                            }
                            auto first = 0;
                            auto range = solution->GetSize();
                            while (range > 0)
                            {
                                auto halfRange = range / 2;

                                auto& mid = (*solution)[first + halfRange];
                                if (mid.first < t)
                                {
                                    first = first + halfRange + 1;
                                    range = range - halfRange - 1;
                                }
                                else
                                {
                                    range = halfRange;
                                }
                            }

                            auto& val = (*solution)[first];
                            auto probability = GetNorm(val.second[0]);
                            return probability;
                        },
                        Color4(0, 1, 1, 1),
                        1.0
                    );
                }
            }
        }
    );

    Window::Loop();
    GThreadPool::ShutDown();
}
