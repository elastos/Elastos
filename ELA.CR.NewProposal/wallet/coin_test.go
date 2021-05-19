// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package wallet

import (
	"bytes"
	"testing"

	"github.com/elastos/Elastos.ELA/account"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/stretchr/testify/assert"
)

func TestCoin_Serialize_Deserialize(t *testing.T) {
	programHash, err := common.Uint168FromAddress("EYGv9wNyEMtVHAkGJvdkFLb7FJneRWdbEu")
	assert.NoError(t, err)
	output := &types.Output{
		AssetID:     *account.SystemAssetID,
		Value:       common.Fixed64(100),
		OutputLock:  10,
		ProgramHash: *programHash,
		Type:        types.OTVote,
		Payload:     &outputpayload.VoteOutput{},
	}

	coin := Coin{
		TxVersion: 0x09,
		Output:    output,
		Height:    100,
	}

	buf := new(bytes.Buffer)
	err = coin.Serialize(buf)
	assert.NoError(t, err)
	newCoin := new(Coin)
	err = newCoin.Deserialize(buf)
	assert.NoError(t, err)

	assert.Equal(t, 9, int(newCoin.TxVersion))
	assert.Equal(t, output, newCoin.Output)
	assert.Equal(t, uint32(100), newCoin.Height)
}
