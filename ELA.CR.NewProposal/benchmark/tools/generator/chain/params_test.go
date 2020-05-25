// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package chain

import (
	"bytes"
	"math/rand"
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestGenerationParams_Deserialize(t *testing.T) {
	params := randomParams()

	buf := new(bytes.Buffer)
	assert.NoError(t, params.Serialize(buf))

	var params2 GenerationParams
	assert.NoError(t, params2.Deserialize(buf))

	assert.True(t, paramsEqual(params, &params2))
}

func randomParams() *GenerationParams {
	return &GenerationParams{
		Mode:               GenerationMod(rand.Intn(int(Minimal))),
		PrepareStartHeight: rand.Uint32(),
		RandomStartHeight:  rand.Uint32(),
		InputsPerBlock:     rand.Uint32(),
		MaxRefersCount:     rand.Uint32(),
		MinRefersCount:     rand.Uint32(),
		AddressCount:       rand.Uint32(),
	}
}

func paramsEqual(params, params2 *GenerationParams) bool {
	return params.Mode == params2.Mode &&
		params.PrepareStartHeight == params2.PrepareStartHeight &&
		params.RandomStartHeight == params2.RandomStartHeight &&
		params.InputsPerBlock == params2.InputsPerBlock &&
		params.MaxRefersCount == params2.MaxRefersCount &&
		params.MinRefersCount == params2.MinRefersCount &&
		params.AddressCount == params2.AddressCount
}
