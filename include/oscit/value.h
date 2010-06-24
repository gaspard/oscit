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

#ifndef OSCIT_INCLUDE_OSCIT_VALUE_H_
#define OSCIT_INCLUDE_OSCIT_VALUE_H_

#include <string.h> // strlen
#include <string>
#include <sstream>  // ostringstream
#include <stdarg.h> // ... String and Error constructors
#include <assert.h>

#include "oscit/value_types.h"
#include "oscit/thash.h"
#include "oscit/string_data.h"
#include "oscit/error.h"
#include "oscit/list.h"
#include "oscit/hash.h"
#include "oscit/midi_message.h"

namespace oscit {

#define FVALUE_BUFFER_SIZE 256

class Matrix;

/** This is just a different typedef for std::string. */
class Json : public std::string
{
 public:
  explicit Json(const char *str) : std::string(str) {}
  explicit Json(const std::string &str) : std::string(str) {}
};


/** Value is the base type of all data transmitted between objects or used as parameters
and return values for osc messages.
 *
 *  Nil vs Empty: Empty means "not initialized", Nil means "initialized to 'no value'". If you pass
 *  Empty as a parameter it will be considered the same as Nil (root does a cast).
 */
class Value {
public:

  enum { AUTO_STEP=0 };

  /** =========================================================    Empty   */
  Value() : type_(EMPTY_VALUE) {}

  /** =========================================================    Real    */

  explicit Value(Real real) : type_(REAL_VALUE), r(real) {}

  explicit Value(int number) : type_(REAL_VALUE), r(number) {}

  /** =========================================================    String  */
  explicit Value(const char *string) : type_(STRING_VALUE) {
    set_string(string);
  }

  explicit Value(const std::string &string) : type_(STRING_VALUE) {
    set_string(string.c_str());
  }

  /** =========================================================    List     */
  explicit Value(const List *list) : type_(LIST_VALUE) {
    set_list(list);
  }

  explicit Value(const List &list) : type_(LIST_VALUE) {
    set_list(&list);
  }

  /** =========================================================    Error    */
  explicit Value(ErrorCode code, const char *str) : type_(ERROR_VALUE) {
    set_error(code, str);
  }

  explicit Value(ErrorCode code, const std::string& str) : type_(ERROR_VALUE) {
    set_error(code, str.c_str());
  }

  /** =========================================================    Hash    */
  explicit Value(const Hash *hash) : type_(HASH_VALUE) {
    set_hash(hash);
  }

  explicit Value(const Hash &hash) : type_(HASH_VALUE) {
    set_hash(&hash);
  }

  /** =========================================================    Matrix  */
  /** This is the easiest way to create a matrix from existing data.
   * Please read opencv docs for information
   */

  /** Create a new matrix value pointing to user-allocated data.
   * If this matrix is further assigned to another value, an error will be raised
   * since the data is not reference counted.
   *
   * @param rows number of rows in the data (height)
   * @param cols number of columns in the data (width)
   * @param type an integer representing a \ref MatrixType of the data that is stored
   * @param data a pointer to the user-allocated data
   * @param step number of <b>bytes</b> to advance from one row to the other. If the
   *              value is <tt>AUTO_STEP</tt>, the number of bytes will be calculated
   *              from the type and number of columns.
   */
  Value(size_t rows, size_t cols, int type, void *data, size_t step=AUTO_STEP);


  /** Create a new matrix value of the given size and type.
   *
   * @param rows number of rows (height)
   * @param cols number of columns (width)
   * @param type an integer representing the \ref MagicType
   */
  Value(size_t rows, size_t cols, int type);

  /** Create a new matrix of Real values of the given size.
   *
   * @param rows number of rows
   * @param cols number of columns
   */
  Value(size_t rows, size_t cols);

  /** Create a new Value by making a copy of the header of the provided matrix (shared data).
   */
  explicit Value(const Matrix *matrix) : type_(MATRIX_VALUE) {
    set_matrix(matrix);
  }

