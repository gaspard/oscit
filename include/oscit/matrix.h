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

#ifndef OSCIT_INCLUDE_OSCIT_MATRIX_H_
#define OSCIT_INCLUDE_OSCIT_MATRIX_H_

#include "opencv/cv.h"
#include "oscit/values.h"

/** \class Matrix is typedef of cv::Mat
 * cv::Mat is defined in cxcore.hpp:734
 */

namespace oscit {

/** The Matrix class is a cv::Mat with a default type of CV_64FC1 / CV_32FC1
 * depending on what Real represents.
 */
class Matrix : public cv::Mat {
public:
#if Real == double
  enum { DefaultType=CV_64FC1 };
#else
  enum { DefaultType=CV_32FC1 };
#endif
  Matrix() : cv::Mat() {}

  Matrix(int _rows, int _cols, int _type = DefaultType) : cv::Mat(_rows, _cols, _type) {}

  Matrix(cv::Size size, int _type = DefaultType) : cv::Mat(size, _type) {}

  Matrix(int _rows, int _cols, int _type, const cv::Scalar& _s) : cv::Mat(_rows, _cols, _type, _s) {}

  /** Construct a default _type matrix and fill it with value r.
   */
  Matrix(int _rows, int _cols, Real r) : cv::Mat(_rows, _cols, DefaultType, cv::Scalar(r)) {}

  Matrix(cv::Size size, int _type, const cv::Scalar& _s) : cv::Mat(size, _type, _s) {}

  // copy constructor
  Matrix(const Matrix& m) : cv::Mat(m) {}

  Matrix(int _rows, int _cols, int _type, void* _data, size_t _step=cv::Mat::AUTO_STEP) :
      cv::Mat(_rows, _cols, _type, _data, _step) {}
  Matrix(cv::Size _size, int _type, void* _data, size_t _step=cv::Mat::AUTO_STEP) : cv::Mat(_size, _type, _data, _step) {}
  // creates a matrix header for a part of the bigger matrix
  Matrix(const cv::Mat& m, const cv::Range& rowRange, const cv::Range& colRange) : cv::Mat(m, rowRange, colRange) {}

  /** Create a sub-matrix from a bigger matrix
   *
   * @param m the original matrix
   * @param roi the region of interest
   */
  Matrix(const cv::Mat& m, const cv::Rect& roi) : cv::Mat(m, roi) {}

  // builds matrix from std::vector with or without copying the data
  template<typename _Tp> explicit Matrix(const std::vector<_Tp>& vec, bool copyData=false) : cv::Mat(vec, copyData) {}
  // helper constructor to compile matrix expressions
  Matrix(const cv::MatExpr_Base& expr) : cv::Mat(expr) {}
};
}

#endif // OSCIT_INCLUDE_OSCIT_MATRIX_H_
