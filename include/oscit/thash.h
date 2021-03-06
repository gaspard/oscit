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

#ifndef OSCIT_INCLUDE_OSCIT_THASH_H_
#define OSCIT_INCLUDE_OSCIT_THASH_H_

#include <cstdio>
#include <string>
#include <list>
#include <iostream>

typedef unsigned int uint;

namespace oscit {

typedef std::list<std::string>::iterator StringIterator;
typedef std::list<std::string>::const_iterator ConstStringIterator;

template<class K, class T>
struct THashElement
{
  THashElement() : obj(0), next(0) {}
  ~THashElement() { if (obj) delete obj; }
  K     key;
  T*    obj;
  /// lookup collision chain
  THashElement<K,T> * next;
};

/** Dictionary. The dictionary stores a copy of the data. Pointers to the copy are returned.
  * We return pointers so that we can return NULL if the value is not found.
  * K is the key class, T is the object class
  */
template <class K, class T>
class THash {
public:
  typedef typename std::list<K>::const_iterator ConstIterator;
  typedef typename std::list<K>::iterator Iterator;

  THash(unsigned int size) : size_(size) {
    thash_table_ = new THashElement<K,T>[size];
  }

  // copy constructor (even if we do not use it, we need it for object instantiation: explicit not possible)
  THash(const THash& other) {
    copy(other);
  }

  virtual ~THash() {
    THashElement<K,T> * current, * next;
    // remove collisions
    for(size_t i=0;i<size_;i++) {
      current = thash_table_[i].next;
      while (current) {
        next = current->next;
        delete current;
        current = next;
      }
    }
    // remove table
    delete[] thash_table_;
  }

  THash& operator=(const THash& other) {
    copy(other);
    return *this;
  }

  void copy(const THash &other) {
    ConstIterator it;
    ConstIterator end = other.end();
    T value;

    size_ = other.size_;
    thash_table_ = new THashElement<K,T>[size_];

    for(it = other.begin(); it != end; it++) {
      if (other.get(*it, &value)) {
        set(*it, value);
      }
    }
  }

  void set(const K &key, const T &pElement);

  /** Get an element of the dictionary and set the retval to this element. Returns false if no element found.
   */
  // FIXME: const T* ?
  bool get(const K &key, T *retval) const;

  /** Return true if the dictionary contains an element at the given key.
   */
  bool has_key(const K &key) const;

  /** Get a pointer to an element in the dictionary.
   * The value of this pointer should not be kept (it may change as the hash is updated).
   * Returns false if no element found.
   */
  bool get(const K &key, const T **retval) const;

  /** Get a pointer to an element in the dictionary (non-const version).
   * The value of this pointer should not be kept (it may change as the hash is updated).
   * Returns false if no element found.
   */
  bool get(const K &key, T **retval);

  /** Get an element's key. Returns false if the element could not be found. */
  bool get_key(const T &pElement, K *retval) const;

  /** Get the default value (last value). */
  bool get(T *retval) const;

  /** Remove object with the given key. */
  void remove(const K &key) {
    if (remove_keeping_key(key)) {
      // object has been removed, remove key
      Iterator it  = keys_.begin();
      Iterator end = keys_.end();
      while (it != end) {
        if (*it == key)
          it = keys_.erase(it);
        else
          it++;
      }
    }
  }

  /** Remove the given element. */
  void remove_element(const T &pElement);

  /** Remove all objects. */
  void clear() {
    Iterator it;
    Iterator end = keys_.end();
    for (it = keys_.begin(); it != end; ++it) {
      remove_keeping_key(*it);
    }
    keys_.clear();
  }

  /** Return true if the dictionary is empty. */
  bool empty() const { return size() == 0; }

  /** Return number of elements (distinct keys). */
  size_t size() const { return keys_.size(); }

  bool operator==(const THash& other) const {
    if (size() != other.size()) return false;
    ConstIterator it, end = keys_.end();
    const T *val1;
    const T *val2;
    for (it = keys_.begin(); it != end; ++it) {
      if (get(*it, &val1) && other.get(*it, &val2)) {
        if (*val1 != *val2) return false;
      } else {
        return false;
      }
    }
    return true;
  }