  /** Create a new Value by making a copy of the header of the provided matrix (shared data).
   */
  explicit Value(const Matrix &matrix) : type_(MATRIX_VALUE) {
    set_matrix(&matrix);
  }


  /** =========================================================    Midi    */
  explicit Value(const MidiMessage *midi_message) : type_(MIDI_VALUE) {
    set_midi(midi_message);
  }

  explicit Value(const MidiMessage &midi_message) : type_(MIDI_VALUE) {
    set_midi(&midi_message);
  }

  /** =========================================================    Any     */
  /** Create a value from a TypeTag string. */
  explicit Value(TypeTag type_tag) : type_(EMPTY_VALUE) {
    set_type_tag(type_tag.str_);
  }

  /** Create a value by parsing a JSON string. */
  explicit Value(const Json &json) : type_(EMPTY_VALUE) {
    build_from_json(json.c_str());
  }

  /** Create a default value from a type character like 'f'. */
  explicit Value(char type_char) : type_(EMPTY_VALUE) {
    set_type(type_from_char(type_char));
  }

  /** Copy constructor (needed since many methods return a Value). */
  Value(const Value &value) : type_(EMPTY_VALUE) {
    *this = value;
  }

  ~Value() {
    clear();
  }


  /** Share the content of another Value.
   * TODO: use 'const' to select between 'share' and 'copy' ?
   * FIXME: we are sharing a 'const' !
   * Solution: const Value = const xxx ==> share
   *           Value = const xxx       ==> copy first
   */
  Value &set(const Value &other) {
    // In case current value is a Hash or List, we must retain ReferenceCounted element before clear
    // or foo = foo[3] will fail.
    ReferenceCounted *old_ref = NULL;
    if (type_ == LIST_VALUE) {
      old_ref = ReferenceCounted::acquire(list_);
    } else if (type_ == HASH_VALUE) {
      old_ref = ReferenceCounted::acquire(hash_);
    }

    switch (other.type_) {
      case REAL_VALUE:
        set(other.r);
        break;
      case STRING_VALUE:
        share(other.string_);
        break;
      case LIST_VALUE:
        share(other.list_);
        break;
      case ERROR_VALUE:
        share(other.error_);
        break;
      case HASH_VALUE:
        share(other.hash_);
        break;
      case MATRIX_VALUE:
        // FIXME: share matrix header
        set(other.matrix_);
        break;
      case MIDI_VALUE:
        share(other.midi_message_);
        break;
      case ANY_VALUE:
        set_any();
        break;
      case TRUE_VALUE:
        set_true();
        break;
      case FALSE_VALUE:
        set_false();
        break;
      case EMPTY_VALUE:
        /* we consider that if you set a value with empty it means you want Nil.
         * This is very useful for return values. */
        /* continue */
      case NIL_VALUE:
        /* continue */
      default:
        set_nil();
    }

    if (old_ref) {
      ReferenceCounted::release(old_ref);
    }

    return *this;
  }

  /** Share the content of another Value. */
  Value &copy(const Value &other) {
    switch (other.type_) {
      case REAL_VALUE:
        set(other.r);
        break;
      case STRING_VALUE:
        set(*other.string_);
        break;
      case LIST_VALUE:
        set(other.list_);
        break;
      case ERROR_VALUE:
        set(other.error_);
        break;
      case HASH_VALUE:
        set(other.hash_);
        break;
      case MATRIX_VALUE:
        set(other.matrix_);
        break;
      case MIDI_VALUE:
        set(other.midi_message_);
        break;
      case ANY_VALUE:
        set_any();
        break;
      case TRUE_VALUE:
        set_true();
        break;
      case FALSE_VALUE:
        set_false();
        break;
      case EMPTY_VALUE:
        /* we consider that if you set a value with empty it means you want Nil.
         * This is very useful for return values.
         * THIS IS BAD. FIXME: remove.
         */
        /* continue */
      case NIL_VALUE:   /* continue */
      default:
        set_nil();
    }
    return *this;
  }

