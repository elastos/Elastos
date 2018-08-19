package sdk

import (
	"time"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/crypto"
	"github.com/elastos/Elastos.ELA.Utility/p2p"

	ela "github.com/elastos/Elastos.ELA/core"
)

const (
	ProtocolVersion = p2p.EIP001Version // The protocol version implemented SPV protocol
	OpenService     = 1 << 2
)

func GenesisHeader(foundation *common.Uint168) *ela.Header {
	// header
	header := ela.Header{
		Version:    ela.BlockVersion,
		Previous:   common.EmptyHash,
		MerkleRoot: common.EmptyHash,
		Timestamp:  uint32(time.Unix(time.Date(2017, time.December, 22, 10, 0, 0, 0, time.UTC).Unix(), 0).Unix()),
		Bits:       0x1d03ffff,
		Nonce:      ela.GenesisNonce,
		Height:     uint32(0),
	}

	// ELA coin
	elaCoin := &ela.Transaction{
		TxType:         ela.RegisterAsset,
		PayloadVersion: 0,
		Payload: &ela.PayloadRegisterAsset{
			Asset: ela.Asset{
				Name:      "ELA",
				Precision: 0x08,
				AssetType: 0x00,
			},
			Amount:     0 * 100000000,
			Controller: common.Uint168{},
		},
		Attributes: []*ela.Attribute{},
		Inputs:     []*ela.Input{},
		Outputs:    []*ela.Output{},
		Programs:   []*ela.Program{},
	}

	coinBase := &ela.Transaction{
		TxType:         ela.CoinBase,
		PayloadVersion: ela.PayloadCoinBaseVersion,
		Payload:        new(ela.PayloadCoinBase),
		Inputs: []*ela.Input{
			{
				Previous: ela.OutPoint{
					TxID:  common.EmptyHash,
					Index: 0x0000,
				},
				Sequence: 0x00000000,
			},
		},
		Attributes: []*ela.Attribute{},
		LockTime:   0,
		Programs:   []*ela.Program{},
	}

	coinBase.Outputs = []*ela.Output{
		{
			AssetID:     elaCoin.Hash(),
			Value:       3300 * 10000 * 100000000,
			ProgramHash: *foundation,
		},
	}

	nonce := []byte{0x4d, 0x65, 0x82, 0x21, 0x07, 0xfc, 0xfd, 0x52}
	txAttr := ela.NewAttribute(ela.Nonce, nonce)
	coinBase.Attributes = append(coinBase.Attributes, &txAttr)

	transactions := []*ela.Transaction{coinBase, elaCoin}
	hashes := make([]common.Uint256, 0, len(transactions))
	for _, tx := range transactions {
		hashes = append(hashes, tx.Hash())
	}
	header.MerkleRoot, _ = crypto.ComputeRoot(hashes)

	return &header
}
