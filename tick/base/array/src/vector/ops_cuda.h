#ifndef TICK_BASE_ARRAY_SRC_vector_operations_cuda_H_
#define TICK_BASE_ARRAY_SRC_vector_operations_cuda_H_

// License: BSD 3 clause

#include "vector/unoptimized.h"

#include <cuda_runtime.h>
#include <cublas.h>
#include "cublas_v2.h"

namespace tick {
namespace detail {

template<typename T>
struct vector_operations_cuda : vector_operations_unoptimized<T> {};

template<typename T>
struct vector_operations_cuda_base {
  promote_t<T> sum(const ulong n, const T *x) const {
    return vector_operations_unoptimized<T>{}.sum(n, x);
  }

  void set(const ulong n, const T alpha, T *x) const {
    return vector_operations_unoptimized<T>{}.set(n, alpha, x);
  }

  void mult_incr(const ulong n, const T alpha, const T *x, T *y) const {
    for (ulong i = 0; i < n; ++i) {
      y[i] += alpha * x[i];
    }
  }
  void scale(const ulong n, const T alpha, T *x) const {
    for (ulong i = 0; i < n; ++i) {
      x[i] *= alpha;
    }
  }
};

// template<>
// struct vector_operations_cuda<float> final : public vector_operations_cuda_base<float> {
//   float absolute_sum(const ulong n, const float *x) const {
//     return cubas_sasum(n, x, 1);
//   }

//   float dot(const ulong n, const float *x, const float *y) const {
//     return cubas_sdot(n, x, 1, y, 1);
//   }

//   void scale(const ulong n, const float alpha, float *x) const {
//     cubas_sscal(n, alpha, x, 1);
//   }

//   void mult_incr(const ulong n, const float alpha, const float *x, float *y) const {
//     cubas_saxpy(n, alpha, x, 1, y, 1);
//   }
// };

template<>
struct vector_operations_cuda<double> final : public vector_operations_cuda_base<double> {
  // double absolute_sum(const ulong n, const double *x) const {
  //   return cubas_dasum(n, x, 1);
  // }

  double dot(const ulong n, const double *x, const double *y) const {

    cublasInit();

    double *d_x, *d_y, *result_, result;
    const size_t sz = sizeof(double) * (size_t)n;

    // cublasAlloc(n, sizeof(double), (void**) &d_x);
    // cublasAlloc(n, sizeof(double), (void**) &d_y);

    // cublasSetVector(n, sizeof(x[0]), x, 1, d_x, 1);
    // cublasSetVector(n, sizeof(y[0]), x, 1, d_y, 1);

    cudaMalloc( (void **)(&d_x), sz);
    cudaMalloc( (void **)(&d_y), sz);
    cudaMalloc( (void **)(&result_), sizeof(double) );

    cudaMemcpy(d_x, x, sz, cudaMemcpyHostToDevice);
    cudaMemcpy(d_y, y, sz, cudaMemcpyHostToDevice);

    cublasHandle_t h;
    cublasCreate(&h);
    cublasSetPointerMode(h, CUBLAS_POINTER_MODE_DEVICE);

    cublasDdot(h, n, d_x, 1, d_y, 1, result_);

    cudaMemcpy(&result, result_, sizeof(double), cudaMemcpyDeviceToHost);

    cublasShutdown();

    return result;
  }

  // void scale(const ulong n, const double alpha, double *x) const {
  //   cubas_dscal(n, alpha, x, 1);
  // }

  // void mult_incr(const ulong n, const double alpha, const double *x, double *y) const {
  //   cubas_daxpy(n, alpha, x, 1, y, 1);
  // }
};

}  // namespace detail

template<typename T>
using vector_operations = detail::vector_operations_cuda<T>;

}  // namespace tick

#endif  // TICK_BASE_ARRAY_SRC_vector_operations_cuda_H_
