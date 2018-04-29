package spv

import (
	"bytes"
	"errors"
	"math/rand"

	"github.com/elastos/Elastos.ELA.SideChain/config"

	"github.com/elastos/Elastos.ELA.SPV/interface"
	. "github.com/elastos/Elastos.ELA/bloom"
	ela "github.com/elastos/Elastos.ELA/core"
)

var spvService *_interface.SPVServiceImpl

func SpvInit() error {
	spvService := _interface.NewSPVService(uint64(rand.Int63()), config.Parameters.SpvSeedList)
	spvService.Start()
	return nil
}

func VerifyTransaction(tx *ela.Transaction) error {
	proof := new(MerkleProof)

	switch object := tx.Payload.(type) {
	case *ela.PayloadIssueToken:
		buf := new(bytes.Buffer)
		if err := object.Deserialize(buf, ela.IssueTokenPayloadVersion); err != nil {
			return errors.New("IssueToken payload serialize failed")
		}
		if err := proof.Deserialize(buf); err != nil {
			return errors.New("IssueToken payload deserialize failed")
		}
	default:
		return errors.New("Invalid payload")
	}

	if err := spvService.VerifyTransaction(*proof, *tx); err != nil {
		return errors.New("SPV module verify transaction failed.")
	}

	return nil
}
