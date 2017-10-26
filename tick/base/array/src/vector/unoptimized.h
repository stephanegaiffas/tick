#ifndef TICK_BASE_ARRAY_SRC_VECTOR_OPERATIONS_UNOPTIMISED_H_
#define TICK_BASE_ARRAY_SRC_VECTOR_OPERATIONS_UNOPTIMISED_H_

// License: BSD 3 clause
#include <thread>

namespace tick {
namespace detail {

template<typename T>
struct vector_operations_unoptimized {
  T dot(const ulong n, const T *x, const T *y) const {
    T result{0};

    for (ulong i = 0; i < n; ++i) {
      result += x[i] * y[i];
    }

    return result;
  }

  std::vector<T> batch_dot(const ulong b, const ulong n, T* x, T**const y) const {

    std::vector<T> result(b);
    size_t th = 2;
    size_t it = std::floor(b/th); // b = 16 | th = 2 | it = 8
    size_t ma = it * th;

    std::cout << "b  : " << b << std::endl;
    std::cout << "it : " << it << std::endl;
    std::cout << "ma : " << ma << std::endl;

    if(it){
      std::cout << "THREADING" << std::endl;
      std::vector<std::thread> threads;
      for(size_t t = 0; t < th; t++){
        threads.emplace_back([=](const size_t s, const size_t m) mutable -> void {
          for (ulong i = (s * it); i < (s + it); i++) {
            result[i] = dot(n, x, y[i]);
          }
        }, t, ma);
      }
      for(auto& t : threads) t.join();     
    }

    for (ulong i = ma; i < b; ++i) {
      result[i] = dot(n, x, y[i]);
    }    

    return result;
  }

  tick::promote_t<T> sum(const ulong n, const T *x) const {
    return std::accumulate(x, x + n, tick::promote_t<T>{0});
  }

  void scale(const ulong n, const T alpha, T *x) const {
    for (ulong i = 0; i < n; ++i) {
      x[i] *= alpha;
    }
  }

  void set(const ulong n, const T alpha, T *x) const {
    for (ulong i = 0; i < n; ++i) {
      x[i] = alpha;
    }
  }

  void mult_incr(const ulong n, const T alpha, const T *x, T *y) const {
    for (ulong i = 0; i < n; ++i) {
      y[i] += alpha * x[i];
    }
  }
  
  void batch_multi_incr(const ulong b, const ulong n, const T *alpha,  T ** const x, T *y) const {
    size_t it = std::floor(b/8);
    size_t ma = it * 8;

    std::cout << "b  : " << b << std::endl;
    std::cout << "it : " << it << std::endl;
    std::cout << "ma : " << ma << std::endl;

    std::vector<std::thread> threads;
    for(size_t t = 0; t < 8; t++){
      threads.emplace_back([=](const size_t s, const size_t m){
        for (ulong i = s; i < m; i+=8) {
          std::cout << "alpha : " << alpha[i] << std::endl;
          mult_incr(n, alpha[i], x[i], y);
        }
      }, t, ma);
    }
    for(auto& t : threads) t.join();    

    for (ulong i = ma; i < b; ++i) {
      mult_incr(n, alpha[i], x[i], y);
    }
  }

};

}  // namespace detail
}  // namespace tick

#endif  // TICK_BASE_ARRAY_SRC_VECTOR_OPERATIONS_UNOPTIMISED_H_
