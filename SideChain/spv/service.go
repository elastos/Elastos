package spv

import (
	"bytes"
	"errors"
	"fmt"
	"sync"

	"github.com/elastos/Elastos.ELA.SideChain/config"
	"github.com/elastos/Elastos.ELA.SideChain/types"

	spv "github.com/elastos/Elastos.ELA.SPV/interface"
	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/elalog"
	"github.com/elastos/Elastos.ELA/bloom"
	"github.com/elastos/Elastos.ELA/core"
)

type Config struct {
	Logger        elalog.Logger
	ChainStore    *blockchain.ChainStore
	ListenAddress string
}

type Service struct {
	spv.SPVService

	db *blockchain.ChainStore
}

func NewService(cfg *Config) (*Service, error) {
	spv.UseLogger(cfg.Logger)

	params := config.Parameters
	spvCfg := &spv.Config{
		Magic:           params.SpvMagic,
		Foundation:      params.MainChainFoundationAddress,
		SeedList:        params.SpvSeedList,
		DefaultPort:     params.MainChainDefaultPort,
		MinPeersForSync: params.MinPeersForSync,
		MinOutbound:     params.SpvMinOutbound,
		MaxConnections:  params.SpvMaxConnections,
		OnRollback:      nil, // Not implemented yet
	}

	service, err := spv.NewSPVService(spvCfg)
	if err != nil {
		return nil, err
	}

	//register an invalid address to prevent bloom filter from sending all data
	err = service.RegisterTransactionListener(&listener{
		address: cfg.ListenAddress,
		service: service,
	})
	if err != nil {
		return nil, err
	}

	return &Service{SPVService: service, db: cfg.ChainStore}, nil
}

func (s *Service) VerifyTransaction(tx *types.Transaction) error {
	payloadObj, ok := tx.Payload.(*types.PayloadRechargeToSideChain)
	if !ok {
		return errors.New("[VerifyTransaction] Invalid payload core.PayloadRechargeToSideChain")
	}
	if tx.PayloadVersion == types.RechargeToSideChainPayloadVersion0 {
		proof := new(bloom.MerkleProof)
		mainChainTransaction := new(core.Transaction)

		reader := bytes.NewReader(payloadObj.MerkleProof)
		if err := proof.Deserialize(reader); err != nil {
			return errors.New("[VerifyTransaction] RechargeToSideChain payload deserialize failed")
		}

		reader = bytes.NewReader(payloadObj.MainChainTransaction)
		if err := mainChainTransaction.Deserialize(reader); err != nil {
			return errors.New("[VerifyTransaction] RechargeToSideChain mainChainTransaction deserialize failed")
		}
		if err := s.SPVService.VerifyTransaction(*proof, *mainChainTransaction); err != nil {
			return errors.New("[VerifyTransaction] SPV module verify transaction failed.")
		}
	} else if tx.PayloadVersion == types.RechargeToSideChainPayloadVersion1 {
		_, err := s.db.GetSpvMainchainTx(payloadObj.MainChainTransactionHash)
		if err != nil {
			return errors.New("[VerifyTransaction] Main chain transaction not found")
		}
		return nil
	} else {
		return errors.New("[VerifyTransaction] invalid payload version.")
	}
	return nil
}

func (s *Service) VerifyElaHeader(hash *common.Uint256) error {
	blockChain := s.HeaderStore()
	_, err := blockChain.Get(hash)
	if err != nil {
		return errors.New("[VerifyElaHeader] Verify ela header failed.")
	}
	return nil
}

type listener struct {
	address string
	service spv.SPVService
	db      *blockchain.ChainStore

	mux sync.Mutex
}

func (l *listener) Address() string {
	return l.address
}

func (l *listener) Type() core.TransactionType {
	return core.RechargeToSideChain
}

func (l *listener) Flags() uint64 {
	return spv.FlagNotifyInSyncing
}

func (l *listener) Notify(id common.Uint256, proof bloom.MerkleProof, tx core.Transaction) {
	l.mux.Lock()
	defer l.mux.Unlock()

	mcTx, err := l.db.GetSpvMainchainTx(tx.Hash())
	if err != nil {
		fmt.Println("[Notify] tx already exist")
		return
	}

	batch := l.db.NewBatch()
	l.db.PersistSpvMainchainTx(batch, mcTx)
	err = batch.Commit()
	if err != nil {
		fmt.Errorf("[Notify] persist spv main chain tx failed, err:", err.Error())
		return
	}

	// Submit transaction receipt
	l.service.SubmitTransactionReceipt(id, tx.Hash())
}
