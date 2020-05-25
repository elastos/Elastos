// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package blockchain

import (
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/stretchr/testify/assert"
)

func TestCheckAssetPrecision(t *testing.T) {
	tx := buildTx()
	// valid precision
	for _, output := range tx.Outputs {
		output.AssetID = config.ELAAssetID
		output.ProgramHash = common.Uint168{}
		output.Value = 123456789876
	}
	err := checkAssetPrecision(tx)
	assert.NoError(t, err)

	for _, output := range tx.Outputs {
		output.AssetID = config.ELAAssetID
		output.ProgramHash = common.Uint168{}
		output.Value = 0
	}
	err = checkAssetPrecision(tx)
	assert.NoError(t, err)
}
