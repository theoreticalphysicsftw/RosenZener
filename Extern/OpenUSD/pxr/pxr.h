// Contains modified code from OpenUSD. 

// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef PXR_H
#define PXR_H


#define PXR_USE_NAMESPACES 0

#if PXR_USE_NAMESPACES

#define PXR_NS pxr
#define PXR_INTERNAL_NS pxr_internal__pxrReserved__
#define PXR_NS_GLOBAL ::PXR_NS

namespace PXR_INTERNAL_NS { }

// The root level namespace for all source in the USD distribution.
namespace PXR_NS {
    using namespace PXR_INTERNAL_NS;
}

#define PXR_NAMESPACE_OPEN_SCOPE   namespace PXR_INTERNAL_NS {
#define PXR_NAMESPACE_CLOSE_SCOPE  }  
#define PXR_NAMESPACE_USING_DIRECTIVE using namespace PXR_NS;

#else

#define PXR_NS 
#define PXR_NS_GLOBAL 
#define PXR_NAMESPACE_OPEN_SCOPE   
#define PXR_NAMESPACE_CLOSE_SCOPE 
#define PXR_NAMESPACE_USING_DIRECTIVE

#endif // PXR_USE_NAMESPACES
#endif //PXR_H
