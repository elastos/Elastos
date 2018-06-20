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
#include <cstdint>

// CMemBlock for C block
template<class T, class SIZETYPE=size_t>
class CMemBlock {
public:
	CMemBlock() {
		pValue = new Value((SIZETYPE) 0);
		pValue->AddRef();
	}

	CMemBlock(SIZETYPE size) {
		pValue = new Value(size);
		pValue->AddRef();
	}

	CMemBlock(const CMemBlock &mem) {
		pValue = mem.pValue;
		if (nullptr != pValue)
			pValue->AddRef();
	}

	CMemBlock(CMemBlock &mem) {
		pValue = mem.pValue;
		if (nullptr != pValue)
			pValue->AddRef();
	}

	~CMemBlock() {
		if (nullptr != pValue)
			pValue->Release();
	}

	CMemBlock &operator=(const CMemBlock &mem) {
		if (nullptr != pValue && pValue == mem.pValue)
			return *this;
		if (nullptr != pValue)
			pValue->Release();
		pValue = mem.pValue;
		if (nullptr != pValue)
			pValue->AddRef();

		return *this;
	}

	CMemBlock &operator=(CMemBlock &mem) {
		if (nullptr != pValue && pValue == mem.pValue)
			return *this;
		if (nullptr != pValue)
			pValue->Release();
		pValue = mem.pValue;
		if (nullptr != pValue)
			pValue->AddRef();

		return *this;
	}

	CMemBlock operator+(const CMemBlock &mem) const {
		CMemBlock ret;
		SIZETYPE l = (nullptr != pValue ? pValue->_len : 0) + (nullptr != mem.Pvalue ? mem.pValue->_len : 0);
		ret.Resize(l);
		memcpy(ret, nullptr != pValue ? pValue->data : 0, sizeof(T) * (nullptr != pValue ? pValue->_len : 0));
		memcpy(ret + (nullptr != pValue ? pValue->_len : 0), nullptr != mem.pValue ? mem.pValue->data : 0,
			   sizeof(T) * (nullptr != mem.pValue ? mem.pValue->_len : 0));

		return ret;
	}

	CMemBlock operator+(CMemBlock &mem) const {
		CMemBlock ret;
		SIZETYPE l = (nullptr != pValue ? pValue->_len : 0) + (nullptr != mem.Pvalue ? mem.pValue->_len : 0);
		ret.Resize(l);
		memcpy(ret, nullptr != pValue ? pValue->data : 0, sizeof(T) * (nullptr != pValue ? pValue->_len : 0));
		memcpy(ret + (nullptr != pValue ? pValue->_len : 0), nullptr != mem.pValue ? mem.pValue->data : 0,
			   sizeof(T) * (nullptr != mem.pValue ? mem.pValue->_len : 0));

		return ret;
	}

	CMemBlock &operator+=(const CMemBlock &mem) {
		if (nullptr == pValue) {
			pValue = new Value((SIZETYPE) 0);
			pValue->AddRef();
		}
		SIZETYPE l = pValue->_len + (nullptr != mem.pValue ? mem.pValue->_len : 0);
		pValue->Resize(l);
		memcpy(pValue->data + pValue->_len, nullptr != mem.pValue ? mem.pValue->data : 0,
			   sizeof(T) * (nullptr != mem.pValue ? mem.pValue->_len : 0));

		return *this;
	}

	CMemBlock &operator+=(CMemBlock &mem) {
		if (nullptr == pValue) {
			pValue = new Value((SIZETYPE) 0);
			pValue->AddRef();
		}
		SIZETYPE l = pValue->_len + (nullptr != mem.pValue ? mem.pValue->_len : 0);
		pValue->Resize(l);
		memcpy(pValue->data + pValue->_len, nullptr != mem.pValue ? mem.pValue->data : 0,
			   sizeof(T) * (nullptr != mem.pValue ? mem.pValue->_len : 0));

		return *this;
	}

	void Zero() {
		if (nullptr != pValue)
			pValue->Zero();
	}

	void Clear() {
		if (nullptr != pValue)
			pValue->Clear();
	}

	void DelAt(SIZETYPE st) {
		if (nullptr != pValue)
			pValue->DelAt(st);
	}

	SIZETYPE SetMem(T *pV, SIZETYPE len) {
		return nullptr != pValue ? pValue->SetMem(pV, len) : 0;
	}

	SIZETYPE SetMemFixed(const T *pV, SIZETYPE len) {
		return nullptr != pValue ? pValue->SetMemFixed(pV, len) : 0;
	}

