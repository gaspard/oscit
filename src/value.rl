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
%%{
  machine json;

  action str_a {
     // append a char to build a std::string
    DEBUG(printf("%c-",fc));
    if (fc)
      str_buf += fc; /* append */
  }

  action number {
    // become a RealValue
    tmp_val.set(atof(str_buf.c_str()));
    DEBUG(printf("[number %f/%s/%s\n]", tmp_val.r, str_buf.c_str(), tmp_val.to_json().c_str()));
    str_buf = "";
  }

  action string {
    // become a StringValue
    tmp_val.set(str_buf);
    DEBUG(printf("[string %s]\n", tmp_val.to_json().c_str()));
    str_buf = "";
  }

  action hash_value {
    // Parse a single element of a hash (key:value)
    // Build tmp_val from string and move p forward
    p++;
    p += tmp_val.build_from_json(p, true);
    set(str_buf, tmp_val);
    fhold;
    DEBUG(printf("[hash_value \"%s\":%s]\n", str_buf.c_str(), tmp_val.to_json().c_str()));
    DEBUG(printf("[continue \"%s\"]\n",p));

    str_buf = "";
  }

  action list_value {
    // Parse a single element of a hash (key:value)
    // Build tmp_val from string and move p forward
    p++;
    p += tmp_val.build_from_json(p, true);
    push_back(tmp_val);
    if (*(p-1) == ',') fhold; // hold the ',' separator

    DEBUG(printf("[%p:list_value %s ==> %s/%s]\n", this, tmp_val.to_json().c_str(), to_json().c_str(), p));
    fhold; // eaten by >list_value sub-action
  }

  action lazy_list {
    // we have a value in tmp that should be changed into a list [tmp]
    DEBUG(printf("[%p:lazy_list %s]\n", this, tmp_val.to_json().c_str()));
    push_back(tmp_val);
  }

  action empty_hash {
    DEBUG(printf("[%p:empty_hash %s]\n", this, tmp_val.to_json().c_str()));
    // become an empty HashValue
    if (!is_hash()) {
      set_type(HASH_VALUE);
    }
  }

  action list {
    if (!is_list()) set_type(LIST_VALUE);

    DEBUG(printf("[%p:list %s]\n", this, p));
    // FIXME: how to avoid 'return' by telling parsing to stop ?
    return p - json + 1;
  }

  action nil {
    // become a NilValue
    DEBUG(printf("[nil]\n"));
    tmp_val.set_type(NIL_VALUE);
  }

  action true {
    // become a TrueValue
    DEBUG(printf("[true]\n"));
    tmp_val.set_type(TRUE_VALUE);
  }

  action false {
    // become a FalseValue
    DEBUG(printf("[false]\n"));
    tmp_val.set_type(FALSE_VALUE);
  }

  action set_from_tmp {
    DEBUG(printf("[set_from_tmp %s]\n", tmp_val.to_json().c_str()));
    if (!is_list() && !is_hash()) *this = tmp_val;
  }

  ws        = ' ' | '\t' | '\n';
  end       = ws  | '\0' | '}' | ',' | ']';  # we need '}' and ']' to finish value when embedded in hash: {one:1.34}
  dquote_content = ([^"\\] | '\n') $str_a | ('\\' (any | '\n') $str_a);
  squote_content = ([^'\\] | '\n') $str_a | ('\\' (any | '\n') $str_a);
  word      = ws* (alpha [^ \t\n:]*) $str_a;
  real      = ws* ([\-+]? $str_a ('0'..'9' digit* '.' digit+) $str_a );
  integer   = ws* ([\-+]? $str_a ('0'..'9' digit*) $str_a );
  nil       = 'null';
  true      = 'true';
  false     = 'false';
  number    = real | integer;
  string    = ws* ('"' dquote_content* '"' | '\'' squote_content* '\'');

  hash_content = (string | word | integer) ':' >hash_value;

  strict    = ws* '[' >list_value (',' >list_value)* ']' $list    |
              ws* '{' hash_content (','? hash_content)* '}' |
              ws* '{' ws* '}'        %empty_hash |
                      string             %string |
                      number             %number |
                      true               %true   |
                      false              %false  |
                      nil                %nil;

  lazy_list_content = strict %lazy_list ',' >list_value (',' >list_value)*;

  lazy      = lazy_list_content          %list   |
              hash_content (','? hash_content)*  |
              strict;

  main_strict := strict %set_from_tmp end;
  main_lazy   := lazy   %set_from_tmp end;

}%%

// transition table
%% write data;

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

  %% write init;

  if (strict_mode) {
    cs = json_en_main_strict;
  } else {
    cs = json_en_main_lazy;
  }

  %% write exec;
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
