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

#include "test_helper.h"
#include "oscit/values.h"
#include "opencv/cxtypes.h"

class MatrixValueTest : public TestHelper
{
public:
  void test_is_matrix( void ) {
    Value v(TypeTag("M"));

    assert_false(v.is_empty());
    assert_false(v.is_nil());
    assert_false(v.is_true());
    assert_false(v.is_false());
    assert_false(v.is_real());
    assert_false(v.is_string());
    assert_false(v.is_list());
    assert_false(v.is_error());
    assert_false(v.is_hash());
    assert_true (v.is_matrix());
    assert_false(v.is_any());

    Matrix * m = v.matrix_;
    assert_equal(0, m->rows);
    assert_equal(0, m->cols);
    assert_true(m->type() == 0);

    assert_equal("M", v.type_tag());
    assert_equal(MATRIX_TYPE_TAG_ID, v.type_id());

    assert_equal(MATRIX_TYPE_TAG_ID, hashId("M"));

  }

  void test_create_matrix_value( void ) {
    MatrixValue v(3,2);
    MatrixValue v2;

    assert_true(v.is_matrix());
    assert_true(v2.is_matrix());


    assert_equal(6, v.mat_size());
    assert_equal(0, v2.mat_size());
  }

  void test_create_with_char( void ) {
    Value v('M');

    assert_true(v.is_matrix());
    assert_equal(0, v.mat_size());
  }

  void test_create_with_TypeTag( void ) {
    Value v(TypeTag("M"));

    assert_true(v.is_matrix());
    assert_equal(0, v.mat_size());
  }

  void test_copy( void ) {
    MatrixValue v(2,3);
#if Real == double
    assert_true(v.mat_type() == CV_64FC1);
#else
    assert_true(v.mat_type() == CV_32FC1);
#endif
    Real *v_data = (Real*)v.mat_data();

    v_data[0] = 0.0; v_data[1] = 1.0; v_data[2] = 2.0;
    v_data[3] = 3.0; v_data[4] = 4.0; v_data[5] = 5.0;

    Value v2(v);
    Real *v2_data = (Real*)v2.mat_data();
    Value v3;

    assert_true(v2.is_matrix());
    assert_true(v3.is_empty());

    assert_equal(3.0, v2_data[3]);
    assert_equal(6, v.mat_size());
    assert_equal(6, v2.mat_size());

    v_data[3] = 3.5;

    // shared data
    assert_equal(3.5, v2_data[3]);

    v3 = v;
    Real *v3_data = (Real*)v3.mat_data();

    assert_true(v3.is_matrix());
    assert_equal(6, v3.mat_size());
    assert_equal(3.5, v3_data[3]);

    v_data[3] = 3.8;

    assert_equal(3.8,  v_data[3]);
    assert_equal(3.8, v2_data[3]);
    assert_equal(3.8, v3_data[3]);
  }

  void test_set( void ) {
    Value v;
    Matrix m(2, 2);

    assert_true(v.is_empty());
    v.set(m);
    assert_true(v.is_matrix());
    assert_equal(4, v.mat_size());
  }

  void test_set_tag( void ) {
    Value v;

    v.set_type_tag("M");
    assert_true(v.is_matrix());
    assert_equal(0, v.mat_size());
  }

  void test_set_type( void ) {
    Value v;

    v.set_type(MATRIX_VALUE);
    assert_true(v.is_matrix());
    assert_equal(0, v.mat_size());
  }

  void test_to_json( void ) {
    MatrixValue v(2,3);
    std::ostringstream os(std::ostringstream::out);
    os << v;
    assert_equal("\"Matrix 2x3\"", os.str());
    assert_equal("\"Matrix 2x3\"", v.to_json());
  }

  void test_can_receive( void ) {
    Object object("foo", Oscit::matrix_io("bar"));
    assert_false(object.can_receive(Value()));
    assert_true (object.can_receive(gNilValue));
    assert_true (object.can_receive(gBangValue));
    assert_false(object.can_receive(Value(1.23)));
    assert_false(object.can_receive(Value("foo")));
    assert_false(object.can_receive(Value(BAD_REQUEST_ERROR, "foo")));
    assert_false(object.can_receive(JsonValue("['','']")));
    assert_false(object.can_receive(HashValue()));
    assert_true (object.can_receive(MatrixValue(3, 2)));
    assert_false(object.can_receive(MidiValue()));
  }