  /** Copy the content of the other value. */
  void operator=(const Value &other) {
    set(other);
  }

  /** Return true if the two objects have the same type
   * and contain identical values.
   */
  bool operator==(const Value &other) const {
    if (type_ != other.type_) return false;
    switch (type_) {
      case REAL_VALUE:   return r == other.r;
      case ERROR_VALUE:  return *error_ == *(other.error_);
      case STRING_VALUE: return *string_ == *(other.string_);
      case HASH_VALUE:   return *hash_ == *(other.hash_);
      case MATRIX_VALUE: /* not supported */
        std::cerr << "Matrix values cannot be compared.\n";
        assert(false);
      case MIDI_VALUE:   return *midi_message_ == *(other.midi_message_);
      case LIST_VALUE:   return *list_ == *(other.list_);
      case TRUE_VALUE:   /* continue */
      case FALSE_VALUE:  /* continue */
      case NIL_VALUE:    /* continue */
      case ANY_VALUE:    /* continue */
      case EMPTY_VALUE:  /* continue */
      default:           return true; // ==> type_ == other.type_ (see above)
    }
  }

  bool operator!=(const Value &other) const {
    return !(*this == other);
  }

  const char *type_tag() const {
    switch (type_) {
      case NIL_VALUE:    return "N";
      case TRUE_VALUE:   return "T";
      case FALSE_VALUE:  return "F";
      case REAL_VALUE:   return "f";
      case ERROR_VALUE:  return "E";
      case STRING_VALUE: return "s";
      case HASH_VALUE:   return "H";
      case MATRIX_VALUE: return "M";
      case MIDI_VALUE:   return "m";
      case LIST_VALUE:   return list_->type_tag();
      case ANY_VALUE:    return "*";
      case EMPTY_VALUE:  /* continue */
      default:           return "";
    }
  }

  TypeTagID type_id() const {
    switch (type_) {
      case NIL_VALUE:    return H("N");
      case TRUE_VALUE:   return H("T");
      case FALSE_VALUE:  return H("F");
      case REAL_VALUE:   return H("f");
      case ERROR_VALUE:  return H("E");
      case STRING_VALUE: return H("s");
      case HASH_VALUE:   return H("H");
      case MATRIX_VALUE: return H("M");
      case MIDI_VALUE:   return H("m");
      case LIST_VALUE:   return list_->type_id();
      case ANY_VALUE:    return H("*");
      case EMPTY_VALUE:  /* continue */
      default:           return NO_TYPE_TAG_ID;  // EMPTY_VALUE
    }
  }

  ValueType type() const {
    return type_;
  }

  /** Build the content of the value from the characters in the buffer until NULL.
   *  @return number of characters eaten in the buffer to build Value.
   */
  size_t build_from_json(const char *json, bool strict_mode = false);

  /** Return a string representation of the value. */
  Json to_json() const;

  /** Return a lazy json string representation of the value. */
  Json lazy_json() const;

  /** Change the Value into the specific type. Since a default value must be set,
    * it is better to use 'set'. */
  void set_type(ValueType type) {
    set_type_without_default(type);
    set_default();
  }

  /** Change the Value into something defined in a typeTag. */
  const char *set_type_tag(const char *type_tag) {
    if (strlen(type_tag) > 1) {
      set_type_without_default(LIST_VALUE);
      list_ = new List;
      return list_->set_type_tag(type_tag);
    } else {
      set_type(type_from_char(type_tag[0]));
      return type_tag+1; // eof
    }
  }


  static ValueType type_from_char(char c) {
    switch (c) {
      case REAL_TYPE_TAG:   return REAL_VALUE;
      case STRING_TYPE_TAG: return STRING_VALUE;
      // ERROR_TYPE_TAG == STRING_TYPE_TAG;
      case HASH_TYPE_TAG:   return HASH_VALUE;
      case MATRIX_TYPE_TAG: return MATRIX_VALUE;
      case MIDI_MESSAGE_TYPE_TAG: return MIDI_VALUE;
      case ANY_TYPE_TAG:    return ANY_VALUE;
      case NIL_TYPE_TAG:    return NIL_VALUE;
      case TRUE_TYPE_TAG:   return TRUE_VALUE;
      case FALSE_TYPE_TAG:  return FALSE_VALUE;
      default:              return EMPTY_VALUE;
    }
  }


