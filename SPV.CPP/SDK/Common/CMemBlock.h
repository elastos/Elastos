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

template<class T>
class CMemBlock {
public:
	CMemBlock() {
		pValue = new Value(0);
		pValue->AddRef();
	}

	template <class size_type>
	CMemBlock(const void *buf, size_type len) {
		size_t l = size_t(len);
		pValue = new Value(l);
		memcpy(pValue->data, buf, l);
		pValue->AddRef();
	}

	template<class size_type>
	CMemBlock(size_type size) {
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
		size_t l = (nullptr != pValue ? pValue->_len : 0) + (nullptr != mem.Pvalue ? mem.pValue->_len : 0);
		ret.Resize(l);
		memcpy(ret, nullptr != pValue ? pValue->data : 0, sizeof(T) * (nullptr != pValue ? pValue->_len : 0));
		memcpy(ret + (nullptr != pValue ? pValue->_len : 0), nullptr != mem.pValue ? mem.pValue->data : 0,
			   sizeof(T) * (nullptr != mem.pValue ? mem.pValue->_len : 0));

		return ret;
	}

	CMemBlock operator+(CMemBlock &mem) const {
		CMemBlock ret;
		size_t l = (nullptr != pValue ? pValue->_len : 0) + (nullptr != mem.Pvalue ? mem.pValue->_len : 0);
		ret.Resize(l);
		memcpy(ret, nullptr != pValue ? pValue->data : 0, sizeof(T) * (nullptr != pValue ? pValue->_len : 0));
		memcpy(ret + (nullptr != pValue ? pValue->_len : 0), nullptr != mem.pValue ? mem.pValue->data : 0,
			   sizeof(T) * (nullptr != mem.pValue ? mem.pValue->_len : 0));

		return ret;
	}

	CMemBlock &operator+=(const CMemBlock &mem) {
		if (nullptr == pValue) {
			pValue = new Value(0);
			pValue->AddRef();
		}
		size_t l = pValue->_len + (nullptr != mem.pValue ? mem.pValue->_len : 0);
		pValue->Resize(l);
		memcpy(pValue->data + pValue->_len, nullptr != mem.pValue ? mem.pValue->data : 0,
			   sizeof(T) * (nullptr != mem.pValue ? mem.pValue->_len : 0));

		return *this;
	}

	CMemBlock &operator+=(CMemBlock &mem) {
		if (nullptr == pValue) {
			pValue = new Value(0);
			pValue->AddRef();
		}
		size_t l = pValue->_len + (nullptr != mem.pValue ? mem.pValue->_len : 0);
		pValue->Resize(l);
		memcpy(pValue->data + pValue->_len, nullptr != mem.pValue ? mem.pValue->data : 0,
			   sizeof(T) * (nullptr != mem.pValue ? mem.pValue->_len : 0));

		return *this;
	}

	bool operator==(const CMemBlock &mem) const {
		if (pValue == mem.pValue) {
			return true;
		}

		if (pValue != nullptr && mem.pValue != nullptr) {
			if (pValue->GetSize() == mem.pValue->GetSize()) {
				if (0 == memcmp(pValue->data, mem.pValue->data, pValue->GetSize())) {
					return true;
				}
			}
		}

		return false;
	}

	bool operator!=(const CMemBlock &mem) const {
		return !(*this == mem);
	}

	void Zero() {
		if (nullptr != pValue)
			pValue->Zero();
	}

	void Clear() {
		if (nullptr != pValue)
			pValue->Clear();
	}

	template <class size_type>
	void DelAt(size_type st) {
		if (nullptr != pValue)
			pValue->DelAt(st);
	}

	template <class size_type>
	size_t SetMem(T *pV, size_type len) {
		return nullptr != pValue ? pValue->SetMem(pV, len) : 0;
	}

	template <class Type, class size_type>
	size_t SetMemFixed(const Type *pV, size_type len) {
		return nullptr != pValue ? pValue->SetMemFixed((const T *)pV, len) : 0;
	}

	template <class size_type>
	size_t Resize(size_type size) {
		return nullptr != pValue ? pValue->Resize(size) : 0;
	}

	size_t push_back(T &t) {
		return nullptr != pValue ? pValue->push_back(t) : 0;
	}

