// MIT License
// 
// Copyright (c) 2024 Mihail Mladenov
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


#pragma once

#include <Core.hpp>
#include <OS/Thread.hpp>
#include <Math/Algebra.hpp>
#include <Math/CommonFunctions.hpp>
#include <Math/Constants.hpp>
#include <Math/DifferentialEquations/RungeKutta.hpp>


template <typename TRealScalar>
struct RZConfig
{
    using RealScalar = TRealScalar;
    using Scalar = Complex<RealScalar>;
    using State = Vector<Scalar, 2>;
    using Observable = Matrix<Scalar, 2, 2>;

    RZConfig
    (
        RealScalar rabiFreq,
        RealScalar detuning,
        RealScalar pulseWidth,
        const State& initialState,
        RealScalar timeStep,
        RealScalar simulationSpeed,
        U64 totalIterations
    ) :
        rabiFreq(rabiFreq),
        detuning(detuning),
        pulseWidth(pulseWidth),
        initialState(initialState),
        timeStep(timeStep),
        simulationSpeed(simulationSpeed),
        totalIterations(totalIterations)
    {
    }

    auto operator==(const RZConfig& other) const -> Bool
    {
        return
            rabiFreq.Load() == other.rabiFreq.Load() &&
            detuning.Load() == other.detuning.Load() &&
            pulseWidth.Load() == other.pulseWidth.Load() &&
            initialState.Load() == other.initialState.Load() &&
            timeStep.Load() == other.timeStep.Load() &&
            simulationSpeed.Load() == other.simulationSpeed.Load() &&
            totalIterations.Load() == other.totalIterations.Load();
    }

    auto operator!=(const RZConfig& other) const -> Bool = default;

    auto operator=(const RZConfig& other) -> RZConfig&
    {
        rabiFreq = other.rabiFreq.Load();
        detuning = other.detuning.Load();
        pulseWidth = other.pulseWidth.Load();
        initialState = other.initialState.Load();
        timeStep = other.timeStep.Load();
        simulationSpeed = other.simulationSpeed.Load();
        totalIterations = other.totalIterations.Load();

        return *this;
    }

    Atomic<RealScalar> rabiFreq;
    Atomic<RealScalar> detuning;
    Atomic<RealScalar> pulseWidth;
    Atomic<State> initialState;

    Atomic<RealScalar> timeStep;
    Atomic<RealScalar> simulationSpeed;
    Atomic<U64> totalIterations;
};

template <typename TRealScalar>
struct RZSimulator
{
    using RealScalar = TRealScalar;
    using Scalar = Complex<RealScalar>;
    using State = Vector<Scalar, 2>;
    using Observable = Matrix<Scalar, 2, 2>;
    using Solution = Array<Pair<RealScalar, State>>;

    template <typename... TArgs>
    RZSimulator(TArgs&&... args) :
        currentCfg(Forward<TArgs>(args)...),
        cfg(Forward<TArgs>(args)...)
    {
    }

    auto GetHamiltonian(const RealScalar t) -> Observable
    {
        Scalar omegaT = Scalar(0.5 * cfg.rabiFreq.Load() * Sech(t / cfg.pulseWidth.Load()), 0);
        Scalar deltaT = Scalar(cfg.detuning.Load(), 0);
        return Matrix<Scalar, 2, 2>
        (
            Scalar(0), omegaT,
            omegaT, Scalar(cfg.detuning.Load(), 0)
        );
    }

    auto Solve() -> Void
    {
        RefreshCfg();
        ScopedLock<Mutex> lock(solutionMutex);
        solution = SolveRungeKutta
        (
            RealScalar(0),
            cfg.initialState.Load(),
            [&] (RealScalar t, const State& s)
            {
                return Scalar(0, -1) * Constants<RealScalar>::Planck * GetHamiltonian(t) * s;
            },
            cfg.timeStep.Load(),
            cfg.totalIterations.Load() 
        );
    }

    auto GetSolution() const -> LockedPtr<const Solution>
    {
        return LockedPtr(solutionMutex, &solution);
    }

    RZConfig<RealScalar> currentCfg;
private:

    auto RefreshCfg() -> Void
    {
        if (currentCfg != cfg)
        {
            cfg = currentCfg;
        }
    }

    RZConfig<RealScalar> cfg;
    Mutex solutionMutex;
    Solution solution;
};
