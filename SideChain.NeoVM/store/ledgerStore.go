package store

import (
	"errors"
	"bytes"
	"fmt"

	sb "github.com/elastos/Elastos.ELA.SideChain/blockchain"
	side "github.com/elastos/Elastos.ELA.SideChain/types"
	"github.com/elastos/Elastos.ELA.SideChain/database"
	"github.com/elastos/Elastos.ELA.SideChain/interfaces"

	"github.com/elastos/Elastos.ELA/common"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/types"
)

var (
	ErrDBNotFound = errors.New("leveldb: not found")
)

type LedgerStore struct {
	*sb.ChainStore
}

func NewLedgerStore(store *sb.ChainStore) (*LedgerStore, error) {
	ledger := &LedgerStore{
		ChainStore: store,
	}
	ledger.RegisterFunctions(true, sb.StoreFuncNames.PersistTransactions, ledger.persistTransactions)
	ledger.RegisterFunctions(true, PersisAccount, ledger.PersisAccount)

	return ledger, nil
}

func (c *LedgerStore) persistTransactions(batch database.Batch, b *side.Block) error {
	c.WriteTxLookupEntries(b)
	var receipts types.Receipts
	for _, txn := range b.Transactions {
		if err := c.PersistTransaction(batch, txn, b.Header.GetHeight()); err != nil {
			return err
		}

		if txn.TxType == side.RegisterAsset {
			regPayload := txn.Payload.(*side.PayloadRegisterAsset)
			if err := c.PersistAsset(batch, txn.Hash(), regPayload.Asset); err != nil {
				return err
			}
		}

		if txn.TxType == side.RechargeToSideChain {
			rechargePayload := txn.Payload.(*side.PayloadRechargeToSideChain)
			hash, err := rechargePayload.GetMainchainTxHash(txn.PayloadVersion)
			if err != nil {
				return err
			}
			c.PersistMainchainTx(batch, *hash)
		}

		if txn.TxType == side.Deploy {
			err := c.PersistDeployTransaction(b, txn, batch)
			if err != nil {
				log.Error(err.Error())
				//return err will effect manual mining
				return nil
			}
		}

		if txn.TxType == side.Invoke {
			receipt, err := c.PersisInvokeTransaction(b, txn, batch)
			if err != nil {
				log.Error(err.Error())
				//return err will effect manual mining
				return nil
			}
			receipts = append(receipts, receipt)
		}
	}
	if len(receipts) > 0 {
		//CheckBlockStates
		err  := checkBlockStates(b, receipts)

		if err != nil {
			log.Error("CheckBlockStates errors:", err)
		}

		err = c.WriteReceipts(b, receipts)
		if err != nil {
			log.Error("WriteReceipts errors:", err)
		}

	}
	return nil
}


func checkBlockStates(block *side.Block, receipts types.Receipts) error {
	header := block.Header.(*types.Header)
	// Validate the received block's bloom with the one derived from the generated receipts.
	// For valid blocks this should always validate to true.
	rbloom := types.CreateBloom(receipts).Bytes()
	if !bytes.Equal(rbloom, header.Bloom) {
		return fmt.Errorf("invalid bloom (remote: %x  local: %x)", header.Bloom, rbloom)
	}

	// Tre receipt hash
	receiptSha := receipts.Hash()
	if !receiptSha.IsEqual(header.ReceiptHash) {
		return fmt.Errorf("invalid receipt hash (remote: %x local: %x)", header.ReceiptHash, receiptSha)
	}

	return nil
}

func (c *LedgerStore) GetUnspents(txid common.Uint256) ([]*side.Output, error) {
	if ok, _ := c.ContainsUnspent(txid, 0); ok {
		tx, _, err := c.GetTransaction(txid)
		if err != nil {
			return nil, err
		}
		return tx.Outputs, nil
	}
	return nil, errors.New("[GetUnspent] NOT ContainsUnspent.")
}

func (s *LedgerStore) GetHeader(hash common.Uint256) (interfaces.Header, error) {
	var h = types.NewHeader()

	prefix := []byte{byte(sb.DATA_Header)}
	data, err := s.Get(append(prefix, hash.Bytes()...))
	if err != nil {
		//TODO: implement error process
		return nil, err
	}

	r := bytes.NewReader(data)
	// first 8 bytes is sys_fee
	_, err = common.ReadUint64(r)
	if err != nil {
		return nil, err
	}

	// Deserialize block data
	err = h.Deserialize(r)
	if err != nil {
		return nil, err
	}

	return h, err
}