  /** =========================================================    Empty   */
  /** TODO: rename to 'void' ? because empty also means 'empty array'... */
  bool is_empty() const { return type_ == EMPTY_VALUE; }

  /** Change the value to nil. */
  Value &set_empty() {
    set_type_without_default(EMPTY_VALUE);
    return *this;
  }

  /** =========================================================    Nil     */
  bool is_nil() const    { return type_ == NIL_VALUE; }

  /** Change the value to nil. */
  Value &set_nil() {
    set_type_without_default(NIL_VALUE);
    return *this;
  }

  /** =========================================================    True     */
  bool is_true() const    { return type_ == TRUE_VALUE; }
  bool is_bang() const    { return type_ == TRUE_VALUE; }

  /** Change the value to nil. */
  Value &set_true() {
    set_type_without_default(TRUE_VALUE);
    return *this;
  }


  /** =========================================================    False     */
  bool is_false() const    { return type_ == FALSE_VALUE; }

  /** Change the value to nil. */
  Value &set_false() {
    set_type_without_default(FALSE_VALUE);
    return *this;
  }

  /** =========================================================    Real    */
  bool is_real() const  { return type_ == REAL_VALUE; }

  /** Change the Value into a RealValue. */
  Value &set(Real real) {
    set_type_without_default(REAL_VALUE);
    r = real;
    return *this;
  }

  /** Safe accessor for Real value.
   * returns the default value if the current Value is not a real number.
   */
  Real get_real(Real default_result = 0) const {
    return is_real() ? r : default_result;
  }

  /** =========================================================    String  */
  bool is_string() const { return type_ == STRING_VALUE; }

  /** Change the Value into a StringValue. */
  Value &set(const char *string) {
    set_type_without_default(STRING_VALUE);
    set_string(string);
    return *this;
  }

  /** Change the Value into a StringValue. */
  Value &set(const std::string &string) {
    set_type_without_default(STRING_VALUE);
    set_string(string);
    return *this;
  }

  Value& append(const std::string &string) {
    return append(string.c_str());
  }

  Value& append(const char *str) {
    if (is_error()) {
      error_->append(str);
    } else if (is_string()) {
      string_->append(str);
    }
    return *this;
  }

  inline const std::string &str() const {
    return *string_;
  }

  inline std::string &str() {
    return *string_;
  }

  inline const char *c_str() const {
    return string_->c_str();
  }

  /** Split the string with the given character.
   * @param c the character to use for string splitting
   * @return a ListValue with one or more strings
   */
  Value split(char c) const;

  /** Split the string with the given character.
   * @param str the string to use for splitting
   * @return a ListValue with one or more strings
   */
  Value split(const char *str) const;

  /** =========================================================    List    */
  bool is_list() const   { return type_ == LIST_VALUE; }

  /** Change the Value into a List by copying the content of the argument. */
  Value &set(const List *list) {
    set_type_without_default(LIST_VALUE);
    set_list(list);
    return *this;
  }

  /** Change the Value into a List by copying the content of the argument. */
  Value &set(const List &list) {
    set_type_without_default(LIST_VALUE);
    set_list(&list);
    return *this;
  }

  const Value &operator[](size_t pos) const {
    return *((*list_)[pos]);
  }

  Value &operator[](size_t pos) {
    return *((*list_)[pos]);
  }

  const Value &value_at(size_t pos) const {
    return *((*list_)[pos]);
  }

  Value &value_at(size_t pos) {
    return *((*list_)[pos]);
  }

  const Value &last() const {
    if (is_list()) {
      Value *last = list_->last();
      return last ? *last : gNilValue;
    } else {
      return *this;
    }
  }

