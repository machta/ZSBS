
// Include Files
#include "xnrm2.h"
#include "rt_nonfinite.h"
#include "spike_cluster_cpp.h"

// Function Definitions

//
// Arguments    : int n
//                const emxArray_real_T *x
//                int ix0
// Return Type  : double
//
double b_xnrm2(int n, const emxArray_real_T *x, int ix0) {
  double y;
  double scale;
  int kend;
  int k;
  double absxk;
  double t;
  y = 0.0;
  if (!(n < 1)) {
    if (n == 1) {
      y = std::abs(x->data[ix0 - 1]);
    } else {
      scale = 2.2250738585072014E-308;
      kend = (ix0 + n) - 1;
      for (k = ix0; k <= kend; k++) {
        absxk = std::abs(x->data[k - 1]);
        if (absxk > scale) {
          t = scale / absxk;
          y = 1.0 + y * t * t;
          scale = absxk;
        } else {
          t = absxk / scale;
          y += t * t;
        }
      }

      y = scale * std::sqrt(y);
    }
  }

  return y;
}

//
// Arguments    : int n
//                const emxArray_real_T *x
//                int ix0
// Return Type  : double
//
double xnrm2(int n, const emxArray_real_T *x, int ix0) {
  double y;
  double scale;
  int kend;
  int k;
  double absxk;
  double t;
  y = 0.0;
  if (!(n < 1)) {
    if (n == 1) {
      y = std::abs(x->data[ix0 - 1]);
    } else {
      scale = 2.2250738585072014E-308;
      kend = (ix0 + n) - 1;
      for (k = ix0; k <= kend; k++) {
        absxk = std::abs(x->data[k - 1]);
        if (absxk > scale) {
          t = scale / absxk;
          y = 1.0 + y * t * t;
          scale = absxk;
        } else {
          t = absxk / scale;
          y += t * t;
        }
      }

      y = scale * std::sqrt(y);
    }
  }

  return y;
}
