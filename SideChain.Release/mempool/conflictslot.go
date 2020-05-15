// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package mempool

import (
	"errors"
	"fmt"

	"github.com/elastos/Elastos.ELA/common"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/types"
)

// KeyType defines the conflictSlot supported key types.
type KeyType byte

// GetKeyFunc defines the general function about get key from a tx.
type GetKeyFunc func(*blockchain.BlockChain, *types.Transaction) (interface{}, error)

// KeyTypeFuncPair defines a pair about tx type and related GetKeyFunc.
type KeyTypeFuncPair struct {
	Type types.TxType
	Func GetKeyFunc
}

// self-defined tx type indicates all types of txs.
const allType types.TxType = 0xff

const (
	// Str KeyType will treat key as type of string.
	Str KeyType = iota

	// Hash KeyType will treat key as type of Uint256.
	Hash

	// ProgramHash KeyType will treat key as type of Uint168.
	ProgramHash

	// Hash KeyType will treat key as type of string array.
	StrArray

	// Hash KeyType will treat key as type of Uint256 array.
	HashArray

	// ProgramHash KeyType will treat key as type of Uint168 array.
	ProgramHashArray
)

// conflictSlot hold a set of transactions references that may conflict with
//	incoming transactions, those transactions will process with same rule to
//	generate key by which to detect the conflict.
type conflictSlot struct {
	keyType        KeyType
	conflictTypes  map[types.TxType]GetKeyFunc
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

func (s *conflictSlot) VerifyTx(chain *blockchain.BlockChain, tx *types.Transaction) error {
	getKey := s.getKeyFromTx(tx)
	if getKey == nil {
		return nil
	}

	key, err := getKey(chain, tx)
	if err != nil {
		return errors.New("error occurred when get key from tx")
	}

	return s.txProcess(key, s.keyType,
		func(key string) error {
			if _, ok := s.stringSet[key]; ok {
				return errors.New(fmt.Sprintf(
					"string key %s alread exist in tx pool", key))
			}
			return nil
		}, func(key common.Uint256) error {
			if _, ok := s.hashSet[key]; ok {
				return errors.New(fmt.Sprintf(
					"Hash key %s alread exist in tx pool",
					key.String()))
			}
			return nil
		}, func(key common.Uint168) error {
			if _, ok := s.programHashSet[key]; ok {
				addr, err := key.ToAddress()
				if err != nil {
					return errors.New("program Hash convert to address error")
				}
				return errors.New(fmt.Sprintf(
					"address %s alread exist in tx pool", addr))
			}
			return nil
		})
}

func (s *conflictSlot) AppendTx(chain *blockchain.BlockChain, tx *types.Transaction) error {
	getKey := s.getKeyFromTx(tx)
	if getKey == nil {
		return nil
	}

	key, err := getKey(chain, tx)
	if err != nil {
		return errors.New("error occurred when get key from tx")
	}
	return s.appendKey(key, tx)
}

func (s *conflictSlot) getKeyFromTx(tx *types.Transaction) (getKey GetKeyFunc) {
	var exist bool
	getKey, exist = s.conflictTypes[tx.TxType]
	if !exist {
		getKey = s.conflictTypes[allType]
	}
	return getKey
}

func (s *conflictSlot) appendKey(key interface{},
	tx *types.Transaction) error {
	return s.txProcess(key, s.keyType,
		func(key string) error {
			s.stringSet[key] = tx
			return nil
		}, func(key common.Uint256) error {
			s.hashSet[key] = tx
			return nil
		}, func(key common.Uint168) error {
			s.programHashSet[key] = tx
			return nil
		},
	)
}

func (s *conflictSlot) RemoveTx(chain *blockchain.BlockChain, tx *types.Transaction) error {
	getKey := s.getKeyFromTx(tx)
	if getKey == nil {
		return nil
	}

	key, err := getKey(chain, tx)
	if err != nil {
		return errors.New("error occurred when get key from tx")
	}
	return s.removeKey(key)
}

func (s *conflictSlot) removeKey(key interface{}) error {
	return s.txProcess(key, s.keyType,
		func(key string) error {
			delete(s.stringSet, key)
			return nil
		}, func(key common.Uint256) error {
			delete(s.hashSet, key)
			return nil
		}, func(key common.Uint168) error {
			delete(s.programHashSet, key)
			return nil
		},
	)
}

func (s *conflictSlot) txProcess(key interface{}, t KeyType,
	processStrKey func(string) error,
	processHashKey func(common.Uint256) error,
	processProgramHashKey func(common.Uint168) error) error {

	switch t {
	case Str:
		str, ok := key.(string)
		if !ok {
			return errors.New("tx key type cast error")
		}
		return processStrKey(str)
	case Hash:
		hash, ok := key.(common.Uint256)
		if !ok {
			return errors.New("tx key type cast error")
		}
		return processHashKey(hash)
	case ProgramHash:
		addr, ok := key.(common.Uint168)
		if !ok {
			return errors.New("tx key type cast error")
		}
		return processProgramHashKey(addr)
	case StrArray:
		strArray, ok := key.([]string)
		if !ok {
			return errors.New("tx key type cast error")
		}
		for _, v := range strArray {
			if err := s.txProcess(v, Str, processStrKey, processHashKey,
				processProgramHashKey); err != nil {
				return err
			}
		}
		return nil
	case HashArray:
		hashArray, ok := key.([]common.Uint256)
		if !ok {
			return errors.New("tx key type cast error")
		}
		for _, v := range hashArray {
			if err := s.txProcess(v, Hash, processStrKey, processHashKey,
				processProgramHashKey); err != nil {
				return err
			}
		}
		return nil
	case ProgramHashArray:
		programHashArray, ok := key.([]common.Uint168)
		if !ok {
			return errors.New("tx key type cast error")
		}
		for _, v := range programHashArray {
			if err := s.txProcess(v, ProgramHash, processStrKey, processHashKey,
				processProgramHashKey); err != nil {
				return err
			}
		}
		return nil
	default:
		return errors.New("unknown key type")
	}
}

func NewConflictSlot(t KeyType, conflictTypes ...KeyTypeFuncPair) *conflictSlot {
	ts := make(map[types.TxType]GetKeyFunc)
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
