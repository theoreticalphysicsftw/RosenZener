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
#include <Math/Algebra.hpp>
#include <Math/CommonFunctions.hpp>
#include <Math/Constants.hpp>


template <typename TRealScalar>
struct RZSimulator
{
    using RealScalar = TRealScalar;
    using Scalar = Complex<RealScalar>;
    using State = Vector<Scalar, 2>;
    using Observable = Matrix<Scalar, 2, 2>;

    RZSimulator
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

    Observable GetHamiltonian(const RealScalar t)
    {
        RealScalar omegaT = 0.5 * rabiFreq * Sech(t / pulseWidth);
        RealScalar deltaT = detuning;
        return Matrix
        (
            RealScalar(0), omegaT,
            omegaT, detuning
        );
    }

    auto Solve() -> Void
    {
        solution = SolveRungeKutta
        (
            RealScalar(0),
            initialState,
            [&] (RealScalar t, const State& s)
            {
                return Scalar(0, -1) * Constants<RealScalar>::Planck * GetHamiltionian(t) * s;
            },
            timeStep,
            totalIterations 
        );
    }

    Array<Pair<RealScalar, State>> solution;

private:
    RealScalar rabiFreq;
    RealScalar detuning;
    RealScalar pulseWidth;
    State initialState;

    RealScalar timeStep;
    RealScalar simulationSpeed;
    U64 totalIterations;
};
