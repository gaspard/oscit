/*
  ==============================================================================

   This file is part of the OSCIT library (http://rubyk.org/liboscit)
   Copyright (c) 2007-2010 by Gaspard Bucher - Buma (http://teti.ch).

  ------------------------------------------------------------------------------

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.

  ==============================================================================
*/

#include "oscit/matrix.h"

#include <vector>

#include "oscit/values.h"


namespace oscit {

/* ============================================= Value ========= */

// -------------------------------------------------------------
Value::Value(size_t rows, size_t cols, size_t type, void *data, size_t step)
    : type_(MATRIX_VALUE) {
  matrix_ = new Matrix(rows, cols, type, data, step);
}

// -------------------------------------------------------------
Value::Value(size_t rows, size_t cols)
    : type_(MATRIX_VALUE) {
#if Real == double
  matrix_ = new Matrix(rows, cols, CV_64FC1);
#else
  matrix_ = new Matrix(rows, cols, CV_32FC1);
#endif
}

// -------------------------------------------------------------
Value::Value(size_t rows, size_t cols, int type)
    : type_(MATRIX_VALUE) {
  matrix_ = new Matrix(rows, cols, type);
}

// -------------------------------------------------------------
void Value::set_matrix(const Matrix *matrix) {
  if (matrix->data && !matrix->refcount) {
    // Trying to copy a user-allocated data matrix without reference count
    matrix_ = new Matrix(); // dummy
  } else {
    matrix_ = new Matrix(*matrix);
  }
}

// -------------------------------------------------------------
size_t Value::mat_size() const {
  return is_matrix() ? matrix_->rows * matrix_->cols : 0;
}

// -------------------------------------------------------------
int Value::mat_type() const {
  return is_matrix() ? matrix_->type() : 0;
}

// -------------------------------------------------------------
void *Value::mat_data() const {
  return is_matrix() ? matrix_->data : NULL;
}

// -------------------------------------------------------------
Matrix *Value::build_matrix() {
  return new Matrix();
}

// -------------------------------------------------------------
void Value::delete_matrix() {
  delete matrix_;
}

// -------------------------------------------------------------

template<class T>
static std::ostream &out_matrix(std::ostream &out_stream, const Matrix &mat, const char *format) {
  char buf[200];
  size_t channels = mat.channels();
  size_t rows = mat.rows;
  size_t cols = mat.cols;
  size_t row_step = mat.step1();
  T *row_start;
  T *data = (T*)mat.data;

  for (size_t j = 0; j < rows; ++j) {
    row_start = data + j * row_step;
    if (j == 0) {
		  out_stream << "[";
    } else {
      out_stream << " ";
    }

    for (size_t i = 0; i < cols; ++i) {
      if (channels > 1) {
        for (size_t c = 0; c < channels; ++c) {
          if (c == 0) {
      		  out_stream << "{";
          } else {
            out_stream << " ";
          }

          snprintf(buf, 200, format, *(row_start + i * channels + c));
          out_stream << buf;
        }
        out_stream << "}";
      } else {
        snprintf(buf, 200, format, *(row_start + i));
        out_stream << buf;
      }
    }

    if (j == rows -1) {
      out_stream << " ]\n";
    } else {
      out_stream << "\n";
    }
  }
  return out_stream;
}

std::ostream &operator<< (std::ostream &out_stream, const Matrix &mat) {
  switch(mat.elemSize1()) {
    case 8: return out_matrix<double>(out_stream, mat, "%7.3f");
    case 4: return out_matrix<float> (out_stream, mat, "%7.3f");
    case 1: return out_matrix<uint>  (out_stream, mat, "%3i" );
    default:
      out_stream << "Cannot output matrix with elemSize1 of " << mat.elemSize1() << "\n";
      return out_stream;
  }
}

} // oscit
