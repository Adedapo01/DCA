// Copyright (C) 2009-2016 ETH Zurich
// Copyright (C) 2007?-2016 Center for Nanophase Materials Sciences, ORNL
// All rights reserved.
//
// See LICENSE.txt for terms of usage.
// See CITATION.txt for citation guidelines if you use this code for scientific publications.
//
// Author: Peter Staar (taa@zurich.ibm.com)
//
// This class implements the delayed 1D discrete NFFT using a FFTW library with the FFTW3 interface.
// It does only 1 FT at the end of the accumulation if the error is not measured.
// See: 2012 J. Phys.: Conf. Ser. 402 012015

#ifndef DCA_MATH_NFFT_DNFFT_1D_HPP
#define DCA_MATH_NFFT_DNFFT_1D_HPP

#include <stdexcept>
#include <vector>

#include <fftw3.h>

#include "dca/function/domains.hpp"
#include "dca/function/function.hpp"
#include "dca/math/nfft/domains/domains.hpp"
#include "dca/math/nfft/nfft_atomic_convolution.hpp"
#include "dca/math/nfft/nfft_mode_names.hpp"
#include "dca/math/nfft/window_functions/gaussian_window_function.hpp"
#include "dca/math/nfft/window_functions/kaiser_bessel_function.hpp"

namespace dca {
namespace math {
namespace nfft {
// dca::math::nfft::

template <typename scalartype, typename w_dmn_t, typename p_dmn_t, int OVER_SAMPLING = 8>
class Dnfft1D {
private:  // Constant parameters.
  // Change this parameter to affect the interpolation of the gaussian kernel.
  // If EXACT is selected the kernel is directly computed. Suggested value = CUBIC.
  constexpr static NfftModeNames NAME = CUBIC;

  constexpr static int WINDOW_SAMPLING = 32;
  constexpr static double SIGMA_WINDOW_SAMPLING = 2;

public:
  using scalar_type = scalartype;
  using this_type = Dnfft1D<scalartype, w_dmn_t, p_dmn_t>;

  Dnfft1D();

  // Resets accumulated object to zero.
  void initialize();

  // Accumulates f_val at continuous time t_val
  // Preconditions : t_val must be scaled in [-0.5, 0,5].
  //                 coor is the linear index of the label domains (e.g. dmn_variadic<Nu, Nu, Rdmn>)
  void accumulate(int coor, scalartype t_val, scalartype f_val);
  // Same as above but with coor storing a multi index.
  void accumulate(const int* coor, scalartype t_val, scalartype f_val);

  // Performs the FFT of the accumulated data.
  // Postcondition : f_w stores the result.
  template <typename other_scalartype>
  void finalize(
      func::function<std::complex<other_scalartype>, func::dmn_variadic<w_dmn_t, p_dmn_t>>& f_w);

  constexpr int getOversamplingFactor() const {
    return OVER_SAMPLING;
  }
  constexpr int getWindowSamplingFactor() const {
    return WINDOW_SAMPLING;
  }
  int getMaximumFrequency() const {
    return w_dmn_t::dmn_size() / 2;
  }

private:
  using window_function_t = kaiser_bessel_function<1>;

  using linear_coefficients_dmn_t = func::dmn_0<nfft_linear_coefficients_domain>;
  using cubic_coefficients_dmn_t = func::dmn_0<nfft_cubic_coefficients_domain>;

  using oversampling_dmn_t = func::dmn_0<nfft_oversampling_domain<this_type>>;
  using window_sampling_dmn_t = func::dmn_0<nfft_window_sampling_domain<this_type>>;

  using padded_time_dmn_t = func::dmn_0<nfft_time_domain<PADDED, this_type>>;
  using left_oriented_time_dmn_t = func::dmn_0<nfft_time_domain<LEFT_ORIENTED, this_type>>;
  using window_function_time_dmn_t = func::dmn_0<nfft_time_domain<WINDOW_FUNCTION, this_type>>;

  using convolution_time_dmn_t = func::dmn_variadic<oversampling_dmn_t, window_sampling_dmn_t>;

