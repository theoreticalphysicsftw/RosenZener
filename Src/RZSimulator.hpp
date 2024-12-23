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
        RealScalar simulationSpeed
    ) :
        rabiFreq(rabiFreq),
        detuning(detuning),
        pulseWidth(pulseWidth),
        currentState(initialState),
        initialState(initialState)
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

private:
    RealScalar rabiFreq;
    RealScalar detuning;
    RealScalar pulseWidth;
    State currentState;
    State initialState;

    RealScalar timeStep;
    RealScalar simulationSpeed;
};
