/* c_util.h -- internal utility state
 * Copyright (C) forever zhangcl 791398105@qq.com 
 * welcome to use freely
 */

#ifndef TEMPLATE_C_UTIL
#define TEMPLATE_C_UTIL

//#define USE_VARY_MACRO

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <cstdint>


/** As Smart Block, CMemBlock containing any data type recognized by C such as
 *  through malloc calloc..., auto freed when ending with ref number, surely used in module.
 *  \template T  data type contained by Smart Block.
 *  \template SIZETYPE  index/operator type with Smart Block.
 *  size_type is was used for index/operator type outer
 *
 *  Demo 1:
 *  CMemBlock<int> cmBlock;
 *  cmBlock::size_type i, j = 3, n = 10;
 *  for (i = 0; i < n; i++) {
 *  	cmBlock[i] = 0;
 *  }
 *  *(cmBlock + j) = 16;
 *  *(cmBlock + 2) = 15;
 *  cmBlock[7] = 8;
 *  cmBlock[9] = 10;
 *
 *  Demo 2:
 *  CMemBlock<int> cmBlock;
 *  int *arrCon = (int *) malloc(100 * sizeof(int));
 *  cmBlock.SetMem(arrCon, 100 * sizeof(int));
 *  for (CMemBlock<int>::size_type i = 0; i < 100; i++) {
 *  	cmBlock[i] = i;
 *  }
 *
 *  Demo 3:
 *  CMemBlock<int> cmBlock1, cmBlock2;
 *	Anytype arr1[10] = {0, 1, 2, 3, 4};
 *	Anytype arr2[10] = {0, 1, 2, 3, 4};
 *	cmBlock1.SetMemFixed(arr1, sizeof(arr1));
 *	cmBlock2.SetMemFixed(arr2, sizeof(arr2));
 *	CMemBlock<int> cmTotal = cmBlock1 + cmBlock2;
 *	memcpy(cmTotal + 3, arr1, 3);
 *
 *	Demo 4:
 *	CMemBlock<double> cmBlock;
 *	int size = 100;
 *	double arr[size];
 *	memset(arr, 0, sizeof(arr));
 *	cmBlock.Resize(CMemBlock<double>::size_type(size));
 *	double d = double(0.34567864);
 *	for (CMemBlock<double>::size_type i = 0; i < 100; i++) {
 *		cmBlock[i] = d;
 *	}
 *
 *	Demo 5:
 *	void funcAdd(const CMemBlock<int> &in_cmBlock, CMemBlock<int> &out_cmBlock) {
 *		CMemBlock<int>::size_type count = in_cmBlock.GetSize(), i;
 *		out_cmBlock.Resize(count);
 *		int i_add = 10;
 *		for(i = 0; i < count; i++) {
 *			out_cmBlock[i] = in_cmBlock[i] + i_add;
 *		}
 *	}
 *
 *  CMemBlock<int> funcGet() {
 * 		int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
 * 		int *pArr = (int *) malloc(sizeof(arr));
 * 		memcpy(pArr, arr, sizeof(arr));
 * 		CMemBlock<int> cmBlock;
 * 		cmBlock.SetMem(pArr, sizeof(arr));
 * 		return cmBlock;
 *  }
 *	void main() {
 *		CMemBlock<int> cmBlock1 = funcGet(), cmBlock2;
 *		funcAdd(cmBlock1, cmBlock2);
 *		CMemBlock<int>::size_type count = cmBlock2.GetSize();
 *		int i_add = 10;
 *		for (CMemBlock<int>::size_type i = 0; i < count; i++) {
 *			cmBlock2[i] = cmBlock2[i] - i_add;
 *		}
 *	}
 */

template<class T, class SIZETYPE=size_t>
class CMemBlock {
public:
	typedef SIZETYPE size_type;
public:
	CMemBlock() {
		pValue = new Value((size_type) 0);
		pValue->AddRef();
	}

	CMemBlock(int size) {
		pValue = new Value(size_type(size));
		pValue->AddRef();
	}

	CMemBlock(size_type size) {
#ifdef USE_VARY_MACRO
		__glibcxx_requires_subscript(size);
#endif
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
		size_type l = (nullptr != pValue ? pValue->_len : 0) + (nullptr != mem.Pvalue ? mem.pValue->_len : 0);
		ret.Resize(l);
		memcpy(ret, nullptr != pValue ? pValue->data : 0, sizeof(T) * (nullptr != pValue ? pValue->_len : 0));
		memcpy(ret + (nullptr != pValue ? pValue->_len : 0), nullptr != mem.pValue ? mem.pValue->data : 0,
			   sizeof(T) * (nullptr != mem.pValue ? mem.pValue->_len : 0));

		return ret;
	}

	CMemBlock operator+(CMemBlock &mem) const {
		CMemBlock ret;
		size_type l = (nullptr != pValue ? pValue->_len : 0) + (nullptr != mem.Pvalue ? mem.pValue->_len : 0);
		ret.Resize(l);
		memcpy(ret, nullptr != pValue ? pValue->data : 0, sizeof(T) * (nullptr != pValue ? pValue->_len : 0));
		memcpy(ret + (nullptr != pValue ? pValue->_len : 0), nullptr != mem.pValue ? mem.pValue->data : 0,
			   sizeof(T) * (nullptr != mem.pValue ? mem.pValue->_len : 0));

		return ret;
	}

