package spv

import (
	"bytes"
	"crypto/rand"
	"encoding/binary"
	"errors"
	"fmt"
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

	"github.com/boltdb/bolt"
	. "github.com/elastos/Elastos.ELA.Utility/common"
	"strings"
)

var spvService spv.SPVService

func SpvInit(addrCode string) error {
	var err error
	spvlog.Init(config.Parameters.SpvPrintLevel, 20, 1024)

	var id = make([]byte, 8)
	var clientId uint64
	rand.Read(id)
	binary.Read(bytes.NewReader(id), binary.LittleEndian, &clientId)

	spvService, err = spv.NewSPVService(config.Parameters.SpvMagic, config.Parameters.MainChainFoundationAddress, clientId,
		config.Parameters.SpvSeedList, config.Parameters.SpvMinOutbound, config.Parameters.SpvMaxConnections)
	if err != nil {
		return err
	}

	//register an invalid address to prevent bloom filter from sending all data
	err = spvService.RegisterTransactionListener(&SpvListener{ListenAddress: addrCode})
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
	return ela.TransferCrossChainAsset
}

func (l *SpvListener) Flags() uint64 {
	return spv.FlagNotifyInSyncing
}

func (l *SpvListener) Rollback(height uint32) {
}

func (l *SpvListener) Notify(id common.Uint256, proof bloom.MerkleProof, tx ela.Transaction) {
	// Submit transaction receipt
	fmt.Println(" ")
	fmt.Println(" ")
	fmt.Println("========================================================================================")
	fmt.Println("mainchain transaction info")
	fmt.Println("----------------------------------------------------------------------------------------")
	fmt.Println(string(tx.String()))
	fmt.Println("----------------------------------------------------------------------------------------")
	fmt.Println(" ")
	savePayloadInfo(tx)
	defer spvService.SubmitTransactionReceipt(id, tx.Hash())
}

func savePayloadInfo(elaTx ela.Transaction) error {
	db, err := bolt.Open("spv_transaction_info.db", 0644, &bolt.Options{InitialMmapSize: 5000000})
	if err != nil {
		fmt.Println(err)
	}
	defer db.Close()

	err = db.Update(func(tx *bolt.Tx) error {
		tx.CreateBucketIfNotExists([]byte("payload"))
		b := tx.Bucket([]byte("payload"))
		err = b.Put([]byte(elaTx.Hash().String()), []byte(BytesToHexString(elaTx.Payload.Data(elaTx.PayloadVersion))))
		return err
	})
	if err != nil {
		fmt.Println(err)
	}
	return nil
}

func FindPayloadByTransactionHash(transactionHash string) string {
	var data = []byte("0")
	if transactionHash != "" {
		transactionHash = strings.Replace(transactionHash, "0x", "", 1)
		db, err := bolt.Open("spv_transaction_info.db", 0644, &bolt.Options{InitialMmapSize: 5000000})
		if err != nil {
			fmt.Println(err)
		}
		defer db.Close()

		db.View(func(tx *bolt.Tx) error {
			b := tx.Bucket([]byte("payload"))
			if b == nil {
				return errors.New("No payload Bucket found")
			}
			v := b.Get([]byte(transactionHash))
			if v != nil {
				data = v
			}
			return nil
		})
	}
	return string(data)
}