  const Value &first() const {
    if (is_list()) {
      Value *first = list_->first();
      return first ? *first : gNilValue;
    } else {
      return *this;
    }
  }

  void set_value_at(size_t pos, const Value &val) {
    if (!is_list()) return;
    list_->set_value_at(pos, val);
  }

  size_t size() const {
    return type_ == LIST_VALUE ? list_->size() : 0;
  }

  template<class T>
  Value &push_back(const T& elem) {
    return push_back(Value(elem));
  }

  Value &push_back(const Value& val);

  template<class T>
  Value &push_front(const T& elem) {
    return push_front(Value(elem));
  }

  Value &push_front(const Value& val);

  /** Join the strings in the list with the given character.
   * @param str the character to join elements with
   * @return a StringValue with the result of the join operation
   */
  Value join(char c) const;

  /** Join the strings in the list with the given string.
   * @param str the string to join elements with
   * @return a StringValue with the result of the join operation
   */
  Value join(const char *str) const;

  /** =========================================================    Error   */
  bool is_error() const  { return type_ == ERROR_VALUE; }

  /** Change the Value into an ErrorValue. */
  Value &set(ErrorCode code, const char *string) {
    set_type_without_default(ERROR_VALUE);
    set_error(code, string);
    return *this;
  }

  /** Change the Value into an ErrorValue. */
  Value &set(ErrorCode code, const std::string &string) {
    set_type_without_default(ERROR_VALUE);
    set_error(code, string.c_str());
    return *this;
  }

  /** Change the Value into a ListValue by copying the content of the argument. */
  Value &set(const Error *error) {
    set_type_without_default(ERROR_VALUE);
    set_error(error);
    return *this;
  }

  /** Change the Value into a ListValue by copying the content of the argument. */
  Value &set(const Error &error) {
    set_type_without_default(ERROR_VALUE);
    set_error(&error);
    return *this;
  }

  const std::string& error_message() const {
    return error_->message();
  }

  ErrorCode error_code() const {
    return error_->code();
  }

  /** =========================================================    Hash    */
  bool is_hash() const   { return type_ == HASH_VALUE; }

  /** Change the Value into a HashValue by copying the content of the argument. */
  Value &set(const Hash *hash) {
    set_type_without_default(HASH_VALUE);
    set_hash(hash);
    return *this;
  }

  /** Change the Value into a HashValue by copying the content of the argument. */
  Value &set(const Hash &hash) {
    set_type_without_default(HASH_VALUE);
    set_hash(&hash);
    return *this;
  }

  template<class T>
  void set(const char *key, const T &val) {
    set(std::string(key), Value(val));
  }

  template<class T>
  void set(const std::string &key, const T &val) {
    set(key, Value(val));
  }

  void set(const std::string &key, const Value &val) {
    if (!is_hash()) set_type(HASH_VALUE);
    hash_->set(key, val);
  }

  bool get(const std::string &key, Value *retval) const {
    if (!is_hash()) return false;
    // TODO: how to make sure the retval is not changed ?
    return hash_->get(key, retval);
  }

  HashIterator begin() const {
    return is_hash() ? hash_->begin() : gEmptyHash.begin();
  }

  HashIterator end() const {
    return is_hash() ? hash_->end() : gEmptyHash.end();
  }

  const Value operator[](const std::string &key) const {
    if (!is_hash()) return gNilValue;
    Value res;
    if (get(key, &res)) {
      return res;
    } else {
      return gNilValue;
    }
  }

  Value operator[](const std::string &key) {
    if (!is_hash()) return gNilValue;
    Value res;
    if (get(key, &res)) {
      return res;
    } else {
      return gNilValue;
    }
  }

  const Value operator[](const char *&key) const {
    // we need *&key to disable ambiguous overloading with operator[](size_t)
    if (!is_hash()) return gNilValue;
    Value res;
    if (get(key, &res)) {
      return res;
    } else {
      return gNilValue;
    }
  }