	CMemBlock &operator+=(const CMemBlock &mem) {
		if (nullptr == pValue) {
			pValue = new Value((size_type) 0);
			pValue->AddRef();
		}
		size_type l = pValue->_len + (nullptr != mem.pValue ? mem.pValue->_len : 0);
		pValue->Resize(l);
		memcpy(pValue->data + pValue->_len, nullptr != mem.pValue ? mem.pValue->data : 0,
			   sizeof(T) * (nullptr != mem.pValue ? mem.pValue->_len : 0));

		return *this;
	}

	CMemBlock &operator+=(CMemBlock &mem) {
		if (nullptr == pValue) {
			pValue = new Value((size_type) 0);
			pValue->AddRef();
		}
		size_type l = pValue->_len + (nullptr != mem.pValue ? mem.pValue->_len : 0);
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

	void DelAt(size_type st) {
		if (nullptr != pValue)
			pValue->DelAt(st);
	}

	size_type SetMem(T *pV, size_type len) {
		return nullptr != pValue ? pValue->SetMem(pV, len) : 0;
	}

	size_type SetMemFixed(const T *pV, size_type len) {
		return nullptr != pValue ? pValue->SetMemFixed(pV, len) : 0;
	}

	size_type Resize(int size) {
		return Resize(size_type(size));
	}

	size_type Resize(size_type size) {
		return nullptr != pValue ? pValue->Resize(size) : 0;
	}

	size_type push_back(T &t) {
		return nullptr != pValue ? pValue->push_back(t) : 0;
	}

	size_type GetSize() const {
		return nullptr != pValue ? pValue->GetSize() : 0;
	}

	size_type GetSize() {
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
		return nullptr != pValue ? *pValue->data : (*this)[0];
	}

	T &operator*() {
		return nullptr != pValue ? *pValue->data : (*this)[0];
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

	operator const T *() {
		return nullptr != pValue ? pValue->data : 0;
	}

	T **operator&() const {
		return nullptr != pValue ? &pValue->data : 0;
	}

	T **operator&() {
		return nullptr != pValue ? &pValue->data : 0;
	}

	T *operator+(int off) const {
		return operator+(size_type(off));
	}

	T *operator+(int off) {
		return operator+(size_type(off));
	}

	T *operator+(size_type off) const {
#ifdef USE_VARY_MACRO
		__glibcxx_requires_subscript(off);
#endif
		return nullptr != pValue ? pValue->data + off : 0;
	}

	T *operator+(size_type off) {
#ifdef USE_VARY_MACRO
		__glibcxx_requires_subscript(off);
#endif
		return nullptr != pValue ? pValue->data + off : 0;
	}

	T &operator[](int off) const {
		return operator[](size_type(off));
	}

	T &operator[](int off) {
		return operator[](size_type(off));
	}

	T &operator[](size_type off) const {
#ifdef USE_VARY_MACRO
		__glibcxx_requires_subscript(off);
#endif
		return nullptr != pValue->data ? pValue->data[off] : (*this)[0];
	}

	T &operator[](size_type off) {
#ifdef USE_VARY_MACRO
		__glibcxx_requires_subscript(off);
#endif
		return nullptr != pValue->data ? pValue->data[off] : (*this)[0];
	}

private:
	class Value {
		bool fixed;
	public:
		size_type AddRef() {
			__sync_fetch_and_add(&_ref, 1);
			return _ref;
		}

		size_type Release() {
			__sync_fetch_and_sub(&_ref, 1);
			if (0 == _ref) {
				delete this;
				return 0;
			} else {
				return _ref;
			}
		}

		Value(size_type size) {
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
				for (size_type l = 0; l < _len; l++) {
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

		void DelAt(size_type st) {
			if (nullptr != data && _len > 0) {
				if (0 <= st && st < _len) {
					T *p = (T *) malloc((_len - 1) * sizeof(T));
					for (size_type i = 0; i < _len; i++) {
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

		size_type SetMem(T *pV, size_type len) {
			if (nullptr != data && !fixed)
				free(data);
			data = pV;
			fixed = false;
			_len = len;
			return _len;
		}

		size_type SetMemFixed(const T *pV, size_type len) {
			if (nullptr != data && !fixed)
				free(data);
			data = const_cast<T *>(pV);
			fixed = true;
			_len = len;
			return _len;
		}

		size_type Resize(size_type size) {
			if (size == _len)
				return size;
			if (0 < size) {
				T *t = (T *) malloc(size * sizeof(T));
				if (nullptr != data) {
					size_type lt = _len > size ? size : _len;
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

		size_type push_back(T &t) {
			T *pt = (T *) malloc((_len + 1) * sizeof(T));
			memcpy(pt, data, _len * sizeof(T));
			if (nullptr != data && !fixed) free(data);
			data = pt;
			fixed = false;
			data[_len++] = t;
			return _len;
		}

		size_type GetSize() {
			return _len;
		}

		void Reverse() {
			if (nullptr != data && 0 < _len) {
				size_type id = _len / 2;
				T tmp;
				for (size_type i = 0; i < id; i++) {
					tmp = data[i];
					data[i] = data[_len - (i + 1)];
					data[_len - (i + 1)] = tmp;
				}
			}
		}

		size_type _ref;
		T *data;
		size_type _len;
	};

	Value *pValue;
};

typedef CMemBlock<uint8_t, size_t> CMBlock;


#endif
