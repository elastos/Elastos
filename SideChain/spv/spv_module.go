package spv

import (
	"bytes"
	"errors"
	"math/rand"

	"github.com/elastos/Elastos.ELA.SideChain/config"

	"github.com/elastos/Elastos.ELA.SPV/interface"
	spvlog "github.com/elastos/Elastos.ELA.SPV/log"
	"github.com/elastos/Elastos.ELA.SideChain/log"
	. "github.com/elastos/Elastos.ELA/bloom"
	ela "github.com/elastos/Elastos.ELA/core"
)

var spvService _interface.SPVService

func SpvInit() error {
	spvlog.Init()
	spvService, err := _interface.NewSPVService(config.Parameters.Magic, uint64(rand.Int63()), config.Parameters.SpvSeedList)
	if err != nil {
		return err
	}
	go func() {
		if err := spvService.Start(); err != nil {
			log.Info("spvService start failed ：", err)
		}
	}()
	return nil
}

func VerifyTransaction(tx *ela.Transaction) error {
	proof := new(MerkleProof)
	mainChainTransaction := new(ela.Transaction)

	switch object := tx.Payload.(type) {
	case *ela.PayloadIssueToken:
		reader := bytes.NewReader(object.MerkleProof)
		if err := proof.Deserialize(reader); err != nil {
			return errors.New("IssueToken payload deserialize failed")
		}
		reader = bytes.NewReader(object.MainChainTransaction)
		if err := mainChainTransaction.Deserialize(reader); err != nil {
			return errors.New("IssueToken mainChainTransaction deserialize failed")
		}
	default:
		return errors.New("Invalid payload")
	}

	if err := spvService.VerifyTransaction(*proof, *mainChainTransaction); err != nil {
		return errors.New("SPV module verify transaction failed.")
	}

	return nil
}
