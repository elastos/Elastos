/* c_util.h -- internal utility state
 * Copyright (C) forever zhangcl 791398105@qq.com 
 * welcome to use freely
 */

#ifndef TEMPLATE_C_UTIL
#define TEMPLATE_C_UTIL

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <vector>

///
// This header will provid some util for C++'s using
///

// C Point for malloc, free
template<class T, class SIZETYPE=int>
class CMemPoint {
public:
	CMemPoint() {
		pValue = new Value(0);
		pValue->AddRef();
	}

	CMemPoint(T *pdata) {
		pValue = new Value(pdata);
		pValue->AddRef();
	}

	CMemPoint(const CMemPoint &p) {
		pValue = p.pValue;
		pValue->AddRef();
	}

	~CMemPoint() {
		pValue->Release();
	}

	CMemPoint &operator=(T *pdata) {
		pValue->Release();
		pValue = new Value(pdata);
		pValue->AddRef();

		return *this;
	}

	CMemPoint &operator=(const CMemPoint &p) {
		if (pValue == p.pValue)
			return *this;
		pValue->Release();
		pValue = p.pValue;
		pValue->AddRef();

		return *this;
	}

	operator bool() const {
		return pValue->data ? true : false;
	};

	operator void *() const {
		return (void *) pValue->data;
	}

	T &operator*() const {
		return *pValue->data;
	}

	T *&operator->() const {
		return pValue->data;
	}

	operator T *() const {
		return pValue->data;
	}

	T **operator&() const {
		return &pValue->data;
	}

	T *operator+(SIZETYPE off) const {
		return pValue->data + off;
	}

	T &operator[](SIZETYPE off) const {
		return pValue->data[off];
	}

private:
	class Value {
	public:
		SIZETYPE AddRef() {
			__sync_fetch_and_add(&_ref, 1);
			return _ref;
		}

		SIZETYPE Release() {
			__sync_fetch_and_sub(&_ref, 1);
			if (0 == _ref) {
				delete this;
				return 0;
			} else {
				return _ref;
			}
		}

		Value(T *pdata) {
			_ref = 0;
			data = pdata;
		}

		~Value() {
			if (data)
				free(data);
		}

		SIZETYPE _ref;
		T *data;
	};

	Value *pValue;
};

// CMemBlock for C block
template<class T, class SIZETYPE=size_t>
class CMemBlock {
public:
	CMemBlock() {
		pValue = new Value((size_t) 0);
		pValue->AddRef();
	}

	CMemBlock(SIZETYPE size) {
		pValue = new Value(size);
		pValue->AddRef();
	}

	CMemBlock(const CMemBlock &mem) {
		pValue = mem.pValue;
		pValue->AddRef();
	}

	~CMemBlock() {
		pValue->Release();
	}

	CMemBlock &operator=(const CMemBlock &mem) {
		if (pValue == mem.pValue)
			return *this;
		pValue->Release();
		pValue = mem.pValue;
		pValue->AddRef();

		return *this;
	}

	CMemBlock operator+(const CMemBlock &mem) const {
		CMemBlock ret;
		SIZETYPE l = pValue->_len + mem.pValue->_len;
		ret.Resize(l);
		memcpy(ret, pValue->data, sizeof(T) * pValue->_len);
		memcpy(ret + pValue->_len, mem.pValue->data, sizeof(T) * mem.pValue->_len);

		return ret;
	}

	CMemBlock &operator+=(const CMemBlock &mem) {
		SIZETYPE l = pValue->_len + mem.pValue->_len;
		pValue->Resize(l);
		memcpy(pValue->data + pValue->_len - mem.pValue->_len, mem.pValue->data, sizeof(T) * mem.pValue->_len);

		return *this;
	}

	void Zero() {
		pValue->Zero();
	}

	void Clear() {
		pValue->Clear();
	}

	void DelAt(SIZETYPE st) {
		pValue->DelAt(st);
	}

	SIZETYPE SetMem(T *pV, SIZETYPE len) {
		return pValue->SetMem(pV, len);
	}

	SIZETYPE SetMemFixed(const T *pV, SIZETYPE len) {
		return pValue->SetMemFixed(pV, len);
	}

	SIZETYPE Resize(SIZETYPE size) {
		return pValue->Resize(size);
	}

	SIZETYPE push_back(T &t) {
		return pValue->push_back(t);
	}

	SIZETYPE GetSize() const {
		return pValue->GetSize();
	}

	void Reverse() {
		pValue->Reverse();
	}

	operator bool() const {
		return pValue->data ? true : false;
	};

	operator void *() const {
		return (void *) pValue->data;
	}

	T &operator*() const {
		return *pValue->data;
	}

	operator T *() const {
		return pValue->data;
	}

	operator const T *() const {
		return pValue->data;
	}

	T **operator&() const {
		return &pValue->data;
	}

	T *operator+(SIZETYPE off) const {
		return pValue->data + off;
	}