  Value operator[](const char *&key) {
    // we need *&key to disable ambiguous overloading with operator[](size_t)
    if (!is_hash()) return gNilValue;
    Value res;
    if (get(key, &res)) {
      return res;
    } else {
      return gNilValue;
    }
  }

  /** Merge 'other' hash into the current hash.
   * For example if the current content is {"one":{"x":45, "y":100}, "two":{...}} and we
   * receive {"one":{"x":40}, "two":null}, the final result will be
   * {"one":{"x":40, "y":100}}.
   */
  void deep_merge(const Value &other);

  /** Return true if the current value is a hash and it contains an entry
   * for the given key.
   */
  bool has_key(const char *key) const {
    if (!is_hash()) return false;
    return hash_->has_key(std::string(key));
  }

  /** Return true if the current value is a hash and it contains an entry
   * for the given key.
   */
  bool has_key(const std::string &key) const {
    if (!is_hash()) return false;
    return hash_->has_key(key);
  }

  /** Remove entry with the given key.
   */
  void remove(const char *key) {
    if (!is_hash()) return;
    hash_->remove(std::string(key));
  }

  /** Remove entry with the given key.
   */
  void remove(std::string &key) {
    if (!is_hash()) return;
    hash_->remove(key);
  }

  /** =========================================================    Matrix  */
  bool is_matrix() const   { return type_ == MATRIX_VALUE; }

  /** Return a reference to the current matrix (no type check).
   * @return reference to current matrix
   */
  Matrix &matrix() {
    return *matrix_;
  }

  /** Change the Value into a MatrixValue by making a reference to the argument. */
  Value &set(const Matrix *matrix) {
    set_type_without_default(MATRIX_VALUE);
    set_matrix(matrix);
    return *this;
  }

  /** Adopt a given matrix (will be freed on Value destruction).
   */
  Matrix *adopt(Matrix *matrix) {
    set_type_without_default(MATRIX_VALUE);
    matrix_ = matrix;
    return matrix_;
  }

  /** Change the Value into a MatrixValue by making a reference to the argument. */
  Value &set(const Matrix &matrix) {
    set_type_without_default(MATRIX_VALUE);
    set_matrix(&matrix);
    return *this;
  }

  /** Return the matrix size (number of rows * number of columns).
   */
  size_t mat_size() const;

  /** Return the OpenCV type (CV_32FC1 for example).
   */
  int mat_type() const;

  /** Return a pointer to the first element in the matrix.
   */
  void *mat_data() const;

  /** =========================================================    Midi  */
  bool is_midi() const   { return type_ == MIDI_VALUE; }

  /** Change the Value into a MidiValue by making a reference
   *  to the argument. */
  Value &set(const MidiMessage *midi_message) {
    set_type_without_default(MIDI_VALUE);
    set_midi(midi_message);
    return *this;
  }

  /** Change the Value into a MidiMessage by making a reference
   *  to the argument. */
  Value &set(const MidiMessage &midi_message) {
    set_type_without_default(MIDI_VALUE);
    set_midi(&midi_message);
    return *this;
  }

  /** Change the Value into a Midi note.
   */
  void set_as_note(
      unsigned char note,
      unsigned char velocity = 80,
      unsigned int length = 500,
      unsigned int channel = 1,
      time_t wait = 0) {
    if (!is_midi()) set_type(MIDI_VALUE);
    midi_message_->set_as_note(note, velocity, length, channel, wait);
  }

  /** Change the Value into a Midi control change.
   */
  void set_as_ctrl(unsigned char ctrl, unsigned char ctrl_value,
    unsigned int channel = 1, time_t wait = 0) {
    if (!is_midi()) set_type(MIDI_VALUE);
    midi_message_->set_as_ctrl(ctrl, ctrl_value, channel, wait);
  }

  /** =========================================================    Any     */
  bool is_any() const    { return type_ == ANY_VALUE; }

  /** Change the value to nil. */
  Value &set_any() {
    set_type_without_default(ANY_VALUE);
    return *this;
  }

