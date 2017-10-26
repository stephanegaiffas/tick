#ifndef TICK_BASE_ARRAY_SRC_VECTOR_OPERATIONS_H_
#define TICK_BASE_ARRAY_SRC_VECTOR_OPERATIONS_H_

// License: BSD 3 clause

#include <numeric>

#include "defs.h"
#include "promote.h"

#if   defined(TICK_MKL_AVAILABLE) || defined(TICK_USE_MKL)
  #include "vector/ops_mkl.h"

#elif defined(TICK_CBLAS_AVAILABLE) || defined(TICK_USE_CBLAS)
  #include "vector/blas.h"

#elif defined(TICK_CUDA_AVAILABLE) || defined(TICK_USE_CUDA)
  #include "vector/ops_cuda.h"

#else
  #include "vector/unoptimized.h"

namespace tick{
template<typename T>
using vector_operations = detail::vector_operations_unoptimized<T>;
}

#endif

#endif  // TICK_BASE_ARRAY_SRC_VECTOR_OPERATIONS_H_
