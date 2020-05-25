// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package wallet

import (
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/utils/test"

	"github.com/stretchr/testify/assert"
)

const (
	// pubkey1  = "03e630e917b0cfd076478780dcbfed89bc6db71f2865c2c124c6f95a4e3b9b307b"
	address1 = "Eca2gTGQTaqSBEx1cAVztkKtUHUZ2Hp8Rs"

	pubkey2  = "033b4606d3cec58a01a09da325f5849754909fec030e4cf626e6b4104328599fc7"
	address2 = "EgJd7g6irnhjXcEtxyHhEwpG98B9CumBHA"
	code2    = "21033b4606d3cec58a01a09da325f5849754909fec030e4cf626e6b4104328599fc7ac"
)

var wallet *Wallet

func TestWallet_New(t *testing.T) {
	log.NewDefault(test.NodeLogPath, 0, 0, 0)
	ChainParam = &config.DefaultParams

	wallet = New(test.DataDir)

	version, err := wallet.LoadStoredData("Version")
	assert.NoError(t, err)
	assert.Equal(t, "1.0.0", string(version))
}

func TestWallet_ImportAddress(t *testing.T) {
	err := wallet.ImportAddress(address1, true)
	assert.NoError(t, err)

	err = wallet.LoadAddresses()
	assert.NoError(t, err)

	addressInfo, ok := addressBook[address1]
	assert.Equal(t, true, ok)
	assert.Equal(t, address1, addressInfo.address)
	assert.Equal(t, "", string(addressInfo.code))

	err = wallet.DeleteAccountData(address1)
	assert.NoError(t, err)
}

func TestWallet_ImportPubkey(t *testing.T) {
	pubkeyBytes, err := common.HexStringToBytes(pubkey2)
	assert.NoError(t, err)

	err = wallet.ImportPubkey(pubkeyBytes, true)
	assert.NoError(t, err)

	err = wallet.LoadAddresses()
	assert.NoError(t, err)

	addressInfo, ok := addressBook[address2]
	assert.Equal(t, true, ok)
	assert.Equal(t, address2, addressInfo.address)

	code2Bytes, err := common.HexStringToBytes(code2)
	assert.NoError(t, err)
	assert.Equal(t, code2Bytes, addressInfo.code)

	err = wallet.DeleteAccountData(address2)
	assert.NoError(t, err)
}