	SIZETYPE Resize(SIZETYPE size) {
		return nullptr != pValue ? pValue->Resize(size) : 0;
	}

	SIZETYPE push_back(T &t) {
		return nullptr != pValue ? pValue->push_back(t) : 0;
	}

	SIZETYPE GetSize() const {
		return nullptr != pValue ? pValue->GetSize() : 0;
	}

	SIZETYPE GetSize() {
		return nullptr != pValue ? pValue->GetSize() : 0;
	}

	void Reverse() {
		nullptr != pValue ? pValue->Reverse() : 0;
	}

	operator bool() const {
		return nullptr != pValue ? pValue->data ? true : false : false;
	};

	operator bool() {
		return nullptr != pValue ? pValue->data ? true : false : false;
	};

	operator void *() const {
		return (void *) nullptr != pValue ? pValue->data : 0;
	}

	operator void *() {
		return (void *) nullptr != pValue ? pValue->data : 0;
	}

	operator const void *() const {
		return (void *) nullptr != pValue ? pValue->data : 0;
	}

	operator const void *() {
		return (void *) nullptr != pValue ? pValue->data : 0;
	}

	T &operator*() const {
		T t;
		return nullptr != pValue ? *pValue->data : t;
	}

	T &operator*() {
		T t;
		return nullptr != pValue ? *pValue->data : t;
	}

	operator T *() const {
		return nullptr != pValue ? pValue->data : 0;
	}

	operator T *() {
		return nullptr != pValue ? pValue->data : 0;
	}

	operator const T *() const {
		return nullptr != pValue ? pValue->data : 0;
	}

	T **operator&() const {
		return nullptr != pValue ? &pValue->data : 0;
	}

	T **operator&() {
		return nullptr != pValue ? &pValue->data : 0;
	}

	T *operator+(uint64_t off) const {
		return nullptr != pValue ? pValue->data + off : 0;
	}

	T *operator+(uint64_t off) {
		return nullptr != pValue ? pValue->data + off : 0;
	}

	T *operator+(int off) const {
		return nullptr != pValue ? pValue->data + off : 0;
	}

	T *operator+(int off) {
		return nullptr != pValue ? pValue->data + off : 0;
	}

	T *operator+(size_t off) const {
		return nullptr != pValue ? pValue->data + off : 0;
	}

	T *operator+(size_t off) {
		return nullptr != pValue ? pValue->data + off : 0;
	}

	T &operator[](uint64_t off) const {
		T t;
		return nullptr != pValue->data ? pValue->data[off] : t;
	}

	T &operator[](uint64_t off) {
		T t;
		return nullptr != pValue->data ? pValue->data[off] : t;
	}

	T &operator[](int off) const {
		T t;
		return nullptr != pValue->data ? pValue->data[off] : t;
	}

	T &operator[](int off) {
		T t;
		return nullptr != pValue->data ? pValue->data[off] : t;
	}

	T &operator[](size_t off) const {
		T t;
		return nullptr != pValue->data ? pValue->data[off] : t;
	}

	T &operator[](size_t off) {
		T t;
		return nullptr != pValue->data ? pValue->data[off] : t;
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
			if (nullptr != data && !fixed)
				free(data);
		}

		void Zero() {
			if (nullptr != data && _len > 0) {
				for (SIZETYPE l = 0; l < _len; l++) {
					data[l] = 0;
				}
			}
		}

		void Clear() {
			if (nullptr != data) {
				if (!fixed) free(data);
				data = 0;
				fixed = false;
			}
			_len = 0;
		}

		void DelAt(SIZETYPE st) {
			if (nullptr != data && _len > 0) {
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
			if (nullptr != data && !fixed)
				free(data);
			data = pV;
			fixed = false;
			_len = len;
			return _len;
		}

		SIZETYPE SetMemFixed(const T *pV, SIZETYPE len) {
			if (nullptr != data && !fixed)
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
				if (nullptr != data) {
					SIZETYPE lt = _len > size ? size : _len;
					memcpy(t, data, lt * sizeof(T));
					if (!fixed) free(data);
				}
				data = t;
				fixed = false;
				_len = size;
			} else {
				if (nullptr != data) {
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
			if (nullptr != data && !fixed) free(data);
			data = pt;
			fixed = false;
			data[_len++] = t;
			return _len;
		}

		SIZETYPE GetSize() {
			return _len;
		}

		void Reverse() {
			if (nullptr != data && 0 < _len) {
				SIZETYPE id = _len / 2;
				T tmp;
				for (SIZETYPE i = 0; i < id; i++) {
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


#endif