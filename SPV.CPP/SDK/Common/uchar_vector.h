////////////////////////////////////////////////////////////////////////////////
//
// uchar_vector.h
//
// Copyright (c) 2011-2012 Eric Lombrozo
// Copyright (c) 2011-2016 Ciphrex Corp.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//

#ifndef UCHAR_VECTOR_H__
#define UCHAR_VECTOR_H__

#include <stdio.h>
#include <stdint.h>

#include <iostream>
#include <cstring>

#include <vector>
#include <string>
#include <algorithm>

const char g_hexBytes[][3] = {
    "00","01","02","03","04","05","06","07","08","09","0a","0b","0c","0d","0e","0f",
    "10","11","12","13","14","15","16","17","18","19","1a","1b","1c","1d","1e","1f",
    "20","21","22","23","24","25","26","27","28","29","2a","2b","2c","2d","2e","2f",
    "30","31","32","33","34","35","36","37","38","39","3a","3b","3c","3d","3e","3f",
    "40","41","42","43","44","45","46","47","48","49","4a","4b","4c","4d","4e","4f",
    "50","51","52","53","54","55","56","57","58","59","5a","5b","5c","5d","5e","5f",
    "60","61","62","63","64","65","66","67","68","69","6a","6b","6c","6d","6e","6f",
    "70","71","72","73","74","75","76","77","78","79","7a","7b","7c","7d","7e","7f",
    "80","81","82","83","84","85","86","87","88","89","8a","8b","8c","8d","8e","8f",
    "90","91","92","93","94","95","96","97","98","99","9a","9b","9c","9d","9e","9f",
    "a0","a1","a2","a3","a4","a5","a6","a7","a8","a9","aa","ab","ac","ad","ae","af",
    "b0","b1","b2","b3","b4","b5","b6","b7","b8","b9","ba","bb","bc","bd","be","bf",
    "c0","c1","c2","c3","c4","c5","c6","c7","c8","c9","ca","cb","cc","cd","ce","cf",
    "d0","d1","d2","d3","d4","d5","d6","d7","d8","d9","da","db","dc","dd","de","df",
    "e0","e1","e2","e3","e4","e5","e6","e7","e8","e9","ea","eb","ec","ed","ee","ef",
    "f0","f1","f2","f3","f4","f5","f6","f7","f8","f9","fa","fb","fc","fd","fe","ff"
};

const char base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

typedef unsigned int uint;

/// TODO: Add secure allocator capabilities
//template < class Allocator = allocator<unsigned char> >
class uchar_vector : public std::vector<unsigned char>//, Allocator>
{
public:
    uchar_vector();
    uchar_vector(size_type n, const unsigned char& value = 0);
    template <class InputIterator> uchar_vector(InputIterator first, InputIterator last) : std::vector<unsigned char>(first, last) { }
    uchar_vector(const std::vector<unsigned char>& vec);
    uchar_vector(const void *array, unsigned int size);
    uchar_vector(const std::string& hex);

    bool operator <(const uchar_vector &rhs) const;

    bool operator ==(const uchar_vector &rhs) const;

    bool operator !=(const uchar_vector &rhs) const;

    uchar_vector& operator+=(const std::vector<unsigned char>& rhs);

    uchar_vector& operator<<(const std::vector<unsigned char>& rhs);

    uchar_vector& operator<<(unsigned char byte);
        
    const uchar_vector operator+(const std::vector<unsigned char>& rightOperand) const;

    uchar_vector& operator=(const std::string& hex);

    void copyToArray(unsigned char* array);

    void padLeft(unsigned char pad, uint total_length);

    void padRight(unsigned char pad, uint total_length);

    template <class T>
    void append(T n)
    {
    	operator+=(uchar_vector(&n, sizeof(T)));
    }

    void append(const void *array, size_t size);

    std::string getHex(bool spaceBytes = false) const;

    void setHex(std::string hex);

    void reverse();

    uchar_vector getReverse() const;

    std::string getCharsAsString() const;

    void setCharsFromString(const std::string& chars);

    std::string getBase64() const;

    void setBase64(const std::string &base64);

    void clean();

    bool isZero() const;
};

typedef std::string string_secure;
typedef uchar_vector uchar_vector_secure; // not really :p at least not yet!
/*
//
// Allocator that locks its contents from being paged
// out of memory and clears its contents before deletion.
//
template<typename T>
struct secure_allocator : public std::allocator<T>
{
  // MSVC8 default copy constructor is broken
  typedef std::allocator<T> base;
  typedef typename base::size_type size_type;
  typedef typename base::difference_type  difference_type;
  typedef typename base::pointer pointer;
  typedef typename base::const_pointer const_pointer;
  typedef typename base::reference reference;
  typedef typename base::const_reference const_reference;
  typedef typename base::value_type value_type;
  secure_allocator() throw() {}
  secure_allocator(const secure_allocator& a) throw() : base(a) {}
  template <typename U>
  secure_allocator(const secure_allocator<U>& a) throw() : base(a) {}
  ~secure_allocator() throw() {}
  template<typename _Other> struct rebind
  { typedef secure_allocator<_Other> other; };

  T* allocate(std::size_t n, const void *hint = 0)
  {
      T *p;
      p = std::allocator<T>::allocate(n, hint);
      if (p != NULL)
          mlock(p, sizeof(T) * n);
      return p;
  }

  void deallocate(T* p, std::size_t n)
  {
      if (p != NULL)
      {
          memset(p, 0, sizeof(T) * n);
          munlock(p, sizeof(T) * n);
      }
      std::allocator<T>::deallocate(p, n);
  }
};
*/
#endif