  /** Create a value by parsing a JSON string. */
  Value &set(const Json &json) {
    set_type_without_default(NIL_VALUE);
    build_from_json(json.c_str());
    return *this;
  }

  /** Print value into the given stream.
   *  @param out_stream std stream to print value in
   *  @param lazy print mode (lazy = unparsable json but can be easier to read)
   */
  void to_stream(std::ostream &out_stream, bool lazy = false) const;

  template<class T>
  Value &operator<<(const T &val) {
    if (type_ == STRING_VALUE) {
      std::ostringstream oss;
      oss << val;
      string_->append(oss.str());
    } else if (type_ == ERROR_VALUE) {
      std::ostringstream oss;
      oss << val;
      error_->message_.append(oss.str());
    } else {
      // ignore (maybe we could cast to string)
    }
    return *this;
  }

  Value &operator<<(const char *str) {
    if (type_ == STRING_VALUE) {
      string_->append(str);
    } else if (type_ == ERROR_VALUE) {
      error_->message_.append(str);
    } else {
      // ignore (maybe we could cast to string)
    }
    return *this;
  }

  Value &operator<<(const std::string &str) {
    if (type_ == STRING_VALUE) {
      string_->append(str);
    } else if (type_ == ERROR_VALUE) {
      error_->message_.append(str);
    } else {
      // ignore (maybe we could cast to string)
    }
    return *this;
  }

 protected:
  /** Set the value to nil and release/free contained data. */
  void clear()  {
   switch (type_) {
   case LIST_VALUE:
     list_ = ReferenceCounted::release(list_);
     break;
   case STRING_VALUE:
     string_ = ReferenceCounted::release(string_);
     break;
   case ERROR_VALUE:
     error_ = ReferenceCounted::release(error_);
     break;
   case HASH_VALUE:
     hash_ = ReferenceCounted::release(hash_);
     break;
   case MATRIX_VALUE:
     if (matrix_ != NULL) delete_matrix();
     matrix_ = NULL;
     break;
   case MIDI_VALUE:
     midi_message_ = ReferenceCounted::release(midi_message_);
     break;
   default:
     ; // nothing to clear
   }
  }

  /** Change the Value into the specific type. Does not set any default value
    * so the object must be considered uninitialized. */
  void set_type_without_default(ValueType type) {
    clear();
    type_ = type;
  }

  /** Properly initialize the type with a default value. */
  inline void set_default() {
    switch (type_) {
      case REAL_VALUE:
        r = 0.0;
        break;
      case STRING_VALUE:
        set_string("");
        break;
      case LIST_VALUE:
        list_ = new List;
        break;
      case ERROR_VALUE:
        error_ = new Error;
        break;
      case HASH_VALUE:
        hash_ = new Hash(DEFAULT_HASH_TABLE_SIZE);
        break;
      case MATRIX_VALUE:
        matrix_ = build_matrix();
        break;
      case MIDI_VALUE:
        midi_message_ = new MidiMessage;
        break;
      default:
        ; // nothing to set
    }
  }


  /** =========================================================    String  */
  void share(const StringData *string) {
    if (type_ == STRING_VALUE && string_ == string) return;
    set_type_without_default(STRING_VALUE);
    // FIXME: there should be a way to deal with shared content
    // that is protected from changes... Any solution welcome !!
    string_ = ReferenceCounted::acquire(const_cast<StringData*>(string));
  }

  void set_string(const char *string) {
    string_ = new StringData(string);
  }

  void set_string(const std::string &string) {
    string_ = new StringData(string);
  }

  /** =========================================================    Error   */
  void share(const Error *error) {
    if (type_ == ERROR_VALUE && error_ == error) return;
    set_type_without_default(ERROR_VALUE);
    // FIXME: there should be a way to deal with shared content
    // that is protected from changes... Any solution welcome !!
    error_ = ReferenceCounted::acquire(const_cast<Error*>(error));
  }

  /** Set error content. */
  void set_error(ErrorCode code, const char *string) {
    error_ = new Error(code, string);
  }

  /** Set error content. */
  void set_error(const Error *error) {
    error_ = new Error(*error);
  }