  /** Return size of storage (main hash table). */
  unsigned int storage_size() const { return size_; }

  void to_stream(std::ostream &out_stream, bool lazy = false) const;

  /** List of keys. */
  const std::list<K> &keys() const { return keys_; }

  /** Begin iterator over the keys of the dictionary (read-only). */
  ConstIterator begin() const { return keys_.begin(); }

  /** Past end iterator over the keys of the dictionary (read-only). */
  ConstIterator end()   const { return keys_.end(); }

  /** Begin iterator over the keys of the dictionary. */
  Iterator begin() { return keys_.begin(); }

  /** Begin iterator over the keys of the dictionary. */
  Iterator end()   { return keys_.end(); }
protected:
  bool remove_keeping_key(const K &key);

  /* data */
  THashElement<K,T> * thash_table_;
  std::list<K>        keys_;

  unsigned int size_;
};

template <class K, class Te>
void THash<K,Te>::set(const K& key, const Te& pElement) {
  THashElement<K,Te> *  found;
  THashElement<K,Te> ** set_next;
  uint id = hashId(key) % size_;
  found    = &(thash_table_[id]);  // pointer to found element
  set_next = &(found->next);      // where to write the new inserted value if there is one

  while (found && found->obj && found->key != key) { // found->obj is for the case where key == 0
    set_next = &(found->next);
    found    = found->next;
  }
  if (found && !found->obj) {
    // in table value
  } else if (!found) {
    // collision
    found = new THashElement<K,Te>;
    if (!found) {
      // FIXME: alloc error. Raise ?
      fprintf(stderr, "Could not allocate new hash pointer.\n");
      return;
    }
    *set_next = found;
  }
  if (found->obj) {
    // replace value for given key
    delete found->obj;
  } else {
    // new key
    keys_.push_back(key);
  }
  found->obj  = new Te(pElement); // new T(pElement) does not compile in mimas with libjuce...
  found->key  = key;              // T is a macro to create juce String elements...
}

template <class K, class T>
bool THash<K,T>::get(const K &key, T *retval) const {
  THashElement<K,T> *found;
  uint id = hashId(key) % size_;

  found = &(thash_table_[id]);
  while (found && found->obj && found->key != key)
    found = found->next;

  if (found && found->obj && found->key == key) {
    *retval = *(found->obj);
    return true;
  } else {
    return false;
  }
}

template <class K, class T>
bool THash<K,T>::has_key(const K &key) const {
  THashElement<K,T> *found;
  uint id = hashId(key) % size_;

  found = &(thash_table_[id]);
  while (found && found->obj && found->key != key)
    found = found->next;

  if (found && found->obj && found->key == key) {
    return true;
  } else {
    return false;
  }
}

template <class K, class T>
bool THash<K,T>::get(const K &key, const T **retval) const {
  THashElement<K,T> *found;
  uint id = hashId(key) % size_;

  found = &(thash_table_[id]);
  while (found && found->obj && found->key != key)
    found = found->next;

  if (found && found->obj && found->key == key) {
    *retval = found->obj;
    return true;
  } else {
    return false;
  }
}

template <class K, class T>
bool THash<K,T>::get(const K &key, T **retval) {
  THashElement<K,T> *found;
  uint id = hashId(key) % size_;

  found = &(thash_table_[id]);
  while (found && found->obj && found->key != key)
    found = found->next;

  if (found && found->obj && found->key == key) {
    *retval = found->obj;
    return true;
  } else {
    return false;
  }
}

template <class K, class T>
bool THash<K,T>::get_key(const T &pElement, K *retval) const {
  ConstIterator it;
  ConstIterator end = keys_.end();

  T element;

  for(it = keys_.begin(); it != end; it++) {
    if (get(*it, &element) && element == pElement) {
      *retval = *it;
      return true;
    }
  }
  return false;
}


template <class K, class T>
bool THash<K,T>::remove_keeping_key(const K& key) {
  bool element_removed = false;
  THashElement<K,T> *  found, * next;
  THashElement<K,T> ** set_next;
  uint id = hashId(key) % size_;
  found    = &(thash_table_[id]);  // pointer to found element
  set_next = NULL;                // where to write removed element's next if there is one

  while (found && found->obj && found->key != key) {
    set_next = &(found->next);
    found    = found->next;
  }
  if (found && found->obj) {
    element_removed = true;
    if (set_next) {
      // link previous to next
      *set_next = found->next;
      delete found;
    } else {
      // in table
      if ( (next = found->next) ) {
        // There was a collision. Move chain up.
        if (found->obj) {
          delete found->obj;
        }
        found->obj  = next->obj;
        next->obj   = NULL;  // to avoid Real delete
        found->key  = next->key;
        found->next = next->next;
        delete next;
      } else {
        // no collision. free.
        delete found->obj;
        found->obj  = NULL;
      }
    }
  }
  return element_removed;
}


template <class K, class T>
void THash<K,T>::remove_element(const T& pElement) {
  K key;
  while (get_key(pElement, &key)) {
    remove(key);
  }
}

template <class K, class T>
std::ostream& operator<< (std::ostream& pStream, const THash<K,T>& hash) {
  typename THash<K,T>::ConstIterator it,begin,end;
  end   = hash.end();
  begin = hash.begin();
  T value;
  for( it = begin; it != end; it++) {
    if (it != begin) pStream << " ";
    if (hash.get(*it, &value))
      pStream << *it << ":" << value;
    else
      pStream << *it << ":" << "/error/";
  }
  return pStream;
}

template <class T>
std::ostream& operator<< (std::ostream& out_stream, const THash<std::string,T>& hash) {
  hash.to_stream(out_stream, false);
  return out_stream;
}

// FIXME: this should be restricted to K == string...
template <class K, class T>
void THash<K,T>::to_stream(std::ostream &out_stream, bool lazy) const {
  typename THash<K,T>::ConstIterator it,begin,end;
  end   = keys_.end();
  begin = keys_.begin();
  T value;
  for( it = begin; it != end; it++) {
    if (it != begin) out_stream << (lazy ? " " : ", ");
    if (lazy) {
      if (get(*it, &value)) {
        out_stream << *it << ":" << value;
      } else {
        out_stream << *it << ":" << "/error/";
      }
    } else {
      if (get(*it, &value)) {
        // FIXME: escape(*it) !
        out_stream << "\"" << *it << "\"" << ":" << value;
      } else {
        out_stream << "\"" << *it << "\"" << ":" << "/error/";
      }
    }
  }
}

/////// HASH FUNCTIONS  ////////

// ===== uint =====
// Thomas Wang's 32 Bit Mix Function: http://www.cris.com/~Ttwang/tech/inthash.htm
inline uint hashId(const uint key) {
  uint res = key;
  res += ~(res << 15);
  res ^= (res >> 10);
  res += (res << 3);
  res ^= (res >> 6);
  res += ~(res << 11);
  res ^= (res >> 16);
  return res;
}

inline uint hashId(const int key) {
  return hashId((uint)key);
}

inline uint hashId(const unsigned long key) {
  uint res = key;
  res += ~(res << 15);
  res ^= (res >> 10);
  res += (res << 3);
  res ^= (res >> 6);
  res += ~(res << 11);
  res ^= (res >> 16);
  return res;
}

// ===== char *      =====
/** Hashing function null terminated character strings.
 * sdbm function: taken from http://www.cse.yorku.ca/~oz/hash.html
 * This is *not* the same as the Hash macro !
 */
inline uint hashId(const char *str) {
  unsigned long h = 0;
  int c;

  while ( (c = *str++) ) {
    h = c + (h << 6) + (h << 16) - h;
  }

  return h;
}

// We use the simpler hash to avoid too long compile times in the static string hash macro.
inline uint hashId(const char c) {
  return (uint)c;
}

// ===== std::string& =====
inline uint hashId(const std::string &key) {
  return hashId(key.c_str());
}


} // oscit

#endif // OSCIT_INCLUDE_OSCIT_THASH_H_
