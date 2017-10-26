#ifndef TICK_BASE_ARRAY_SRC_VECTOR_OPERATIONS_MKL_H_
#define TICK_BASE_ARRAY_SRC_VECTOR_OPERATIONS_MKL_H_

// License: BSD 3 clause

#include "vector/unoptimized.h"

#include "mkl.h"

namespace tick {
namespace detail {

template<typename T>
struct vector_operations_mkl : vector_operations_unoptimized<T> {};

template<typename T>
struct vector_operations_mkl_base {
  promote_t<T> sum(const ulong n, const T *x) const {
    return vector_operations_unoptimized<T>{}.sum(n, x);
  }

  void set(const ulong n, const T alpha, T *x) const {
    return vector_operations_unoptimized<T>{}.set(n, alpha, x);
  }
};

template<>
struct vector_operations_mkl<float> final : public vector_operations_mkl_base<float> {
  float absolute_sum(const ulong n, const float *x) const {
    return cblas_sasum(n, x, 1);
  }

  float dot(const ulong n, const float *x, const float *y) const {
    return cblas_sdot(n, x, 1, y, 1);
  }

  void scale(const ulong n, const float alpha, float *x) const {
    cblas_sscal(n, alpha, x, 1);
  }

  void mult_incr(const ulong n, const float alpha, const float *x, float *y) const {
    cblas_saxpy(n, alpha, x, 1, y, 1);
  }
};

template<>
struct vector_operations_mkl<double> final : public vector_operations_mkl_base<double> {
  double absolute_sum(const ulong n, const double *x) const {
    return cblas_dasum(n, x, 1);
  }

  double dot(const ulong n, const double *x, const double *y) const {
    return cblas_ddot(n, x, 1, y, 1);
  }

  void scale(const ulong n, const double alpha, double *x) const {
    cblas_dscal(n, alpha, x, 1);
  }

  void mult_incr(const ulong n, const double alpha, const double *x, double *y) const {
    cblas_daxpy(n, alpha, x, 1, y, 1);
  }
};

}  // namespace detail

template<typename T>
using vector_operations = detail::vector_operations_mkl<T>;

}  // namespace tick



#endif  // TICK_BASE_ARRAY_SRC_VECTOR_OPERATIONS_MKL_H_