  /** =========================================================    List    */
  void share(const List *list) {
    if (type_ == LIST_VALUE && list_ == list) return;
    set_type_without_default(LIST_VALUE);
    // FIXME: there should be a way to deal with shared content
    // that is protected from changes... Any solution welcome !!
    list_ = ReferenceCounted::acquire(const_cast<List*>(list));
  }

  /** Set List content by making a copy. */
  void set_list(const List *list) {
    list_ = new List(*list);
  }

  /** =========================================================    Hash    */
  void share(const Hash *hash) {
    if (type_ == HASH_VALUE && hash_ == hash) return;
    set_type_without_default(HASH_VALUE);
    // FIXME: there should be a way to deal with shared content
    // that is protected from changes... Any solution welcome !!
    hash_ = ReferenceCounted::acquire(const_cast<Hash*>(hash));
  }

  /** Set hash content. */
  void set_hash(const Hash *hash) {
    hash_ = new Hash(*hash);
  }

  /** =========================================================    Matrix  */
  /** Set matrix content. Matrix data is automatically shared
   * through opencv's reference counting mechanism.
   */
  void set_matrix(const Matrix *matrix);

  /** @internal.
   * Create an empty matrix.
   */
  Matrix *build_matrix();

  /** @internal.
   * Delete a matrix.
   */
  void delete_matrix();

  /** =========================================================    Midi    */
  void share(const MidiMessage *midi_message) {
    if (type_ == MIDI_VALUE && midi_message_ == midi_message) return;
    set_type_without_default(MIDI_VALUE);
    // FIXME: there should be a way to deal with shared content
    // that is protected from changes... Any solution welcome !!
    midi_message_ = ReferenceCounted::acquire(const_cast<MidiMessage*>(midi_message));
  }

  /** Set hash content. */
  void set_midi(const MidiMessage *midi_message) {
    midi_message_ = new MidiMessage(*midi_message);
  }

  ValueType type_;

 public:
  union {
    /** Store a single Real number.
     *  The value of 'Real' can be a float or double depending on compilation.
     */
    Real r;

    /** Pointer to a reference counted std::string.
     */
    StringData *string_;

    /** Pointer to a list of values (class List).
     *  The list is reference counted.
     */
    List *list_;

    /** Pointer to an Error (contains an error code and message).
     *  The error is reference counted.
     */
    Error *error_;

    /** Pointer to a reference counted dictionary.
     *  Use std::string as keys for the dictionary.
     */
    Hash *hash_;

    /** Pointer to an opencv matrix (cv::Mat).
     *  TODO: reference count matrix_ !
     */
    Matrix *matrix_;

    /** Pointer to a reference counted MidiMessage.
     */
    MidiMessage *midi_message_;
  };
};

class JsonValue : public Value
{
public:
  JsonValue(const char *json) : Value(Json(json)) {}
};

std::ostream &operator<< (std::ostream &out_stream, const Value &val);
std::ostream &operator<< (std::ostream &out_stream, const Matrix &mat);

/** This is a sub-class of Value so that we can use printf style to create
 * String and Error values.
 */
class FValue : public Value {
public:
  /** =========================================================    String  */
  explicit FValue(const char *format, ...) {
    char buffer[FVALUE_BUFFER_SIZE];
    type_ = STRING_VALUE;
    va_list args;
    va_start(args, format);
      vsnprintf(buffer, FVALUE_BUFFER_SIZE, format, args);
    va_end(args);
    set_string(buffer);
  }

  /** =========================================================    Error    */
  explicit FValue(ErrorCode code, const char *format, ...) {
    char buffer[FVALUE_BUFFER_SIZE];
    type_ = ERROR_VALUE;
    va_list args;
    va_start(args, format);
      vsnprintf(buffer, FVALUE_BUFFER_SIZE, format, args);
    va_end(args);
    set_error(code, buffer);
  }
};

} // oscit

#endif // OSCIT_INCLUDE_OSCIT_VALUE_H_
