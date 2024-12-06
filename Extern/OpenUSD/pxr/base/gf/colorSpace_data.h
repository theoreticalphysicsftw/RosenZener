// Modified from the original OpenUSD file.
// Copyright 2024 Mihail Mladenov

//
// Copyright 2024 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//

PXR_NAMESPACE_OPEN_SCOPE

struct ColorSpace::_Data
{
    ~_Data() {
        NcFreeColorSpace(colorSpace);
    }

    const NcColorSpace* colorSpace = nullptr;
};

PXR_NAMESPACE_CLOSE_SCOPE
