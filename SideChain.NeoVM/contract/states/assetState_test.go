package states

import (
	"testing"
	"math/big"
	"bytes"

	"github.com/stretchr/testify/assert"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"

	"github.com/elastos/Elastos.ELA.SideChain/types"
)

func TestAssetState(t *testing.T) {
	assetState := AssetState{}
	assetState.StateVersion = 2
	assetState.AssetId = common.Uint256{3, 3, 1}
	assetState.AssetType = types.Share
	assetState.Name = "name"
	assetState.Amount = 3
	assetState.Avaliable = 4
	assetState.Precision = 3
	assetState.FeeMode = 1
	assetState.Fee = 234
	assetState.FeeAddress = common.Uint168{1, 2, 3}
	pubKey := crypto.PublicKey{X: big.NewInt(1), Y: big.NewInt(2)}
	assetState.Owner = &pubKey
	assetState.Admin = common.Uint168{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2}
	assetState.Issuer = common.Uint168{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 1}
	assetState.Expiration = 233
	assetState.IsFrozen = true

	b := new(bytes.Buffer)
	err := assetState.Serialize(b)
	assert.True(t, err == nil)

	asssetState2 := AssetState{}
	err = asssetState2.Deserialize(b)
	assert.True(t, err == nil)
	assert.True(t, assetState.StateVersion == asssetState2.StateVersion)
	assert.True(t, assetState.AssetId == asssetState2.AssetId)
	assert.True(t, assetState.AssetType == asssetState2.AssetType)
	assert.True(t, assetState.Name == asssetState2.Name)
	assert.True(t, assetState.Amount == asssetState2.Amount)
	assert.True(t, assetState.Avaliable == asssetState2.Avaliable)
	assert.True(t, assetState.Precision == asssetState2.Precision)
	assert.True(t, assetState.FeeMode == asssetState2.FeeMode)
	assert.True(t, assetState.Fee == asssetState2.Fee)
	assert.True(t, assetState.FeeAddress == asssetState2.FeeAddress)
	assert.True(t, crypto.Equal(assetState.Owner, asssetState2.Owner))
	assert.True(t, assetState.Admin.IsEqual(asssetState2.Admin))
	assert.True(t, assetState.Issuer.IsEqual(asssetState2.Issuer))
	assert.True(t, assetState.Expiration == asssetState2.Expiration)
	assert.True(t, assetState.IsFrozen == asssetState2.IsFrozen)
}
