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
        RealScalar timeStart,
        RealScalar timeEnd,
        RealScalar simulationSpeed,
        U64 totalIterations
    ) :
        rabiFreq(rabiFreq),
        detuning(detuning),
        pulseWidth(pulseWidth),
        initialState(initialState),
        timeStart(timeStart),
        timeEnd(timeEnd),
        simulationSpeed(simulationSpeed),
        totalIterations(totalIterations)
    {
    }

    auto operator==(const RZConfig& other) const -> Bool = default;

    auto operator!=(const RZConfig& other) const -> Bool = default;

    RealScalar rabiFreq;
    RealScalar detuning;
    RealScalar pulseWidth;
    State initialState;

    RealScalar timeStart;
    RealScalar timeEnd;
    RealScalar simulationSpeed;
    U64 totalIterations;
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
        cfg(Forward<TArgs>(args)...),
        currentCfg(cfg)
    {
        currentCfg = cfg;
    }

    auto GetHamiltonian(const RealScalar t) -> Observable
    {
        Scalar omegaT = Scalar(0.5 * cfg.rabiFreq * Sech(t / cfg.pulseWidth), 0);
        Scalar deltaT = Scalar(cfg.detuning, 0);
        return Matrix<Scalar, 2, 2>
        (
            Scalar(0), omegaT,
            omegaT, Scalar(cfg.detuning, 0)
        );
    }

    auto Solve() -> Void
    {
        if (!hasInitialSolution || RefreshCfg())
        {
            ScopedLock<Mutex> lock(solutionMutex);
            solution = SolveRungeKutta
            (
                cfg.timeStart,
                cfg.initialState,
                [&](RealScalar t, const State& s)
                {
                    return Scalar(0, -1) * GetHamiltonian(t) * s;
                },
                (cfg.timeEnd - cfg.timeStart) / cfg.totalIterations,
                cfg.totalIterations
            );
            hasInitialSolution = true;
            newSolution = true;
        }
    }

    auto GetSolution() -> LockedPtr<const Solution>
    {
        return LockedPtr<const Solution>(solutionMutex, &solution);
    }

    auto GetSolutionAtTime(RealScalar t) -> State
    {
        if (t < solution[0].first)
        {
            return solution[0].second;
        }
        if (t > solution.GetBack().first)
        {
            return solution.GetBack().second;
        }
        auto first = 0;
        auto range = solution.GetSize();
        while (range > 0)
        {
            auto halfRange = range / 2;

            auto& mid = solution[first + halfRange];
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

        auto val = solution[first].second;
        if (first > 0)
        {
            auto t1 = solution[first].first;
            auto t0 = solution[first - 1].first;
            auto v1 = solution[first].second;
            auto v0 = solution[first - 1].second;
            auto tNorm = Complex<F64>((t - t0) / (t1 - t0));
            val = Lerp(v0, v1, tNorm);
        }
        return val;
    }

    auto GetAnalythicSolutionAtTime(RealScalar t) -> State
    {
        auto ot = cfg.rabiFreq * cfg.pulseWidth;
        auto a = Scalar(0.5 * ot);
        auto b = -a;
        auto c = Scalar(0.5, 0.5 * cfg.detuning * cfg.pulseWidth);
        auto z = Scalar(0.5 * Tanh(t / cfg.pulseWidth) + 0.5);

        // We use analythic expression for the evolution matrix
        auto evolutionMatrix = Observable
        (
            GaussHypergeometric(a, b, c, z),
            GaussHypergeometric(a + Scalar(1) - c, b + Scalar(1) - c , Scalar(2) - c, z),
            (Scalar(0, 2) * a * b) / (c * ot) * Sqrt(z * (Scalar(1) - z)) * GaussHypergeometric(a + Scalar(1), b + Scalar(1), c + Scalar(1), z),
            Exp(Ln(z) * (Scalar(1) - c)) * Sqrt(Scalar(1) - z) * GaussHypergeometric(a + Scalar(1) - c, b + Scalar(1) - c, Scalar(1) - c, z)
        );

        return evolutionMatrix * cfg.initialState;
    }

    auto HasNewSolution() -> Bool
    {
        ScopedLock<Mutex> lock(solutionMutex);
        auto old = newSolution;
        newSolution = false;
        return old;
    }

    Atomic<RZConfig<RealScalar>> currentCfg;
private:

    auto RefreshCfg() -> Bool
    {
        if (cfg != currentCfg.Load())
        {
            cfg = currentCfg.Load();
            return true;
        }
        return false;
    }

    RZConfig<RealScalar> cfg;
    Atomic<Bool> hasInitialSolution = false;
    Bool newSolution = false;
    Mutex solutionMutex;
    Solution solution;
};
