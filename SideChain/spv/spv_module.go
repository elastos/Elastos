package spv

import (
	"bytes"
	"errors"
	"os"

	"github.com/elastos/Elastos.ELA.SideChain/config"
	"github.com/elastos/Elastos.ELA.SideChain/core"
	"github.com/elastos/Elastos.ELA.SideChain/log"

	spv "github.com/elastos/Elastos.ELA.SPV/interface"
	logger "github.com/elastos/Elastos.ELA.SPV/log"
	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA/bloom"
	ela "github.com/elastos/Elastos.ELA/core"
)

const SPVLogPath = "./logs-spv/"

var service spv.SPVService

func SpvInit() error {
	var err error
	var params = config.Parameters
	logger := logger.NewLogger(SPVLogPath, params.SpvPrintLevel,
		10, 1024)

	spv.UseLogger(logger)

	config := &spv.Config{
		Magic:          params.SpvMagic,
		Foundation:     params.MainChainFoundationAddress,
		SeedList:       params.SeedList,
		DefaultPort:    params.MainChainDefaultPort,
		MinOutbound:    params.SpvMinOutbound,
		MaxConnections: params.SpvMaxConnections,
		OnRollback:     nil, // Not implemented yet
	}

	service, err = spv.NewSPVService(config)
	if err != nil {
		return err
	}

	//register an invalid address to prevent bloom filter from sending all data
	err = service.RegisterTransactionListener(&SpvListener{
		ListenAddress: "XagqqFetxiDb9wbartKDrXgnqLagy5yY1z",
	})
	if err != nil {
		return err
	}

	go func() {
		if err := service.Start(); err != nil {
			log.Info("Spv service start failed ：", err)
		}
		log.Info("Spv service stoped")
		os.Exit(-1)
	}()
	return nil
}

func VerifyTransaction(tx *core.Transaction) error {
	proof := new(bloom.MerkleProof)
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

	if err := service.VerifyTransaction(*proof, *mainChainTransaction); err != nil {
		return errors.New("SPV module verify transaction failed.")
	}

	return nil
}

func VerifyElaHeader(hash *common.Uint256) error {
	blockChain := service.HeaderStore()
	_, err := blockChain.Get(hash)
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

func (l *SpvListener) Notify(id common.Uint256, proof bloom.MerkleProof, tx ela.Transaction) {
	// Submit transaction receipt
	defer service.SubmitTransactionReceipt(id, tx.Hash())
}
