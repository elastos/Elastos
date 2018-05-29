package spv

import (
	"bytes"
	"crypto/rand"
	"encoding/binary"
	"errors"
	"os"

	"github.com/elastos/Elastos.ELA.SideChain/config"
	"github.com/elastos/Elastos.ELA.SideChain/log"

	"github.com/elastos/Elastos.ELA.SPV/interface"
	spvlog "github.com/elastos/Elastos.ELA.SPV/log"
	. "github.com/elastos/Elastos.ELA/bloom"
	ela "github.com/elastos/Elastos.ELA/core"
)

var spvService _interface.SPVService
var maxConnections = 12

func SpvInit() error {
	var err error
	spvlog.Init(config.Parameters.SpvPrintLevel)

	var id = make([]byte, 8)
	var clientId uint64
	rand.Read(id)
	binary.Read(bytes.NewReader(id), binary.LittleEndian, clientId)

	spvService, err = _interface.NewSPVService(config.Parameters.SpvMagic, clientId,
		config.Parameters.SpvSeedList, config.Parameters.SpvMinOutbound, config.Parameters.SpvMaxConnections)
	if err != nil {
		return err
	}

	go func() {
		if err := spvService.Start(); err != nil {
			log.Info("Spv service start failed ：", err)
		}
		log.Info("Spv service stoped")
		os.Exit(-1)
	}()
	return nil
}

func VerifyTransaction(tx *ela.Transaction) error {
	proof := new(MerkleProof)
	mainChainTransaction := new(ela.Transaction)

	switch object := tx.Payload.(type) {
	case *ela.PayloadRechargeToSideChain:
		reader := bytes.NewReader(object.MerkleProof)
		if err := proof.Deserialize(reader); err != nil {
			return errors.New("RechargeToSideChain payload deserialize failed")
		}
		reader = bytes.NewReader(object.MainChainTransaction)
		if err := mainChainTransaction.Deserialize(reader); err != nil {
			return errors.New("RechargeToSideChain mainChainTransaction deserialize failed")
		}
	default:
		return errors.New("Invalid payload")
	}

	if err := spvService.VerifyTransaction(*proof, *mainChainTransaction); err != nil {
		return errors.New("SPV module verify transaction failed.")
	}

	return nil
}
