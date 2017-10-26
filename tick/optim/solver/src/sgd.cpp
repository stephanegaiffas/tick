// License: BSD 3 clause

//
// Created by Martin Bompaire on 22/10/15.
//

#include "sgd.h"

SGD::SGD(ulong epoch_size,
         double tol,
         RandType rand_type,
         double step,
         int seed)
    : StoSolver(epoch_size, tol, rand_type, seed),
      step(step) {}

void SGD::solve() {
    if (model->is_sparse()) {
        solve_sparse();
    } else {
        // Dense case
        ArrayDouble grad(iterate.size());
        grad.init_to_zero();

        const ulong start_t = t;
        for (t = start_t; t < start_t + epoch_size; ++t) {
            const ulong i = get_next_i();
            model->grad_i(i, iterate, grad);
            step_t = get_step_t();
            iterate.mult_incr(grad, -step_t);
            prox->call(iterate, step_t, iterate);
        }
    }
}

void SGD::solve_sparse() {
    // The model is sparse, so it is a ModelGeneralizedLinear and the iteration looks a
    // little bit different
    ulong n_features = model->get_n_features();
    bool use_intercept = model->use_intercept();

    std::vector<double>  steps;
    std::vector<double>  deltas;
    std::vector<BaseArrayDouble> barrays;

    ArrayDouble s_iterate = iterate;

    ulong start_t = t;
    for (t = start_t; t < start_t + epoch_size; ++t) {
        ulong i = get_next_i();
        steps.emplace_back(get_step_t());
        deltas.emplace_back(-step_t * model->grad_i_factor(i, iterate));
        // barrays.emplace_back(model->get_features(i));
        barrays.emplace_back(model->get_features(i));
        if (use_intercept) {
          s_iterate = view(iterate, 0, n_features);
          iterate[n_features] += deltas[deltas.size() - 1];
        }
    }
    for (const auto& barray : barrays) {
        std::cout << "SIZE: " << barray.size() << std::endl;
    }

    std::vector<double*> barraysVP;
    for(const auto& barray : barrays){
      barraysVP.emplace_back(barray.data());
    }
    tick::vector_operations<double>{}.batch_multi_incr(
      steps.size(), barrays[0].size(), deltas.data(), barraysVP.data(), s_iterate.data()
    );

    for (const auto& step : steps) {
        prox->call(iterate, step, iterate);
    }

    // ulong start_t = t;
    // for (t = start_t; t < start_t + epoch_size; ++t) {
    //     ulong i = get_next_i();
    //     // Sparse features vector
    //     BaseArrayDouble x_i = model->get_features(i);
    //     // Gradient factor
    //     double alpha_i = model->grad_i_factor(i, iterate);
    //     // Update the step
    //     double step_t = get_step_t();
    //     double delta = -step_t * alpha_i;
    //     if (use_intercept) {
    //         // Get the features vector, which is sparse here
    //         ArrayDouble iterate_no_interc = view(iterate, 0, n_features);
    //         iterate_no_interc.mult_incr(x_i, delta);
    //         iterate[n_features] += delta;
    //     } else {
    //         // Stochastic gradient descent step
    //         iterate.mult_incr(x_i, delta);
    //     }
    //     // Apply the prox. No lazy-updating here yet
    //     prox->call(iterate, step_t, iterate);
    // }
}

inline double SGD::get_step_t() {
    return step / (t + 1);
}