  using padded_time_p_dmn_t = func::dmn_variadic<padded_time_dmn_t, p_dmn_t>;
  using left_oriented_time_p_dmn_t = func::dmn_variadic<left_oriented_time_dmn_t, p_dmn_t>;

  void initialize_domains();
  void initialize_functions();

  void convolute_to_f_tau_exact(int index, scalartype t_val, scalartype f_val);
  void convolute_to_f_tau_fine_linear_interpolation(int index, scalartype t_val, scalartype f_val);
  void convolute_to_f_tau_fine_cubic_interpolation(int index, scalartype t_val, scalartype f_val);

  void unroll_linear_interpolation_fast(int N, scalartype* f_tmp_ptr, scalartype* matrix_ptr,
                                        scalartype* y_ptr);
  void unroll_cubic_interpolation_fast(int N, scalartype* f_tmp_ptr, scalartype* matrix_ptr,
                                       scalartype* y_ptr);

  void fold_time_domain_back();

  template <typename other_scalartype>
  void FT_f_tau_to_f_w(
      func::function<std::complex<other_scalartype>, func::dmn_variadic<w_dmn_t, p_dmn_t>>& f_w);

  std::vector<int>& integer_wave_vectors;

  p_dmn_t p_dmn_t_obj;

  func::function<scalartype, padded_time_dmn_t> tau;
  func::function<scalartype, window_function_time_dmn_t> fine_tau;

  func::function<scalartype, padded_time_p_dmn_t> f_tau;
  func::function<scalartype, oversampling_dmn_t> f_tmp;

  func::function<scalartype, left_oriented_time_p_dmn_t> f_tau_left_oriented;
  func::function<std::complex<scalartype>, left_oriented_time_p_dmn_t> f_omega;

  func::function<scalartype, func::dmn_variadic<oversampling_dmn_t, window_sampling_dmn_t>>
      convolution_time_values;
  func::function<scalartype, func::dmn_variadic<oversampling_dmn_t, window_sampling_dmn_t>> window_function;

  func::function<scalartype,
                 func::dmn_variadic<linear_coefficients_dmn_t, oversampling_dmn_t, window_sampling_dmn_t>>
      linear_convolution_matrices;
  func::function<scalartype,
                 func::dmn_variadic<cubic_coefficients_dmn_t, oversampling_dmn_t, window_sampling_dmn_t>>
      cubic_convolution_matrices;

  func::function<scalartype,
                 func::dmn_variadic<oversampling_dmn_t, linear_coefficients_dmn_t, window_sampling_dmn_t>>
      linear_convolution_matrices_2;
  func::function<scalartype,
                 func::dmn_variadic<oversampling_dmn_t, cubic_coefficients_dmn_t, window_sampling_dmn_t>>
      cubic_convolution_matrices_2;

  func::function<scalartype, w_dmn_t> phi_wn;
};

template <typename scalartype, typename w_dmn_t, typename p_dmn_t, int OVER_SAMPLING>
Dnfft1D<scalartype, w_dmn_t, p_dmn_t, OVER_SAMPLING>::Dnfft1D()
    : integer_wave_vectors(w_dmn_t::parameter_type::get_integer_wave_vectors()),

      tau("tau"),
      fine_tau("fine_tau"),

      f_tau("f_tau"),
      f_tmp("f_tmp"),

      f_tau_left_oriented("f_tau_left_oriented"),
      f_omega("f_omega"),

      convolution_time_values("convolution_time_values"),
      window_function("window_function"),

      linear_convolution_matrices("linear_convolution_matrices"),
      cubic_convolution_matrices("cubic_convolution_matrices"),

      linear_convolution_matrices_2("linear_convolution_matrices_2"),
      cubic_convolution_matrices_2("cubic_convolution_matrices_2"),

