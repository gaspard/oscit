
#line 1 "/Users/gaspard/git/oscit/src/value.rl"
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

#include "oscit/values.h"

//#define DEBUG_PARSER

#include <string.h>  // strlen
#include <stdlib.h>  // atof

#include <iostream>
#include <sstream>

#include "oscit/matrix.h"

/** Ragel parser definition to create Values from JSON. */
namespace oscit {

#ifdef DEBUG_PARSER
#define DEBUG(x) x
#else
#define DEBUG(x)
#endif

Value gNilValue('N');
Value gTrueValue('T');
Value gFalseValue('F');
Value gEmptyValue;
Hash  gEmptyHash(1);


// ------------------------------------------------------------- escape
static std::string escape(const std::string &string) {
  std::string res;
  size_t len = 0;
  const char *last_append = string.c_str();
  const char *ptr = string.c_str();
  while (*ptr) {
    if (*ptr == '"') {
      // append \"
      if (len) {
        res.append(last_append, len);
        last_append = ptr + 1;
        len = 0;
      }
      res.append("\\\"");
      ++ptr;
    } else if (*ptr == '\\') {
      ++len;
      ++ptr;
      if (*ptr) {
        // in case string ends with "\"
        ++ptr;
        ++len;
      }
    } else {
      // append
      ++len;
      ++ptr;
    }
  }

  if (len) res.append(last_append, len);
  return res;
}

// ------------------------------------------------------------- operator<<
std::ostream &operator<<(std::ostream &out_stream, const Value &val) {
  val.to_stream(out_stream);
  return out_stream;
}

// ------------------------------------------------------------- to_stream
void Value::to_stream(std::ostream &out_stream, bool lazy) const {
  size_t sz;
  switch (type()) {
    case REAL_VALUE:
      out_stream << r;
      break;
    case ERROR_VALUE:
      out_stream << "\"" << error_code() << " " << escape(error_message()) << "\"";
      break;
    case STRING_VALUE:
      if (lazy) {
        out_stream << str();
      } else {
        out_stream << "\"" << escape(str()) << "\"";
      }
      break;
    case HASH_VALUE:
      if (lazy) {
        hash_->to_stream(out_stream, true);
      } else {
        out_stream << "{" << *hash_ << "}";
      }
      break;
    case MATRIX_VALUE:
      // FIXME: replace by {"=M":[[xxx], [xxx]]}
      out_stream << "\"Matrix " << matrix_->rows << "x" << matrix_->cols << "\"";
      break;
    case MIDI_VALUE:
      // FIXME: replace by {"=m":[byte, byte, byte, ...]}
      out_stream << "\"MidiMessage " << *midi_message_ << "\"";
      break;
    case LIST_VALUE:
      sz = size();
      if (!lazy) out_stream << "[";
      for (size_t i = 0; i < sz; ++i) {
        if (i > 0) out_stream << ", ";
        out_stream << this->operator[](i);
      }
      if (!lazy) out_stream << "]";
      break;
    case TRUE_VALUE:
      out_stream << "true";
      break;
    case FALSE_VALUE:
      out_stream << "false";
      break;
    case EMPTY_VALUE: /* continue */
    case ANY_VALUE:   /* continue */
    case NIL_VALUE:   /* continue */
    default:
      out_stream << "null";
  }
}

// ------------------------------------------------------------- to_json
Json Value::to_json() const {
  std::ostringstream os(std::ostringstream::out);
  os << *this;
  return (Json)os.str();
}

// ------------------------------------------------------------- lazy_json
Json Value::lazy_json() const {
  std::ostringstream os(std::ostringstream::out);
  to_stream(os, true);
  return (Json)os.str();
}

// ------------------------------------------------------------- push_back
Value &Value::push_back(const Value& val) {
  if (!val.is_empty()) {
    if (is_list()) {
      list_->push_back(val);
    } else if (is_empty() && val.is_list()) {
      set_type(LIST_VALUE);
      list_->push_back(val);
    } else if (is_empty()) {
      set(val);
    } else {
      // copy self as first element
      Value original(*this);
      set_type(LIST_VALUE);
      list_->push_back(original);
      list_->push_back(val);
    }
  }
  return *this;
}

// ------------------------------------------------------------- push_front
Value &Value::push_front(const Value& val) {
  if (!val.is_empty()) {
    if (is_list()) {
      list_->push_front(val);
    } else if (is_empty()) {
      set(val);
    } else {
      // copy self as first element
      Value original(*this);
      set_type(LIST_VALUE);
      list_->push_back(val); // push_back is faster
      list_->push_back(original);
    }
  }
  return *this;
}

// ------------------------------------------------------------- s_deep_merge
static void s_deep_merge(const Value a, const Value b) {
  HashIterator it, end = b.end();
  std::string key;
  Hash *res_hash = a.hash_;
  Value val_a, val_b;

  for (it = b.begin(); it != end; ++it) {
    key = *it;
    if (!b.hash_->get(key, &val_b)) continue; // this should never happen, just in case ;-)

    if (a.hash_->get(key, &val_a)) {
      // existing key
      if (val_a.is_hash() && val_b.is_hash()) {
        // deep merge
        s_deep_merge(val_a, val_b);
      } else if (val_b.is_nil()) {
        // remove
        res_hash->remove(key);
      } else {
        // update
        res_hash->set(key, val_b);
      }
    } else {
      // new key
      res_hash->set(key, val_b);
    }
  }
}

// ------------------------------------------------------------- deep_merge
void Value::deep_merge(const Value &other) {
  if (!is_hash() || !other.is_hash()) return;
  s_deep_merge(*this, other);
}

// ------------------------------------------------------------- split
template<typename T>
Value s_split(T &token, size_t token_len, const std::string &string) {
  ListValue res;

  std::string element;

  size_t start_pos = 0;
  size_t end_pos;

  do {
    end_pos = string.find(token, start_pos);
    res.push_back(string.substr(start_pos, end_pos - start_pos));
    start_pos = end_pos + token_len;
  } while (end_pos != std::string::npos);

  return res;
}

Value Value::split(char c) const {
  if (!is_string()) return ListValue();
  return s_split(c, 1, *string_);
}

Value Value::split(const char *str) const {
  if (!is_string()) return ListValue();
  return s_split(str, strlen(str), *string_);
}

// ------------------------------------------------------------- join
template<typename T>
Value s_join(T &token, const List &list) {
  std::string res;

  size_t max = list.size();
  for(size_t i = 0; i < max; ++i) {
    if (!list[i]->is_string()) continue;
    if (res != "") res.append(token);
    res.append(list[i]->str());
  }
  return Value(res);
}

Value Value::join(const char *str) const {
  if (!is_list()) return StringValue();
  return s_join(str, *list_);
}

Value Value::join(char c) const {
  char buf[2];
  buf[0] = c;
  buf[1] = '\0';
  if (!is_list()) return StringValue();
  return s_join(buf, *list_);
}

/* ============================================= JSON Parser ========= */

#line 427 "/Users/gaspard/git/oscit/src/value.rl"


// transition table

#line 309 "/Users/gaspard/git/oscit/src/value.cpp"
static const char _json_actions[] = {
	0, 1, 0, 1, 3, 1, 4, 1, 
	7, 1, 11, 2, 1, 11, 2, 2, 
	11, 2, 6, 11, 2, 7, 11, 2, 
	8, 11, 2, 9, 11, 2, 10, 11, 
	3, 0, 8, 11, 3, 0, 9, 11, 
	3, 0, 10, 11, 3, 4, 7, 11, 
	3, 5, 4, 11, 3, 7, 0, 11, 
	4, 1, 5, 4, 11, 4, 2, 5, 
	4, 11, 4, 4, 7, 0, 11, 4, 
	6, 5, 4, 11, 5, 8, 5, 4, 
	0, 11, 5, 9, 5, 4, 0, 11, 
	5, 10, 5, 4, 0, 11
};

static const short _json_key_offsets[] = {
	0, 0, 18, 33, 35, 43, 60, 73, 
	75, 76, 76, 78, 78, 80, 83, 87, 
	87, 89, 89, 91, 102, 104, 113, 115, 
	122, 136, 138, 139, 154, 167, 169, 169, 
	171, 174, 178, 178, 185, 190, 195, 200, 
	205, 213, 218, 223, 228, 236, 241, 246, 
	251, 259, 273, 284, 286, 293, 293, 295, 
	295, 297, 307, 309, 318, 320, 327, 341, 
	343, 344, 359, 372, 374, 374, 376, 379, 
	383, 383, 390, 391, 392, 393, 394, 401, 
	402, 403, 404, 411, 412, 413, 414, 421, 
	421, 428, 441, 445, 453
};

static const char _json_trans_keys[] = {
	32, 34, 39, 43, 45, 91, 102, 110, 
	116, 123, 9, 10, 48, 57, 65, 90, 
	97, 122, 32, 34, 39, 43, 45, 91, 
	123, 9, 10, 48, 57, 65, 90, 97, 
	122, 34, 92, 0, 32, 44, 58, 93, 
	125, 9, 10, 0, 32, 34, 39, 44, 
	93, 125, 9, 10, 43, 45, 48, 57, 
	65, 90, 97, 122, 32, 34, 39, 43, 
	45, 9, 10, 48, 57, 65, 90, 97, 
	122, 34, 92, 58, 39, 92, 48, 57, 
	58, 48, 57, 32, 58, 9, 10, 39, 
	92, 48, 57, 0, 32, 44, 46, 58, 
	93, 125, 9, 10, 48, 57, 48, 57, 
	0, 32, 44, 93, 125, 9, 10, 48, 
	57, 44, 93, 0, 32, 44, 93, 125, 
	9, 10, 32, 34, 39, 43, 45, 125, 
	9, 10, 48, 57, 65, 90, 97, 122, 
	34, 92, 58, 32, 34, 39, 44, 125, 
	9, 10, 43, 45, 48, 57, 65, 90, 
	97, 122, 32, 34, 39, 43, 45, 9, 
	10, 48, 57, 65, 90, 97, 122, 39, 
	92, 48, 57, 58, 48, 57, 32, 58, 
	9, 10, 0, 32, 44, 93, 125, 9, 
	10, 32, 58, 97, 9, 10, 32, 58, 
	108, 9, 10, 32, 58, 115, 9, 10, 
	32, 58, 101, 9, 10, 0, 32, 44, 
	58, 93, 125, 9, 10, 32, 58, 117, 
	9, 10, 32, 58, 108, 9, 10, 32, 
	58, 108, 9, 10, 0, 32, 44, 58, 
	93, 125, 9, 10, 32, 58, 114, 9, 
	10, 32, 58, 117, 9, 10, 32, 58, 
	101, 9, 10, 0, 32, 44, 58, 93, 
	125, 9, 10, 32, 34, 39, 43, 45, 
	91, 102, 110, 116, 123, 9, 10, 48, 
	57, 32, 34, 39, 43, 45, 91, 123, 
	9, 10, 48, 57, 34, 92, 0, 32, 
	44, 93, 125, 9, 10, 39, 92, 48, 
	57, 0, 32, 44, 46, 93, 125, 9, 
	10, 48, 57, 48, 57, 0, 32, 44, 
	93, 125, 9, 10, 48, 57, 44, 93, 
	0, 32, 44, 93, 125, 9, 10, 32, 
	34, 39, 43, 45, 125, 9, 10, 48, 
	57, 65, 90, 97, 122, 34, 92, 58, 
	32, 34, 39, 44, 125, 9, 10, 43, 
	45, 48, 57, 65, 90, 97, 122, 32, 
	34, 39, 43, 45, 9, 10, 48, 57, 
	65, 90, 97, 122, 39, 92, 48, 57, 
	58, 48, 57, 32, 58, 9, 10, 0, 
	32, 44, 93, 125, 9, 10, 97, 108, 
	115, 101, 0, 32, 44, 93, 125, 9, 
	10, 117, 108, 108, 0, 32, 44, 93, 
	125, 9, 10, 114, 117, 101, 0, 32, 
	44, 93, 125, 9, 10, 0, 32, 44, 
	93, 125, 9, 10, 32, 34, 39, 43, 
	45, 9, 10, 48, 57, 65, 90, 97, 
	122, 32, 58, 9, 10, 0, 32, 44, 
	58, 93, 125, 9, 10, 0
};

static const char _json_single_lengths[] = {
	0, 10, 7, 2, 6, 7, 5, 2, 
	1, 0, 2, 0, 0, 1, 2, 0, 
	2, 0, 0, 7, 0, 5, 2, 5, 
	6, 2, 1, 5, 5, 2, 0, 0, 
	1, 2, 0, 5, 3, 3, 3, 3, 
	6, 3, 3, 3, 6, 3, 3, 3, 
	6, 10, 7, 2, 5, 0, 2, 0, 
	0, 6, 0, 5, 2, 5, 6, 2, 
	1, 5, 5, 2, 0, 0, 1, 2, 
	0, 5, 1, 1, 1, 1, 5, 1, 
	1, 1, 5, 1, 1, 1, 5, 0, 
	5, 5, 2, 6, 0
};

static const char _json_range_lengths[] = {
	0, 4, 4, 0, 1, 5, 4, 0, 
	0, 0, 0, 0, 1, 1, 1, 0, 
	0, 0, 1, 2, 1, 2, 0, 1, 
	4, 0, 0, 5, 4, 0, 0, 1, 
	1, 1, 0, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 2, 2, 0, 1, 0, 0, 0, 
	1, 2, 1, 2, 0, 1, 4, 0, 
	0, 5, 4, 0, 0, 1, 1, 1, 
	0, 1, 0, 0, 0, 0, 1, 0, 
	0, 0, 1, 0, 0, 0, 1, 0, 
	1, 4, 1, 1, 0
};

static const short _json_index_offsets[] = {
	0, 0, 15, 27, 30, 38, 51, 61, 
	64, 66, 67, 70, 71, 73, 76, 80, 
	81, 84, 85, 87, 97, 99, 107, 110, 
	117, 128, 131, 133, 144, 154, 157, 158, 
	160, 163, 167, 168, 175, 180, 185, 190, 
	195, 203, 208, 213, 218, 226, 231, 236, 
	241, 249, 262, 272, 275, 282, 283, 286, 
	287, 289, 298, 300, 308, 311, 318, 329, 
	332, 334, 345, 355, 358, 359, 361, 364, 
	368, 369, 376, 378, 380, 382, 384, 391, 
	393, 395, 397, 404, 406, 408, 410, 417, 
	418, 425, 435, 439, 447
};

static const char _json_indicies[] = {
	0, 2, 3, 4, 4, 7, 8, 9, 
	10, 11, 0, 5, 6, 6, 1, 0, 
	2, 3, 4, 4, 7, 11, 0, 5, 
	6, 6, 1, 13, 14, 12, 15, 15, 
	16, 17, 15, 15, 15, 1, 18, 19, 
	20, 21, 19, 18, 18, 19, 22, 23, 
	6, 6, 1, 24, 20, 21, 22, 22, 
	24, 23, 6, 6, 1, 26, 27, 25, 
	17, 1, 25, 26, 29, 28, 28, 23, 
	1, 17, 23, 1, 1, 17, 1, 6, 
	12, 13, 31, 30, 30, 5, 1, 32, 
	32, 33, 34, 17, 32, 32, 32, 5, 
	1, 35, 1, 32, 32, 33, 32, 32, 
	32, 35, 1, 7, 36, 1, 18, 18, 
	37, 18, 18, 18, 1, 11, 38, 39, 
	40, 40, 43, 11, 41, 42, 42, 1, 
	45, 46, 44, 47, 1, 48, 38, 39, 
	48, 49, 48, 40, 41, 42, 42, 1, 
	48, 38, 39, 40, 40, 48, 41, 42, 
	42, 1, 45, 51, 50, 50, 41, 1, 
	47, 41, 1, 1, 47, 1, 42, 44, 
	52, 52, 53, 52, 52, 52, 1, 1, 
	17, 54, 1, 6, 1, 17, 55, 1, 
	6, 1, 17, 56, 1, 6, 1, 17, 
	57, 1, 6, 58, 59, 60, 17, 58, 
	58, 59, 6, 1, 17, 61, 1, 6, 
	1, 17, 62, 1, 6, 1, 17, 63, 
	1, 6, 64, 65, 66, 17, 64, 64, 
	65, 6, 1, 17, 67, 1, 6, 1, 
	17, 68, 1, 6, 1, 17, 69, 1, 
	6, 70, 71, 72, 17, 70, 70, 71, 
	6, 73, 74, 75, 76, 76, 78, 79, 
	80, 81, 82, 73, 77, 1, 73, 74, 
	75, 76, 76, 78, 82, 73, 77, 1, 
	84, 85, 83, 86, 86, 86, 86, 86, 
	86, 1, 83, 84, 88, 87, 87, 77, 
	1, 89, 89, 89, 90, 89, 89, 89, 
	77, 1, 91, 1, 89, 89, 89, 89, 
	89, 89, 91, 1, 78, 92, 1, 93, 
	93, 93, 93, 93, 93, 1, 82, 94, 
	95, 96, 96, 99, 82, 97, 98, 98, 
	1, 101, 102, 100, 103, 1, 104, 94, 
	95, 104, 105, 104, 96, 97, 98, 98, 
	1, 104, 94, 95, 96, 96, 104, 97, 
	98, 98, 1, 101, 107, 106, 106, 97, 
	1, 103, 97, 1, 1, 103, 1, 98, 
	100, 108, 108, 108, 108, 108, 108, 1, 
	109, 1, 110, 1, 111, 1, 112, 1, 
	113, 113, 113, 113, 113, 113, 1, 114, 
	1, 115, 1, 116, 1, 117, 117, 117, 
	117, 117, 117, 1, 118, 1, 119, 1, 
	120, 1, 121, 121, 121, 121, 121, 121, 
	1, 1, 122, 122, 123, 122, 122, 122, 
	1, 24, 20, 21, 22, 22, 24, 23, 
	6, 6, 1, 1, 17, 1, 6, 124, 
	122, 125, 17, 124, 124, 122, 6, 1, 
	0
};

static const char _json_trans_targs[] = {
	2, 0, 3, 16, 18, 19, 14, 22, 
	36, 41, 45, 24, 3, 4, 15, 87, 
	88, 5, 87, 89, 7, 10, 12, 13, 
	6, 7, 8, 9, 10, 11, 16, 17, 
	87, 88, 20, 21, 23, 88, 25, 29, 
	31, 32, 33, 35, 25, 26, 34, 27, 
	28, 23, 29, 30, 87, 88, 37, 38, 
	39, 40, 90, 87, 91, 42, 43, 44, 
	90, 87, 91, 46, 47, 48, 90, 87, 
	91, 50, 51, 54, 56, 57, 60, 74, 
	79, 83, 62, 51, 52, 53, 92, 54, 
	55, 92, 58, 59, 61, 92, 63, 67, 
	69, 70, 71, 73, 63, 64, 72, 65, 
	66, 61, 67, 68, 92, 75, 76, 77, 
	78, 92, 80, 81, 82, 92, 84, 85, 
	86, 92, 87, 88, 90, 91
};

static const char _json_trans_actions[] = {
	0, 0, 0, 0, 1, 1, 1, 5, 
	1, 1, 1, 0, 1, 0, 0, 14, 
	61, 3, 9, 9, 0, 0, 1, 1, 
	0, 1, 0, 0, 1, 0, 1, 0, 
	11, 56, 1, 1, 7, 48, 0, 0, 
	1, 1, 1, 0, 1, 0, 0, 3, 
	0, 0, 1, 0, 17, 71, 1, 1, 
	1, 1, 40, 29, 88, 1, 1, 1, 
	32, 23, 76, 1, 1, 1, 36, 26, 
	82, 0, 0, 0, 1, 1, 5, 0, 
	0, 0, 0, 1, 0, 0, 14, 1, 
	0, 11, 1, 1, 7, 9, 0, 0, 
	1, 1, 1, 0, 1, 0, 0, 3, 
	0, 0, 1, 0, 17, 0, 0, 0, 
	0, 29, 0, 0, 0, 23, 0, 0, 
	0, 26, 20, 44, 52, 66
};

static const int json_start = 1;
static const int json_first_final = 87;
static const int json_error = 0;

static const int json_en_main_strict = 49;
static const int json_en_main_lazy = 1;


#line 431 "/Users/gaspard/git/oscit/src/value.rl"

/** This is a crude JSON parser. */
size_t Value::build_from_json(const char *json, bool strict_mode) {
  DEBUG(printf("\nbuild_from_json:\"%s\"\n",json));
  std::string str_buf;
  Value tmp_val;
  set_empty(); // clear
  // =============== Ragel job ==============

  int cs;
  const char * p  = json;
  const char * pe = json + strlen(p) + 1;

  
#line 566 "/Users/gaspard/git/oscit/src/value.cpp"
	{
	cs = json_start;
	}

#line 445 "/Users/gaspard/git/oscit/src/value.rl"

  if (strict_mode) {
    cs = json_en_main_strict;
  } else {
    cs = json_en_main_lazy;
  }

  
#line 580 "/Users/gaspard/git/oscit/src/value.cpp"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_keys = _json_trans_keys + _json_key_offsets[cs];
	_trans = _json_index_offsets[cs];

	_klen = _json_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _json_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += ((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _json_indicies[_trans];
	cs = _json_trans_targs[_trans];

	if ( _json_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _json_actions + _json_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 303 "/Users/gaspard/git/oscit/src/value.rl"
	{
     // append a char to build a std::string
    DEBUG(printf("%c-",(*p)));
    if ((*p))
      str_buf += (*p); /* append */
  }
	break;
	case 1:
#line 310 "/Users/gaspard/git/oscit/src/value.rl"
	{
    // become a RealValue
    tmp_val.set(atof(str_buf.c_str()));
    DEBUG(printf("[number %f/%s/%s\n]", tmp_val.r, str_buf.c_str(), tmp_val.to_json().c_str()));
    str_buf = "";
  }
	break;
	case 2:
#line 317 "/Users/gaspard/git/oscit/src/value.rl"
	{
    // become a StringValue
    tmp_val.set(str_buf);
    DEBUG(printf("[string %s]\n", tmp_val.to_json().c_str()));
    str_buf = "";
  }
	break;
	case 3:
#line 324 "/Users/gaspard/git/oscit/src/value.rl"
	{
    // Parse a single element of a hash (key:value)
    // Build tmp_val from string and move p forward
    p++;
    p += tmp_val.build_from_json(p, true);
    set(str_buf, tmp_val);
    p--;
    DEBUG(printf("[hash_value \"%s\":%s]\n", str_buf.c_str(), tmp_val.to_json().c_str()));
    DEBUG(printf("[continue \"%s\"]\n",p));

    str_buf = "";
  }
	break;
	case 4:
#line 337 "/Users/gaspard/git/oscit/src/value.rl"
	{
    // Parse a single element of a hash (key:value)
    // Build tmp_val from string and move p forward
    p++;
    p += tmp_val.build_from_json(p, true);
    push_back(tmp_val);
    if (*(p-1) == ',') p--; // hold the ',' separator

    DEBUG(printf("[%p:list_value %s ==> %s/%s]\n", this, tmp_val.to_json().c_str(), to_json().c_str(), p));
    p--; // eaten by >list_value sub-action
  }
	break;
	case 5:
#line 349 "/Users/gaspard/git/oscit/src/value.rl"
	{
    // we have a value in tmp that should be changed into a list [tmp]
    DEBUG(printf("[%p:lazy_list %s]\n", this, tmp_val.to_json().c_str()));
    push_back(tmp_val);
  }
	break;
	case 6:
#line 355 "/Users/gaspard/git/oscit/src/value.rl"
	{
    DEBUG(printf("[%p:empty_hash %s]\n", this, tmp_val.to_json().c_str()));
    // become an empty HashValue
    if (!is_hash()) {
      set_type(HASH_VALUE);
    }
  }
	break;
	case 7:
#line 363 "/Users/gaspard/git/oscit/src/value.rl"
	{
    if (!is_list()) set_type(LIST_VALUE);

    DEBUG(printf("[%p:list %s]\n", this, p));
    // FIXME: how to avoid 'return' by telling parsing to stop ?
    return p - json + 1;
  }
	break;
	case 8:
#line 371 "/Users/gaspard/git/oscit/src/value.rl"
	{
    // become a NilValue
    DEBUG(printf("[nil]\n"));
    tmp_val.set_type(NIL_VALUE);
  }
	break;
	case 9:
#line 377 "/Users/gaspard/git/oscit/src/value.rl"
	{
    // become a TrueValue
    DEBUG(printf("[true]\n"));
    tmp_val.set_type(TRUE_VALUE);
  }
	break;
	case 10:
#line 383 "/Users/gaspard/git/oscit/src/value.rl"
	{
    // become a FalseValue
    DEBUG(printf("[false]\n"));
    tmp_val.set_type(FALSE_VALUE);
  }
	break;
	case 11:
#line 389 "/Users/gaspard/git/oscit/src/value.rl"
	{
    DEBUG(printf("[set_from_tmp %s]\n", tmp_val.to_json().c_str()));
    if (!is_list() && !is_hash()) *this = tmp_val;
  }
	break;
#line 769 "/Users/gaspard/git/oscit/src/value.cpp"
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	_out: {}
	}

#line 453 "/Users/gaspard/git/oscit/src/value.rl"
  if (p != pe) --p;

  return p - json;
}

} // oscit


/*

// old stuff, remove if we decide we do not need to stream matrix data as json...

template<>
void MatrixData::to_stream(std::ostream& pStream) const
{
  char buffer[20];
  if (size() == 0) {
#ifdef _TESTING_
    if (sShowId)
      pStream << "<" << type_name() << "[" << mId << "] 0>";
    else
#endif
    pStream << "<" << type_name() << " 0>";

  } else {
    size_t sz = 16;
    size_t start;
    if (sz > size()) sz = size();
    start = size() - sz;
#ifdef _TESTING_
    if (sShowId)
      snprintf(buffer, 20, "<%s[%lu] [ % .2f", type_name(), mId, data[start]);
    else
#endif
    snprintf(buffer, 20, "<%s [ % .2f", type_name(), data[start]);

    pStream << buffer;
    for (size_t i= start+1; i < start+sz; i++) {
      snprintf(buffer, 20, " % .2f", data[i]);
      pStream << buffer;
    }
    pStream << " ], " << mRowCount << "x" << mColCount << ">";
  }
}


template<>
void CharMatrixData::to_stream(std::ostream& pStream) const
{
  if (size() == 0) {
#ifdef _TESTING_
    if (sShowId)
      pStream << "<" << type_name() << "[" << mId << "] 0>";
    else
#endif
    pStream << "<" << type_name() << " 0>";

  } else {
    size_t sz = 16;
    size_t start;
    if (sz > size()) sz = size();
    start = size() - sz;
#ifdef _TESTING_
    if (sShowId)
      pStream << "<" << type_name() << "[" << mId << "]" << " [ " << data[start];
    else
#endif
    pStream << "<" << type_name() << " [ " << data[start];

    for (size_t i= start+1; i < start+sz; i++) {
      pStream << " " << data[i];
    }
    pStream << " ], " << mRowCount << "x" << mColCount << ">";
  }
}


template<>
void MatrixData::to_json(std::ostream& pStream) const
{
  char buffer[20];

  size_t sz = size();
  pStream << "[";

  for (size_t i = 0; i < sz; i++) {
    if (i > 0) pStream << ",";
    snprintf(buffer, 20, "%.2f", data[i]);
    pStream << buffer;
  }

  pStream << "]";
}


template<>
void CharMatrixData::to_json(std::ostream& pStream) const
{
  size_t sz = size();
  pStream << "[";

  for (size_t i = 0; i < sz; i++) {
    if (i > 0) pStream << ",";
    pStream << data[i];
  }

  pStream << "]";
}
*/
