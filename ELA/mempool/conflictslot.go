// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package mempool

import (
	"fmt"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/errors"
)

// keyType defines the conflictSlot supported key types.
type keyType byte

// getKeyFunc defines the general function about get key from a tx.
type getKeyFunc func(*types.Transaction) (interface{}, error)

// keyTypeFuncPair defines a pair about tx type and related getKeyFunc.
type keyTypeFuncPair struct {
	Type types.TxType
	Func getKeyFunc
}

// self-defined tx type indicates all types of txs.
const allType types.TxType = 0xff

const (
	// str keyType will treat key as type of string.
	str keyType = iota

	// hash keyType will treat key as type of Uint256.
	hash

	// programHash keyType will treat key as type of Uint168.
	programHash

	// hash keyType will treat key as type of string array.
	strArray

	// hash keyType will treat key as type of Uint256 array.
	hashArray

	// programHash keyType will treat key as type of Uint168 array.
	programHashArray
)

// conflictSlot hold a set of transactions references that may conflict with
//	incoming transactions, those transactions will process with same rule to
//	generate key by which to detect the conflict.
type conflictSlot struct {
	keyType        keyType
	conflictTypes  map[types.TxType]getKeyFunc
	stringSet      map[string]*types.Transaction
	hashSet        map[common.Uint256]*types.Transaction
	programHashSet map[common.Uint168]*types.Transaction
}

func (s *conflictSlot) Empty() bool {
	return len(s.stringSet) == 0 && len(s.hashSet) == 0 &&
		len(s.programHashSet) == 0
}

func (s *conflictSlot) Contains(key interface{}) (ok bool) {
	switch k := key.(type) {
	case string:
		_, ok = s.stringSet[k]
	case common.Uint256:
		_, ok = s.hashSet[k]
	case common.Uint168:
		_, ok = s.programHashSet[k]
	}
	return
}

func (s *conflictSlot) GetTx(key interface{}) (tx *types.Transaction) {
	switch k := key.(type) {
	case string:
		tx = s.stringSet[k]
	case common.Uint256:
		tx = s.hashSet[k]
	case common.Uint168:
		tx = s.programHashSet[k]
	}
	return
}

func (s *conflictSlot) VerifyTx(tx *types.Transaction) errors.ELAError {
	getKey := s.getKeyFromTx(tx)
	if getKey == nil {
		return nil
	}

	key, err := getKey(tx)
	if err != nil {
		return errors.SimpleWithMessage(errors.ErrTxPoolFailure, err,
			"error occurred when get key from tx")
	}

	return s.txProcess(key, s.keyType,
		func(key string) errors.ELAError {
			if _, ok := s.stringSet[key]; ok {
				return errors.SimpleWithMessage(errors.ErrTxPoolTxDuplicate,
					nil, fmt.Sprintf(
						"string key %s alread exist in tx pool", key))
			}
			return nil
		}, func(key common.Uint256) errors.ELAError {
			if _, ok := s.hashSet[key]; ok {
				return errors.SimpleWithMessage(errors.ErrTxPoolTxDuplicate,
					nil, fmt.Sprintf(
						"hash key %s alread exist in tx pool",
						key.String()))
			}
			return nil
		}, func(key common.Uint168) errors.ELAError {
			if _, ok := s.programHashSet[key]; ok {
				addr, err := key.ToAddress()
				if err != nil {
					return errors.SimpleWithMessage(errors.ErrTxPoolFailure,
						err, "program hash convert to address error")
				}
				return errors.SimpleWithMessage(errors.ErrTxPoolTxDuplicate,
					nil, fmt.Sprintf(
						"address %s alread exist in tx pool", addr))
			}
			return nil
		})
}

func (s *conflictSlot) AppendTx(tx *types.Transaction) errors.ELAError {
	getKey := s.getKeyFromTx(tx)
	if getKey == nil {
		return nil
	}

	key, err := getKey(tx)
	if err != nil {
		return errors.SimpleWithMessage(errors.ErrTxPoolFailure, err,
			"error occurred when get key from tx")
	}
	return s.appendKey(key, tx)
}

func (s *conflictSlot) getKeyFromTx(tx *types.Transaction) (getKey getKeyFunc) {
	var exist bool
	getKey, exist = s.conflictTypes[tx.TxType]
	if !exist {
		getKey = s.conflictTypes[allType]
	}
	return getKey
}

