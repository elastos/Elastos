package types

import (
	"errors"
	"io"
	"bytes"

	"github.com/elastos/Elastos.ELA/common"

	"github.com/elastos/Elastos.ELA.SideChain/types"
)

// Receipt represents the results of a transaction.
type Receipt struct {
	// Consensus fields: These fields are defined by the Yellow Paper
	Status bool   `json:"status"`
	Logs   []*Nep5Log `json:"logs"              gencodec:"required"`

	// Implementation fields: These fields are added by geth when processing a transaction.
	// They are stored in the chain database.
	TxHash          common.Uint256 `json:"transactionHash" gencodec:"required"`
	ContractAddress common.Uint168 `json:"contractAddress"`
	GasUsed         uint64          `json:"gasUsed" gencodec:"required"`

	// Inclusion information: These fields provide information about the inclusion of the
	// transaction corresponding to this receipt.
	BlockHash        common.Uint256 `json:"blockHash,omitempty"`
	BlockNumber      uint32         `json:"blockNumber,omitempty"`
	TransactionIndex uint32         `json:"transactionIndex"`
}

// NewReceipt creates a barebone transaction receipt, copying the init fields.
func NewReceipt(contractAddr common.Uint168, tx *types.Transaction, block *types.Block) (*Receipt, error) {
	r := &Receipt{TxHash: tx.Hash(), Status: false, ContractAddress: contractAddr}
	r.BlockNumber = block.GetHeight()
	index, err := r.GetTxIndex(tx.Hash(), block)
	r.TransactionIndex = index
	if err != nil {
		return r, err
	}
	return r, nil
}

func (r *Receipt) GetTxIndex(txHash common.Uint256, block *types.Block) (uint32, error) {
	for i, tx := range block.Transactions {
		if tx.Hash().IsEqual(txHash) {
			return uint32(i), nil
		}
	}
	return 0, errors.New("transaction is not in block")
}

func (r *Receipt) Serialize(w io.Writer, version byte) error {
	var err error
	if r.Status == true {
		err = common.WriteUint8(w, 1)
	} else {
		err = common.WriteUint8(w, 0)
	}
	if err != nil {
		return err
	}

	err = common.WriteVarUint(w, uint64(len(r.Logs)))
	if err != nil {
		return err
	}
	for _, log := range r.Logs {
		err = log.Serialize(w, version)
		if err != nil {
			return err
		}
	}

	err = r.TxHash.Serialize(w)
	if err != nil {
		return err
	}

	err = r.ContractAddress.Serialize(w)
	if err != nil {
		return err
	}

	err = common.WriteUint64(w, r.GasUsed)
	if err != nil {
		return err
	}
	return nil
}

func (receipt *Receipt) Deserialize(r io.Reader, version byte) error {
	state, err := common.ReadUint8(r)
	if err != nil {
		return err
	}
	receipt.Status = false
	if state == 1 {
		receipt.Status = true
	}

	count, err := common.ReadVarUint(r, 0)
	if err != nil {
		return err
	}
	receipt.Logs = make([]*Nep5Log, count)
	for i := uint64(0); i < count; i++ {
		receipt.Logs[i] = &Nep5Log{}
		err = receipt.Logs[i].Deserialize(r, version)
		if err != nil {
			return err
		}
	}

	receipt.TxHash = common.Uint256{}
	err = receipt.TxHash.Deserialize(r)
	if err != nil {
		return err
	}

	receipt.ContractAddress = common.Uint168{}
	err = receipt.ContractAddress.Deserialize(r)
	if err != nil {
		return err
	}

	receipt.GasUsed, err = common.ReadUint64(r)
	if err != nil {
		return err
	}

	return nil
}

type Receipts []*Receipt

func (r Receipts) Hash() common.Uint256 {
	if len(r) == 0 {
		return common.EmptyHash
	}
	buf := new(bytes.Buffer)
	for _, receipt := range r {
		receipt.Serialize(buf, 0)
	}
	return common.Sha256D(buf.Bytes())
}