      phi_wn("phi_wn") {
  initialize_domains();
  initialize_functions();
}

template <typename scalartype, typename w_dmn_t, typename p_dmn_t, int OVER_SAMPLING>
void Dnfft1D<scalartype, w_dmn_t, p_dmn_t, OVER_SAMPLING>::initialize() {
  f_tau = 0.;
}

template <typename scalartype, typename w_dmn_t, typename p_dmn_t, int OVER_SAMPLING>
void Dnfft1D<scalartype, w_dmn_t, p_dmn_t, OVER_SAMPLING>::initialize_domains() {
  oversampling_dmn_t::parameter_type::initialize(*this);
  window_sampling_dmn_t::parameter_type::initialize(*this);

  nfft_time_domain<LEFT_ORIENTED, this_type>::initialize(*this);
  nfft_time_domain<PADDED, this_type>::initialize(*this);
  nfft_time_domain<WINDOW_FUNCTION, this_type>::initialize(*this);
  nfft_time_domain<FOLDED_WINDOW_FUNCTION, this_type>::initialize(*this);

  {
    tau.reset();
    fine_tau.reset();
    convolution_time_values.reset();

    assert(tau.size() == padded_time_dmn_t::dmn_size());
    for (int l = 0; l < tau.size(); l++)
      tau(l) = padded_time_dmn_t::get_elements()[l];

    assert(fine_tau.size() == window_function_time_dmn_t::dmn_size());
    for (int l = 0; l < fine_tau.size(); l++)
      fine_tau(l) = window_function_time_dmn_t::get_elements()[l];

    for (int i = 0; i < oversampling_dmn_t::dmn_size(); i++)
      for (int j = 0; j < window_sampling_dmn_t::dmn_size(); j++)
        convolution_time_values(i, j) =
            nfft_time_domain<WINDOW_FUNCTION,
                             this_type>::get_elements()[j + i * window_sampling_dmn_t::dmn_size()];
  }
}

template <typename scalartype, typename w_dmn_t, typename p_dmn_t, int OVER_SAMPLING>
void Dnfft1D<scalartype, w_dmn_t, p_dmn_t, OVER_SAMPLING>::initialize_functions() {
  {
    window_function_t::n = padded_time_dmn_t::dmn_size();
    window_function_t::m = OVER_SAMPLING;

    window_function_t::sigma = SIGMA_WINDOW_SAMPLING;
  }

  {
    f_tmp.reset();
    f_tau.reset();
    window_function.reset();

    f_tau_left_oriented.reset();
    f_omega.reset();

    linear_convolution_matrices.reset();
    cubic_convolution_matrices.reset();

    linear_convolution_matrices_2.reset();
    cubic_convolution_matrices_2.reset();

    int index = 0;
    scalar_type delta = convolution_time_values(0, 1) - convolution_time_values(0, 0);
    for (int i = 0; i < oversampling_dmn_t::dmn_size(); i++) {
      for (int j = 0; j < window_sampling_dmn_t::dmn_size(); j++) {
        assert(std::abs(convolution_time_values(i, j) - fine_tau(index)) < 1.e-6);

        scalar_type tau = convolution_time_values(i, j);

        scalar_type f0 = window_function_t::phi_t(tau);
        scalar_type f1 = window_function_t::phi_t(tau + delta);

        scalar_type df0 = window_function_t::d_phi_t(tau);
        scalar_type df1 = window_function_t::d_phi_t(tau + delta);

        scalar_type a = f0;
        scalar_type b = df0;

        scalar_type c = -(3. * f0 - 3. * f1 + 2. * df0 * delta + df1 * delta) / std::pow(delta, 2);
        scalar_type d = -(-2. * f0 + 2. * f1 - 1. * df0 * delta - df1 * delta) / std::pow(delta, 3);

        window_function(i, j) = f0;

        linear_convolution_matrices(0, i, j) = f0;
        linear_convolution_matrices(1, i, j) = (f1 - f0) / delta;

        cubic_convolution_matrices(0, i, j) = a;
        cubic_convolution_matrices(1, i, j) = b;
        cubic_convolution_matrices(2, i, j) = c;
        cubic_convolution_matrices(3, i, j) = d;

        linear_convolution_matrices_2(i, 0, j) = f0;
        linear_convolution_matrices_2(i, 1, j) = (f1 - f0) / delta;
        linear_convolution_matrices(1, i, j);

        cubic_convolution_matrices_2(i, 0, j) = a;
        cubic_convolution_matrices_2(i, 1, j) = b;
        cubic_convolution_matrices_2(i, 2, j) = c;
        cubic_convolution_matrices_2(i, 3, j) = d;

        index += 1;
      }
    }
  }

  {
    assert(w_dmn_t::dmn_size() == integer_wave_vectors.size());

    for (int l = 0; l < w_dmn_t::dmn_size(); l++)
      phi_wn(l) = window_function_t::phi_wn(integer_wave_vectors[l]);
  }
}

template <typename scalartype, typename w_dmn_t, typename p_dmn_t, int OVER_SAMPLING>
inline void Dnfft1D<scalartype, w_dmn_t, p_dmn_t, OVER_SAMPLING>::accumulate(int coor,
                                                                             scalartype t_val,
                                                                             scalartype f_val) {
  assert(t_val > -0.5 - 1.e-6 and t_val < 0.5 + 1.e-6);

  switch (NAME) {
    case EXACT:
      convolute_to_f_tau_exact(coor, t_val, f_val);
      break;

    case LINEAR:
      convolute_to_f_tau_fine_linear_interpolation(coor, t_val, f_val);
      break;

    case CUBIC:
      convolute_to_f_tau_fine_cubic_interpolation(coor, t_val, f_val);
      break;

    default:
      throw std::logic_error(__FUNCTION__);
  }
}

template <typename scalartype, typename w_dmn_t, typename p_dmn_t, int OVER_SAMPLING>
inline void Dnfft1D<scalartype, w_dmn_t, p_dmn_t, OVER_SAMPLING>::accumulate(const int* coor,
                                                                             scalartype t_val,
                                                                             scalartype f_val) {
  assert(t_val > -0.5 - 1.e-6 and t_val < 0.5 + 1.e-6);

  int linind = 0;
  p_dmn_t_obj.subind_2_linind(coor, linind);

  switch (NAME) {
    case EXACT:
      convolute_to_f_tau_exact(linind, t_val, f_val);
      break;

    case LINEAR:
      convolute_to_f_tau_fine_linear_interpolation(linind, t_val, f_val);
      break;

    case CUBIC:
      convolute_to_f_tau_fine_cubic_interpolation(linind, t_val, f_val);
      break;

    default:
      throw std::logic_error(__FUNCTION__);
  }
}

template <typename scalartype, typename w_dmn_t, typename p_dmn_t, int OVER_SAMPLING>
template <typename other_scalartype>
void Dnfft1D<scalartype, w_dmn_t, p_dmn_t, OVER_SAMPLING>::finalize(
    func::function<std::complex<other_scalartype>, func::dmn_variadic<w_dmn_t, p_dmn_t>>& f_w) {
  fold_time_domain_back();

  FT_f_tau_to_f_w(f_w);
}

template <typename scalartype, typename w_dmn_t, typename p_dmn_t, int OVER_SAMPLING>
inline void Dnfft1D<scalartype, w_dmn_t, p_dmn_t, OVER_SAMPLING>::convolute_to_f_tau_exact(
    int index, scalartype t_val, scalartype f_val) {
  assert(t_val > -0.5 - 1.e-6 and t_val < 0.5 + 1.e-6);

  const scalartype T_0 = padded_time_dmn_t::parameter_type::first_element();
  const scalartype one_div_Delta = padded_time_dmn_t::parameter_type::get_one_div_Delta();

  int lambda_0 = (t_val - T_0) * one_div_Delta;

  for (int l = -OVER_SAMPLING; l <= OVER_SAMPLING; l++)
    f_tau(lambda_0 + l, index) += f_val * window_function_t::phi_t(tau(lambda_0 + l) - t_val);
}

template <typename scalartype, typename w_dmn_t, typename p_dmn_t, int OVER_SAMPLING>
inline void Dnfft1D<scalartype, w_dmn_t, p_dmn_t,
                    OVER_SAMPLING>::convolute_to_f_tau_fine_linear_interpolation(int index,
                                                                                 scalartype t_val,
                                                                                 scalartype f_val) {
  assert(t_val > -0.5 - 1.e-6 and t_val < 0.5 + 1.e-6);

  const scalartype t_0 = window_function_time_dmn_t::parameter_type::first_element();
  const scalartype T_0 = padded_time_dmn_t::parameter_type::first_element();

  const scalartype one_div_delta = padded_time_dmn_t::parameter_type::get_one_div_delta();
  const scalartype one_div_Delta = padded_time_dmn_t::parameter_type::get_one_div_Delta();

  int tau_0 = (t_val - T_0) * one_div_Delta;
  int tau_1 = (tau(tau_0) - t_val - t_0) * one_div_delta;

  assert(tau(tau_0) - 1.e-10 < t_val && t_val < tau(tau_0 + 1) + 1.e-10);

  scalartype diff_tau = tau(tau_0) - t_val - fine_tau(tau_1);

  assert(diff_tau > -1.e-6 and diff_tau < padded_time_dmn_t::parameter_type::get_delta());

  scalartype y_ptr[2];

  y_ptr[0] = f_val;
  y_ptr[1] = f_val * diff_tau;

  int tau_index = tau_0 - OVER_SAMPLING;
  int delta_tau_index = tau_1 - OVER_SAMPLING * WINDOW_SAMPLING;

  int J = delta_tau_index % WINDOW_SAMPLING;
  int I = (delta_tau_index - J) / WINDOW_SAMPLING;
  assert(delta_tau_index == I * WINDOW_SAMPLING + J);

  scalartype* f_tau_ptr = &f_tau(tau_index, index);
  scalartype* matrix_ptr = &linear_convolution_matrices(0, I, J);

  nfft_atomic_convolution<2 * OVER_SAMPLING + 1, 0>::execute_linear(f_tau_ptr, matrix_ptr, y_ptr);
}

template <typename scalartype, typename w_dmn_t, typename p_dmn_t, int OVER_SAMPLING>
inline void Dnfft1D<scalartype, w_dmn_t, p_dmn_t,
                    OVER_SAMPLING>::convolute_to_f_tau_fine_cubic_interpolation(int index,
                                                                                scalartype t_val,
                                                                                scalartype f_val) {
  assert(t_val > -0.5 - 1.e-6 and t_val < 0.5 + 1.e-6);

  const scalartype t_0 = window_function_time_dmn_t::parameter_type::first_element();
  const scalartype T_0 = padded_time_dmn_t::parameter_type::first_element();

  const scalartype delta = padded_time_dmn_t::parameter_type::get_delta();
  const scalartype Delta = padded_time_dmn_t::parameter_type::get_Delta();

  const scalartype one_div_delta = padded_time_dmn_t::parameter_type::get_one_div_delta();
  const scalartype one_div_Delta = padded_time_dmn_t::parameter_type::get_one_div_Delta();

  int tau_0 = (t_val - T_0) * one_div_Delta;
  scalartype t0_val_lb = T_0 + tau_0 * Delta;

  assert(tau(tau_0) - 1.e-6 < t_val && t_val < tau(tau_0 + 1) + 1.e-6);
  assert(std::abs(tau(tau_0) - t0_val_lb) < 1.e-6);

  // int tau_1 = (tau(tau_0)-t_val-t_0)*one_div_delta;
  int tau_1 = (t0_val_lb - t_val - t_0) * one_div_delta;
  scalartype t1_val_lb = t_0 + tau_1 * delta;

  scalartype diff_tau = t0_val_lb - t_val - t1_val_lb;  // fine_tau(tau_1);

  assert(diff_tau > -1.e-6 and diff_tau < padded_time_dmn_t::parameter_type::get_delta());

  scalartype y_ptr[4];

  y_ptr[0] = f_val;
  y_ptr[1] = y_ptr[0] * diff_tau;
  y_ptr[2] = y_ptr[1] * diff_tau;
  y_ptr[3] = y_ptr[2] * diff_tau;

  {
    int tau_index = tau_0 - OVER_SAMPLING;
    int delta_tau_index = tau_1 - OVER_SAMPLING * WINDOW_SAMPLING;

    int J = delta_tau_index % WINDOW_SAMPLING;
    int I = (delta_tau_index - J) / WINDOW_SAMPLING;
    assert(delta_tau_index == I * WINDOW_SAMPLING + J);

    scalartype* f_tau_ptr = &f_tau(tau_index, index);
    scalartype* matrix_ptr = &cubic_convolution_matrices(0, I, J);

    nfft_atomic_convolution<2 * OVER_SAMPLING + 1, 0>::execute_cubic(f_tau_ptr, matrix_ptr, y_ptr);
  }
}

template <typename scalartype, typename w_dmn_t, typename p_dmn_t, int OVER_SAMPLING>
void Dnfft1D<scalartype, w_dmn_t, p_dmn_t, OVER_SAMPLING>::fold_time_domain_back() {
  f_tau_left_oriented = 0;

  int N_padded = nfft_time_domain<PADDED, this_type>::get_size();
  int N_left_oriented = nfft_time_domain<LEFT_ORIENTED, this_type>::get_size();

  for (int p_ind = 0; p_ind < p_dmn_t::dmn_size(); p_ind++) {
    for (int t_ind = 0; t_ind < N_padded; t_ind++) {
      if (t_ind < 2 * OVER_SAMPLING) {
        f_tau_left_oriented(t_ind - 2 * OVER_SAMPLING + N_left_oriented, p_ind) +=
            f_tau(t_ind, p_ind);
      }

      if (t_ind >= 2 * OVER_SAMPLING and t_ind < N_padded - 2 * OVER_SAMPLING) {
        f_tau_left_oriented(t_ind - 2 * OVER_SAMPLING, p_ind) += f_tau(t_ind, p_ind);
      }

      if (t_ind >= N_padded - 2 * OVER_SAMPLING) {
        f_tau_left_oriented(t_ind - 2 * OVER_SAMPLING - N_left_oriented, p_ind) +=
            f_tau(t_ind, p_ind);
      }
    }
  }
}

template <typename scalartype, typename w_dmn_t, typename p_dmn_t, int OVER_SAMPLING>
template <typename other_scalartype>
void Dnfft1D<scalartype, w_dmn_t, p_dmn_t, OVER_SAMPLING>::FT_f_tau_to_f_w(
    func::function<std::complex<other_scalartype>, func::dmn_variadic<w_dmn_t, p_dmn_t>>& f_w) {
  int N = nfft_time_domain<LEFT_ORIENTED, this_type>::get_size();

  double* f_in = new double[N];
  fftw_complex* f_out = new fftw_complex[N];

  fftw_plan plan = fftw_plan_dft_r2c_1d(N, f_in, f_out, FFTW_ESTIMATE);

  for (int p_ind = 0; p_ind < p_dmn_t::dmn_size(); p_ind++) {
    for (int t_ind = 0; t_ind < N; t_ind++)
      f_in[t_ind] = f_tau_left_oriented(t_ind, p_ind);

    fftw_execute(plan);

    for (int t_ind = 0; t_ind < N / 2; t_ind++) {
      f_omega(t_ind, p_ind).real(-f_out[t_ind][0]);
      f_omega(t_ind, p_ind).imag(f_out[t_ind][1]);
    }

    for (int t_ind = N / 2; t_ind < N; t_ind++) {
      f_omega(t_ind, p_ind).real(-f_out[N - t_ind][0]);
      f_omega(t_ind, p_ind).imag(-f_out[N - t_ind][1]);
    }
  }

  fftw_destroy_plan(plan);

  std::vector<int> w_indices(0);
  for (int w_ind = 0; w_ind < w_dmn_t::dmn_size(); w_ind++) {
    for (int t_ind = 0; t_ind < N; t_ind++) {
      if (integer_wave_vectors[w_ind] == t_ind or integer_wave_vectors[w_ind] + N == t_ind) {
        w_indices.push_back(t_ind);
        break;
      }
    }
  }

  for (int p_ind = 0; p_ind < p_dmn_t::dmn_size(); p_ind++)
    for (int w_ind = 0; w_ind < w_dmn_t::dmn_size(); w_ind++)
      f_w(w_ind, p_ind) = f_omega(w_indices[w_ind], p_ind) / phi_wn(w_ind);

  f_w *= 1. / N;

  delete[] f_in;
  delete[] f_out;
}

}  // nfft
}  // math
}  // dca

#endif  // DCA_MATH_NFFT_DNFFT_1D_HPP