func (s *conflictSlot) appendKey(key interface{},
	tx *types.Transaction) errors.ELAError {
	return s.txProcess(key, s.keyType,
		func(key string) errors.ELAError {
			s.stringSet[key] = tx
			return nil
		}, func(key common.Uint256) errors.ELAError {
			s.hashSet[key] = tx
			return nil
		}, func(key common.Uint168) errors.ELAError {
			s.programHashSet[key] = tx
			return nil
		},
	)
}

func (s *conflictSlot) RemoveTx(tx *types.Transaction) errors.ELAError {
	getKey := s.getKeyFromTx(tx)
	if getKey == nil {
		return nil
	}

	key, err := getKey(tx)
	if err != nil {
		return errors.SimpleWithMessage(errors.ErrTxPoolFailure, err,
			"error occurred when get key from tx")
	}
	return s.removeKey(key)
}

func (s *conflictSlot) removeKey(key interface{}) errors.ELAError {
	return s.txProcess(key, s.keyType,
		func(key string) errors.ELAError {
			delete(s.stringSet, key)
			return nil
		}, func(key common.Uint256) errors.ELAError {
			delete(s.hashSet, key)
			return nil
		}, func(key common.Uint168) errors.ELAError {
			delete(s.programHashSet, key)
			return nil
		},
	)
}

func (s *conflictSlot) txProcess(key interface{}, t keyType,
	processStrKey func(string) errors.ELAError,
	processHashKey func(common.Uint256) errors.ELAError,
	processProgramHashKey func(common.Uint168) errors.ELAError) errors.ELAError {

	switch t {
	case str:
		str, ok := key.(string)
		if !ok {
			return errors.SimpleWithMessage(errors.ErrTxPoolTypeCastFailure,
				nil, "tx key type cast error")
		}
		return processStrKey(str)
	case hash:
		hash, ok := key.(common.Uint256)
		if !ok {
			return errors.SimpleWithMessage(errors.ErrTxPoolTypeCastFailure,
				nil, "tx key type cast error")
		}
		return processHashKey(hash)
	case programHash:
		addr, ok := key.(common.Uint168)
		if !ok {
			return errors.SimpleWithMessage(errors.ErrTxPoolTypeCastFailure,
				nil, "tx key type cast error")
		}
		return processProgramHashKey(addr)
	case strArray:
		strArray, ok := key.([]string)
		if !ok {
			return errors.SimpleWithMessage(errors.ErrTxPoolTypeCastFailure,
				nil, "tx key type cast error")
		}
		for _, v := range strArray {
			if err := s.txProcess(v, str, processStrKey, processHashKey,
				processProgramHashKey); err != nil {
				return err
			}
		}
		return nil
	case hashArray:
		hashArray, ok := key.([]common.Uint256)
		if !ok {
			return errors.SimpleWithMessage(errors.ErrTxPoolTypeCastFailure,
				nil, "tx key type cast error")
		}
		for _, v := range hashArray {
			if err := s.txProcess(v, hash, processStrKey, processHashKey,
				processProgramHashKey); err != nil {
				return err
			}
		}
		return nil
	case programHashArray:
		programHashArray, ok := key.([]common.Uint168)
		if !ok {
			return errors.SimpleWithMessage(errors.ErrTxPoolTypeCastFailure,
				nil, "tx key type cast error")
		}
		for _, v := range programHashArray {
			if err := s.txProcess(v, programHash, processStrKey, processHashKey,
				processProgramHashKey); err != nil {
				return err
			}
		}
		return nil
	default:
		return errors.SimpleWithMessage(errors.ErrTxPoolTypeCastFailure,
			nil, "unknown key type")
	}
}

func newConflictSlot(t keyType, conflictTypes ...keyTypeFuncPair) *conflictSlot {
	ts := make(map[types.TxType]getKeyFunc)
	for _, v := range conflictTypes {
		ts[v.Type] = v.Func
	}

	return &conflictSlot{
		keyType:        t,
		conflictTypes:  ts,
		stringSet:      map[string]*types.Transaction{},
		hashSet:        map[common.Uint256]*types.Transaction{},
		programHashSet: map[common.Uint168]*types.Transaction{},
	}
}