	T &operator[](SIZETYPE off) const {
		return pValue->data[off];
	}

private:
	class Value {
		bool fixed;
	public:
		SIZETYPE AddRef() {
			__sync_fetch_and_add(&_ref, 1);
			return _ref;
		}

		SIZETYPE Release() {
			__sync_fetch_and_sub(&_ref, 1);
			if (0 == _ref) {
				delete this;
				return 0;
			} else {
				return _ref;
			}
		}

		Value(SIZETYPE size) {
			fixed = false;

			_ref = 0;
			if (0 < size) {
				data = (T *) malloc(size * sizeof(T));
				_len = size;
			} else {
				data = 0;
				_len = 0;
			}
		}

		~Value() {
			if (data && !fixed)
				free(data);
		}

		void Zero() {
			if (data && _len > 0) {
				for (SIZETYPE l = 0; l < _len; l++) {
					data[l] = 0;
				}
			}
		}

		void Clear() {
			if (data) {
				if (!fixed) free(data);
				data = 0;
				fixed = false;
			}
			_len = 0;
		}

		void DelAt(SIZETYPE st) {
			if (data && _len > 0) {
				if (0 <= st && st < _len) {
					T *p = (T *) malloc((_len - 1) * sizeof(T));
					for (SIZETYPE i = 0; i < _len; i++) {
						if (i < st) {
							p[i] = data[i];
						} else if (i > st) {
							p[i - 1] = data[i];
						}
					}
					if (!fixed) free(data);
					data = p;
					fixed = false;
					_len--;
				}
			}
		}

		SIZETYPE SetMem(T *pV, SIZETYPE len) {
			if (data && !fixed)
				free(data);
			data = pV;
			fixed = false;
			_len = len;
			return _len;
		}

		SIZETYPE SetMemFixed(const T *pV, SIZETYPE len) {
			if (data && !fixed)
				free(data);
			data = const_cast<T *>(pV);
			fixed = true;
			_len = len;
			return _len;
		}

		SIZETYPE Resize(SIZETYPE size) {
			if (size == _len)
				return size;
			if (0 < size) {
				T *t = (T *) malloc(size * sizeof(T));
				if (data) {
					SIZETYPE lt = _len > size ? size : _len;
					memcpy(t, data, lt * sizeof(T));
					if (!fixed) free(data);
				}
				data = t;
				fixed = false;
				_len = size;
			} else {
				if (data) {
					if (!fixed) free(data);
					data = 0;
					fixed = false;
					_len = 0;
				}
			}
			return _len;
		}

		SIZETYPE push_back(T &t) {
			T *pt = (T *) malloc((_len + 1) * sizeof(T));
			memcpy(pt, data, _len * sizeof(T));
			if (!fixed) free(data);
			data = pt;
			fixed = false;
			data[_len++] = t;
			return _len;
		}

		SIZETYPE GetSize() {
			return _len;
		}

		void Reverse() {
			if (0 < _len) {
				SIZETYPE Mid = _len / 2;
				T tmp;
				for (SIZETYPE i = 0; i < Mid; i++) {
					tmp = data[i];
					data[i] = data[_len - (i + 1)];
					data[_len - (i + 1)] = tmp;
				}
			}
		}

		SIZETYPE _ref;
		T *data;
		SIZETYPE _len;
	};

	Value *pValue;
};

typedef CMemBlock<uint8_t, uint64_t> CMBlock;

template<class T>
class CPObject {
public:
	CPObject() {
		pValue = new Value(0);
		pValue->AddRef();
	}

	CPObject(T *t) {
		pValue = new Value(t);
		pValue->AddRef();
	}

	CPObject(const CPObject &obj) {
		pValue = obj.pValue;
		pValue->AddRef();
	}

	~CPObject() {
		pValue->Release();
	}

public:
	CPObject &operator=(const CPObject &obj) {
		if (pValue == obj.pValue)
			return *this;
		pValue->Release();
		pValue = obj.pValue;
		pValue->AddRef();

		return *this;
	}

	CPObject &operator=(T *t) {
		if (pValue->m_t == t)
			return *this;
		if (pValue->m_t) {
			delete pValue->m_t;
			pValue->m_t = 0;
		}
		pValue->m_t = t;

		return *this;
	}

	operator bool() const {
		return pValue->m_t ? true : false;
	}

	operator T() const {
		return *pValue->m_t;
	}

	operator T *() const {
		return pValue->m_t;
	}

	T *&operator->() const {
		return pValue->m_t;
	}

private:
	class Value {
	public:
		size_t AddRef() {
			__sync_fetch_and_add(&_ref, 1);
			return _ref;
		}

		size_t Release() {
			__sync_fetch_and_sub(&_ref, 1);
			if (0 == _ref) {
				delete this;
				return 0;
			} else {
				return _ref;
			}
		}

		Value(T *t) {
			_ref = 0;
			m_t = t;
		}

		~Value() {
			if (m_t)
				delete m_t;
		}

		size_t _ref;
		T *m_t;
	};

	Value *pValue;
};


#endif