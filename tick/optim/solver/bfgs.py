# License: BSD 3 clause

import numpy as np
from scipy.optimize import fmin_bfgs

from tick.optim.prox import ProxZero, ProxL2Sq
from tick.optim.prox.base import Prox
from tick.optim.solver.base import SolverFirstOrder
from tick.optim.solver.base.utils import relative_distance


class BFGS(SolverFirstOrder):
    """
    BFGS (Broyden, Fletcher, Goldfarb, and Shanno ) algorithm.

    This is a simple wrapping of `scipy.optimize.fmin_bfgs`

    Parameters
    ----------
    tol : `float`, default=0.
        The tolerance of the solver (iterations stop when the stopping
        criterion is below it). By default the solver does ``max_iter``
        iterations

    max_iter : `int`, default=100
        Maximum number of iterations of the solver

    verbose : `bool`, default=True
        If `True`, we verbose things, otherwise the solver does not
        print anything (but records information in history anyway)

    print_every : `int`, default=10
        Print history information when ``n_iter`` (iteration number) is
        a multiple of ``print_every``

    record_every : `int`, default=1
        Record history information when ``n_iter`` (iteration number) is
        a multiple of ``record_every``

    Attributes
    ----------
    model : `Model`
        The model to solve

    prox : `Prox`
        Proximal operator to solve

    time_start : `str`
        Start date of the call to ``solve()``

    time_elapsed : `float`
        Duration of the call to ``solve()``, in seconds

    time_end : `str`
        End date of the call to ``solve()``

    References
    ----------
    Quasi-Newton method of Broyden, Fletcher, Goldfarb,
    and Shanno (BFGS), see
    Wright, and Nocedal 'Numerical Optimization', 1999, pg. 198.
    """

    _attrinfos = {
        "_prox_grad": {
            "writable": False
        }
    }

    def __init__(self, tol: float = 0.,
                 max_iter: int = 100, verbose: bool = True,
                 print_every: int = 10, record_every: int = 1):
        SolverFirstOrder.__init__(self, step=None, tol=tol,
                                  max_iter=max_iter, verbose=verbose,
                                  print_every=print_every,
                                  record_every=record_every)
        self._prox_grad = None

    def set_prox(self, prox: Prox):
        """Set proximal operator in the solver.

        Parameters
        ----------
        prox : `Prox`
            The proximal operator of the penalization function

        Returns
        -------
        output : `Solver`
            The solver with given prox

        Notes
        -----
        In some solvers, ``set_model`` must be called before
        ``set_prox``, otherwise and error might be raised.
        """
        if type(prox) is ProxZero:
            SolverFirstOrder.set_prox(self, prox)
            self._set("_prox_grad", lambda x: x)
        elif type(prox) is ProxL2Sq:
            SolverFirstOrder.set_prox(self, prox)
            self._set("_prox_grad", lambda x: prox.strength * x)
        else:
            raise ValueError("BFGS only accepts ProxZero and ProxL2sq "
                             "for now")
        return self

    def solve(self, x0=None):
        """
        Launch the solver

        Parameters
        ----------
        x0 : `np.array`, shape=(n_coeffs,), default=`None`
            Starting point of the solver

        Returns
        -------
        output : `np.array`, shape=(n_coeffs,)
            Obtained minimizer for the problem
        """
        self._start_solve()
        coeffs = self._solve(x0)
        self._set("solution", coeffs)
        self._end_solve()
        return self.solution

    def _solve(self, x0: np.ndarray = None):
        if x0 is None:
            x0 = np.zeros(self.model.n_coeffs)
        obj = self.objective(x0)

        # A closure to maintain history along internal BFGS's iterations
        n_iter = [0]
        prev_x = x0.copy()
        prev_obj = [obj]

        def insp(xk):
            x = xk
            rel_delta = relative_distance(x, prev_x)
            prev_x[:] = x
            obj = self.objective(x)
            rel_obj = abs(obj - prev_obj[0]) / abs(prev_obj[0])
            prev_obj[0] = obj
            self._handle_history(n_iter[0], force=False, obj=obj,
                                 x=xk.copy(),
                                 rel_delta=rel_delta,
                                 rel_obj=rel_obj)
            n_iter[0] += 1

        insp.n_iter = n_iter
        insp.self = self
        insp.prev_x = prev_x
        insp.prev_obj = prev_obj

        # We simply call the scipy.optimize.fmin_bfgs routine
        x_min, f_min, _, _, _, _, _ = \
            fmin_bfgs(lambda x: self.model.loss(x) + self.prox.value(x),
                      x0,
                      lambda x: self.model.grad(x) + self._prox_grad(x),
                      maxiter=self.max_iter, gtol=self.tol,
                      callback=insp, full_output=True,
                      disp=False, retall=False)

        return x_min