	size_t GetSize() const {
		return nullptr != pValue ? pValue->GetSize() : 0;
	}

	size_t GetSize() {
		return nullptr != pValue ? pValue->GetSize() : 0;
	}

	void Reverse() {
		nullptr != pValue ? pValue->Reverse() : 0;
	}

	void Memcpy(const CMemBlock &src) {
		Resize(src.GetSize());
		memcpy(pValue->data, src.pValue->data, src.GetSize());
	}

	operator bool() const {
		return nullptr != pValue ? pValue->data ? true : false : false;
	};

	operator bool() {
		return nullptr != pValue ? pValue->data ? true : false : false;
	};

	T &operator*() const {
		return nullptr != pValue ? *pValue->data : (*this)[0];
	}

	T &operator*() {
		return nullptr != pValue ? *pValue->data : (*this)[0];
	}

	template <class Type>
	operator Type *() const {
		return (Type *) (nullptr != pValue ? pValue->data : 0);
	}

	template <class Type>
	operator Type *() {
		return (Type *) (nullptr != pValue ? pValue->data : 0);
	}

	template <class Type>
	operator const Type *() const {
		return (const Type *) (nullptr != pValue ? pValue->data : 0);
	}

	template <class Type>
	operator const Type *() {
		return (const Type *) (nullptr != pValue ? pValue->data : 0);
	}

	T **operator&() const {
		return nullptr != pValue ? &pValue->data : 0;
	}

	T **operator&() {
		return nullptr != pValue ? &pValue->data : 0;
	}

	T *operator+(size_t off) const {
		return nullptr != pValue ? pValue->data + off : 0;
	}

	T *operator+(size_t off) {
		return nullptr != pValue ? pValue->data + off : 0;
	}

	template <class size_type>
	T &operator[](size_type off) const {
		return nullptr != pValue->data ? pValue->data[off] : (*this)[0];
	}

	template <class size_type>
	T &operator[](size_type off) {
		return nullptr != pValue->data ? pValue->data[off] : (*this)[0];
	}

private:
	class Value {
		bool fixed;
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

		template <class size_type>
		Value(size_type size) {
			size_t s = size_t(size);
			fixed = false;

			_ref = 0;
			if (0 < s) {
				data = (T *) malloc(s * sizeof(T));
				_len = s;
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
				for (size_t l = 0; l < _len; l++) {
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

		template <class size_type>
		void DelAt(size_type st) {
			size_t at = size_t(st);
			if (nullptr != data && _len > 0) {
				if (0 <= at && at < _len) {
					T *p = (T *) malloc((_len - 1) * sizeof(T));
					for (size_t i = 0; i < _len; i++) {
						if (i < at) {
							p[i] = data[i];
						} else if (i > at) {
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

		template <class size_type>
		size_t SetMem(T *pV, size_type len) {
			if (nullptr != data && !fixed)
				free(data);
			data = pV;
			fixed = false;
			_len = size_t(len);
			return _len;
		}

		template <class size_type>
		size_t SetMemFixed(const T *pV, size_type len) {
			if (nullptr != data && !fixed)
				free(data);
			data = const_cast<T *>(pV);
			fixed = true;
			_len = size_t(len);
			return _len;
		}

		template <class size_type>
		size_t Resize(size_type size) {
			size_t s = size_t(size);
			if (s == _len)
				return s;
			if (0 < s) {
				T *t = (T *) malloc(s * sizeof(T));
				if (nullptr != data) {
					size_t lt = _len > s ? s : _len;
					memcpy(t, data, lt * sizeof(T));
					if (!fixed) free(data);
				}
				data = t;
				fixed = false;
				_len = s;
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

		size_t GetSize() {
			return _len;
		}

		void Reverse() {
			if (nullptr != data && 0 < _len) {
				size_t id = _len / 2;
				T tmp;
				for (size_t i = 0; i < id; i++) {
					tmp = data[i];
					data[i] = data[_len - (i + 1)];
					data[_len - (i + 1)] = tmp;
				}
			}
		}

		size_t _ref;
		T *data;
		size_t _len;
	};

	Value *pValue;
};

typedef CMemBlock<uint8_t> CMBlock;


#endif
