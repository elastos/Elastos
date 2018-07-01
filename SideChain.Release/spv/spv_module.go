package spv

import (
	"bytes"
	"crypto/rand"
	"encoding/binary"
	"errors"
	"os"

	"github.com/elastos/Elastos.ELA.SideChain/config"
	"github.com/elastos/Elastos.ELA.SideChain/core"
	"github.com/elastos/Elastos.ELA.SideChain/log"

	spv "github.com/elastos/Elastos.ELA.SPV/interface"
	spvlog "github.com/elastos/Elastos.ELA.SPV/log"
	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA/bloom"
	. "github.com/elastos/Elastos.ELA/bloom"
	ela "github.com/elastos/Elastos.ELA/core"
)

var spvService spv.SPVService

func SpvInit() error {
	var err error
	spvlog.Init(config.Parameters.SpvPrintLevel, 20, 1024)

	var id = make([]byte, 8)
	var clientId uint64
	rand.Read(id)
	binary.Read(bytes.NewReader(id), binary.LittleEndian, &clientId)

	spvService, err = spv.NewSPVService(config.Parameters.SpvMagic, config.Parameters.MainChainFoundationAddress, clientId,
		config.Parameters.SpvSeedList, config.Parameters.SpvMinOutbound, config.Parameters.SpvMaxConnections)
	//register an invalid address to prevent bloom filter from sending all data
	spvService.RegisterTransactionListener(&SpvListener{ListenAddress: "0000000000000000000000000000000000"})
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

func VerifyTransaction(tx *core.Transaction) error {
	proof := new(MerkleProof)
	mainChainTransaction := new(ela.Transaction)

	payloadObj, ok := tx.Payload.(*core.PayloadRechargeToSideChain)
	if !ok {
		return errors.New("Invalid payload core.PayloadRechargeToSideChain")
	}

	reader := bytes.NewReader(payloadObj.MerkleProof)
	if err := proof.Deserialize(reader); err != nil {
		return errors.New("RechargeToSideChain payload deserialize failed")
	}
	reader = bytes.NewReader(payloadObj.MainChainTransaction)
	if err := mainChainTransaction.Deserialize(reader); err != nil {
		return errors.New("RechargeToSideChain mainChainTransaction deserialize failed")
	}

	if err := spvService.VerifyTransaction(*proof, *mainChainTransaction); err != nil {
		return errors.New("SPV module verify transaction failed.")
	}

	return nil
}

func VerifyElaHeader(hash *common.Uint256) error {
	blockChain := spvService.HeaderStore()
	_, err := blockChain.GetHeader(hash)
	if err != nil {
		return errors.New("Verify ela header failed.")
	}

	return nil
}

type SpvListener struct {
	ListenAddress string
}

func (l *SpvListener) Address() string {
	return l.ListenAddress
}

func (l *SpvListener) Type() ela.TransactionType {
	return ela.RechargeToSideChain
}

func (l *SpvListener) Flags() uint64 {
	return spv.FlagNotifyInSyncing
}

func (l *SpvListener) Rollback(height uint32) {
}

func (l *SpvListener) Notify(id common.Uint256, proof bloom.MerkleProof, tx ela.Transaction) {
	// Submit transaction receipt
	defer spvService.SubmitTransactionReceipt(id, tx.Hash())
}