  // test_equal not supported for matrix types

  void test_user_allocated_data( void ) {
    double raw_data[28] = { 9, 9, 9, 9, 9, 9, 9, // 9 = padding
                           9, 1, 0, 0, 0, 9, 9,  // 4x3
                           9, 0, 2, 3, 0, 9, 9,  //
                           9, 0, 0, 0, 4, 9, 9}; //
    Value mat(4, 7, CV_64FC1, raw_data);

    // remove padding by setting Region of interest (ROI):
    mat.matrix_->adjustROI(-1, 0, -1, -2); // top, bottom, left, right

    assert_equal(3, mat.matrix_->rows);
    assert_equal(4, mat.matrix_->cols);
    double *data = (double*)mat.matrix_->data;
    assert_equal(1.0, data[0]);

    assert_equal(3.0, (data + mat.matrix_->step1())[2]);
  }

  void test_copy_user_allocated_data( void ) {
    double raw_data[28] = { 1, 0, 0, 0,  // 3x4
                            0, 2, 3, 0,  //
                            0, 0, 0, 4}; //
    Value mat(3, 4, CV_64FC1, raw_data);
    double *mat_data = (double*)mat.matrix_->data;

    Value other;

    other = mat;
    assert_equal(0, other.matrix_->rows);
    assert_equal(0, other.matrix_->cols);

    other.adopt(new Matrix(3, 4, CV_64FC1, raw_data));

    assert_equal(3, other.matrix_->rows);
    assert_equal(4, other.matrix_->cols);
    double *other_data = (double*)other.matrix_->data;
    assert_equal(1.0, other_data[0]);

    assert_equal(3.0, (other_data + other.matrix_->step1())[2]);

    (other_data + other.matrix_->step1())[2] = 4.5;
    // data was shared
    assert_equal(4.5, (mat_data + mat.matrix_->step1())[2]);
  }

  void test_copy_empty_matrix( void ) {
    Value a;
    MatrixValue b;
    a = b;
    assert_equal(NULL, a.matrix_->data);
  }

  void test_out_stream_single_channel( void ) {
    double raw_data[12] = { 1, 2, 3, 4,
                            5, 6, 7, 8,
                            9,10,11,12};
    Value mat(3, 4, CV_64FC1, raw_data);
    std::ostringstream oss;
    oss << *mat.matrix_;
    assert_equal("[  1.000  2.000  3.000  4.000\n   5.000  6.000  7.000  8.000\n   9.000 10.000 11.000 12.000 ]\n", oss.str());
  }

  void test_out_stream_two_channels( void ) {
    uint raw_data[8] = { 1, 2, 3, 4,
                         5, 6, 7, 8};
    Value mat(2, 2, CV_8UC2, raw_data);
    std::ostringstream oss;
    oss << *mat.matrix_;
    assert_equal("[{  1   2}{  3   4}\n {  5   6}{  7   8} ]\n", oss.str());
  }

  void test_float_channels( void ) {
    Value mat(2, 2, CV_32FC3);
    // 12 bytes = size of 3 floats (1 per channel)
    assert_equal(12, mat.matrix_->elemSize());
    // 4 bytes = size of 1 float
    assert_equal(4, mat.matrix_->elemSize1());
    // 24 bytes = size of one row = 2 * 3 floats = 6 * 4 bytes
    assert_equal(24, mat.matrix_->step);
    // 6 elements = number elements to move to next row = 2 * 3
    assert_equal(6, mat.matrix_->step1());
    // 3 values per element
    assert_equal(3, mat.matrix_->channels());
  }

  void test_uint_channels( void ) {
    Value mat(2, 2, CV_8UC3);
    // 3 bytes = size of 3 unsigned ints (1 per channel)
    assert_equal(3, mat.matrix_->elemSize());
    // 1 bytes = size of 1 unsigned int
    assert_equal(1, mat.matrix_->elemSize1());
    // 6 bytes = size of one row = 2 * 3 unsigned ints = 6 * 1 bytes
    assert_equal(6, mat.matrix_->step);
    // 6 elements = number elements to move to next row = 2 * 3
    assert_equal(6, mat.matrix_->step1());
    // 3 values per element
    assert_equal(3, mat.matrix_->channels());
  }

};