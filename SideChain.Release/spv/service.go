package spv

import (
	"bytes"
	"errors"
	"github.com/elastos/Elastos.ELA.SideChain/config"
	"github.com/elastos/Elastos.ELA.SideChain/core"
	"github.com/elastos/Elastos.ELA.Utility/elalog"

	spv "github.com/elastos/Elastos.ELA.SPV/interface"
	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA/bloom"
	ela "github.com/elastos/Elastos.ELA/core"
)

type Service struct {
	spv.SPVService
}

func NewService(logger elalog.Logger) (*Service, error) {
	spv.UseLogger(logger)

	params := config.Parameters
	cfg := &spv.Config{
		Magic:          params.SpvMagic,
		Foundation:     params.MainChainFoundationAddress,
		SeedList:       params.SpvSeedList,
		DefaultPort:    params.MainChainDefaultPort,
		MinOutbound:    params.SpvMinOutbound,
		MaxConnections: params.SpvMaxConnections,
		OnRollback:     nil, // Not implemented yet
	}

	service, err := spv.NewSPVService(cfg)
	if err != nil {
		return nil, err
	}

	//register an invalid address to prevent bloom filter from sending all data
	err = service.RegisterTransactionListener(&listener{
		address: "XagqqFetxiDb9wbartKDrXgnqLagy5yY1z",
		service: service,
	})
	if err != nil {
		return nil, err
	}

	return &Service{service}, nil
}

func (s *Service) VerifyTransaction(tx *core.Transaction) error {
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

	if err := s.SPVService.VerifyTransaction(*proof, *mainChainTransaction); err != nil {
		return errors.New("SPV module verify transaction failed.")
	}

	return nil
}

func (s *Service) VerifyElaHeader(hash *common.Uint256) error {
	blockChain := s.SPVService.HeaderStore()
	_, err := blockChain.Get(hash)
	if err != nil {
		return errors.New("Verify ela header failed.")
	}

	return nil
}

type listener struct {
	address string
	service spv.SPVService
}

func (l *listener) Address() string {
	return l.address
}

func (l *listener) Type() ela.TransactionType {
	return ela.RechargeToSideChain
}

func (l *listener) Flags() uint64 {
	return spv.FlagNotifyInSyncing
}

func (l *listener) Notify(id common.Uint256, proof bloom.MerkleProof, tx ela.Transaction) {
	// Submit transaction receipt
	l.service.SubmitTransactionReceipt(id, tx.Hash())
}
