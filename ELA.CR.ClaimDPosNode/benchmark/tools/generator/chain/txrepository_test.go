// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package chain

import (
	"bytes"
	"math"
	"math/rand"
	"testing"

	"github.com/elastos/Elastos.ELA/account"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"

	"github.com/stretchr/testify/assert"
)

func TestTxRepository_Deserialize(t *testing.T) {
	repo1 := randomTxRepository()

	buf := new(bytes.Buffer)
	assert.NoError(t, repo1.Serialize(buf))

	repo2 := &TxRepository{}
	assert.NoError(t, repo2.Deserialize(buf))

	assert.True(t, txRepositoriesEqual(repo1, repo2))
}

func randomTxRepository() *TxRepository {
	result := &TxRepository{
		params:         randomParams(),
		foundationUTXO: randomUTXO(),
		foundation:     randomAccount(),
		accounts:       map[common.Uint168]*account.Account{},
		utxos:          map[common.Uint168][]types.UTXO{},
	}

	for i := 0; i < 5; i++ {
		key := randomUint168()
		result.accountKeys = append(result.accountKeys, *key)
		result.accounts[*key] = randomAccount()

		utxos := make([]types.UTXO, 0, 5)
		for j := 0; j < 5; j++ {
			utxos = append(utxos, randomUTXO())
		}
		result.utxos[*key] = utxos
	}
	return result
}

func txRepositoriesEqual(first, second *TxRepository) bool {
	if !paramsEqual(first.params, second.params) ||
		!utxosEqual(&first.foundationUTXO, &second.foundationUTXO) ||
		!accountsEqual(first.foundation, second.foundation) {
		return false
	}

	for i := range first.accountKeys {
		if !first.accountKeys[i].IsEqual(second.accountKeys[i]) {
			return false
		}
	}

	for k, v := range first.accounts {
		if v2, ok := second.accounts[k]; ok {
			if !accountsEqual(v, v2) {
				return false
			}
		} else {
			return false
		}
	}

	for k, v := range first.utxos {
		if v2, ok := second.utxos[k]; ok {
			for i := range v {
				if !utxosEqual(&v[i], &v2[i]) {
					return false
				}
			}
		} else {
			return false
		}
	}
	return true
}

func utxosEqual(first, second *types.UTXO) bool {
	return first.Index == second.Index && first.Value == second.Value &&
		first.TxID.IsEqual(second.TxID)
}

func accountsEqual(first, second *account.Account) bool {
	return bytes.Equal(first.PrivateKey, second.PrivateKey)
}

func randomAccount() *account.Account {
	ac, _ := account.NewAccount()
	return ac
}

func randomUint168() *common.Uint168 {
	randBytes := make([]byte, 21)
	rand.Read(randBytes)
	result, _ := common.Uint168FromBytes(randBytes)

	return result
}

func randomUint256() *common.Uint256 {
	randBytes := make([]byte, 32)
	rand.Read(randBytes)

	result, _ := common.Uint256FromBytes(randBytes)
	return result
}

func randomUTXO() types.UTXO {
	return types.UTXO{
		TxID:  *randomUint256(),
		Index: uint16(rand.Int31n(math.MaxUint16)),
		Value: common.Fixed64(rand.Int63()),
	}
}
