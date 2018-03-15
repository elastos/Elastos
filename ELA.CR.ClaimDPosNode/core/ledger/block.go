package ledger

import (
	"io"
	"time"
	"bytes"
	"errors"
	"math/rand"
	"encoding/binary"

	"Elastos.ELA/crypto"
	. "Elastos.ELA/common"
	"Elastos.ELA/core/asset"
	"Elastos.ELA/common/log"
	"Elastos.ELA/common/config"
	"Elastos.ELA/core/signature"
	tx "Elastos.ELA/core/transaction"
	"Elastos.ELA/common/serialization"
	"Elastos.ELA/core/contract/program"
	"Elastos.ELA/core/transaction/payload"
)

const (
	BlockVersion     uint32 = 0
	GenesisNonce     uint32 = 2083236893
	InvalidBlockSize int    = -1
)

var (
	MaxBlockSize = config.Parameters.MaxBlockSize
)

type Block struct {
	Blockdata    *Blockdata
	Transactions []*tx.Transaction

	hash *Uint256
}

func (b *Block) Serialize(w io.Writer) error {
	b.Blockdata.Serialize(w)
	err := serialization.WriteUint32(w, uint32(len(b.Transactions)))
	if err != nil {
		return errors.New("Block item Transactions length serialization failed.")
	}

	for _, transaction := range b.Transactions {
		transaction.Serialize(w)
	}
	return nil
}

func (b *Block) Deserialize(r io.Reader) error {
	if b.Blockdata == nil {
		b.Blockdata = new(Blockdata)
	}
	b.Blockdata.Deserialize(r)

	//Transactions
	var i uint32
	Len, err := serialization.ReadUint32(r)
	if err != nil {
		return err
	}
	var txhash *Uint256
	var tharray []*Uint256
	for i = 0; i < Len; i++ {
		transaction := new(tx.Transaction)
		transaction.Deserialize(r)
		txhash = transaction.Hash()
		b.Transactions = append(b.Transactions, transaction)
		tharray = append(tharray, txhash)
	}

	merkleRoot, err := crypto.ComputeRoot(tharray)
	if err != nil {
		return errors.New("Block Deserialize merkleTree compute failed")
	}
	b.Blockdata.TransactionsRoot = *merkleRoot

	return nil
}

func (b *Block) Trim(w io.Writer) error {
	b.Blockdata.Serialize(w)
	err := serialization.WriteUint32(w, uint32(len(b.Transactions)))
	if err != nil {
		return errors.New("Block item Transactions length serialization failed.")
	}
	for _, transaction := range b.Transactions {
		temp := *transaction
		hash := temp.Hash()
		hash.Serialize(w)
	}
	return nil
}

func (b *Block) FromTrimmedData(r io.Reader) error {
	if b.Blockdata == nil {
		b.Blockdata = new(Blockdata)
	}
	b.Blockdata.Deserialize(r)

	//Transactions
	var i uint32
	Len, err := serialization.ReadUint32(r)
	if err != nil {
		return err
	}
	var txhash *Uint256
	var tharray []*Uint256
	for i = 0; i < Len; i++ {
		txhash.Deserialize(r)
		transaction := new(tx.Transaction)
		transaction.SetHash(txhash)
		b.Transactions = append(b.Transactions, transaction)
		tharray = append(tharray, txhash)
	}

	merkleRoot, err := crypto.ComputeRoot(tharray)
	if err != nil {
		return errors.New("Block Deserialize merkleTree compute failed")
	}
	b.Blockdata.TransactionsRoot = *merkleRoot

	return nil
}

func (tx *Block) GetSize() int {
	var buffer bytes.Buffer
	if err := tx.Serialize(&buffer); err != nil {
		return InvalidBlockSize
	}

	return buffer.Len()
}

func (b *Block) GetDataContent() []byte {
	return signature.GetDataContent(b)
}

func (b *Block) GetProgramHashes() ([]Uint168, error) {
	return b.Blockdata.GetProgramHashes()
}

func (b *Block) SetPrograms(prog []*program.Program) {
	b.Blockdata.SetPrograms(prog)
}

func (b *Block) GetPrograms() []*program.Program {
	return b.Blockdata.GetPrograms()
}

func (b *Block) Hash() *Uint256 {
	if b.hash == nil {
		b.hash = new(Uint256)
		*b.hash = b.Blockdata.Hash()
	}
	return b.hash
}

func (b *Block) Verify() error {
	log.Info("This function is expired.please use Validation/blockValidator to Verify Block.")
	return nil
}

func GenesisBlockInit() (*Block, error) {
	genesisBlockdata := &Blockdata{
		Version:          BlockVersion,
		PrevBlockHash:    Uint256{},
		TransactionsRoot: Uint256{},
		Timestamp:        uint32(time.Unix(time.Date(2017, time.December, 22, 10, 0, 0, 0, time.UTC).Unix(), 0).Unix()),
		Bits:             0x1d03ffff,
		//Bits:   config.Parameters.ChainParam.PowLimitBits,
		Nonce:  GenesisNonce,
		Height: uint32(0),
	}

	//transaction
	systemToken := &tx.Transaction{
		TxType:         tx.RegisterAsset,
		PayloadVersion: 0,
		Payload: &payload.RegisterAsset{
			Asset: &asset.Asset{
				Name:      "ELA",
				Precision: 0x08,
				AssetType: 0x00,
			},
			Amount:     0 * 100000000,
			Controller: EmptyValue,
		},
		Attributes: []*tx.TxAttribute{},
		UTXOInputs: []*tx.UTXOTxInput{},
		Outputs:    []*tx.TxOutput{},
		Programs:   []*program.Program{},
	}

	foundationProgramHash, err := Uint68FromAddress(FoundationAddress)
	if err != nil {
		return nil, err
	}

	trans, err := tx.NewCoinBaseTransaction(&payload.CoinBase{}, 0)
	if err != nil {
		return nil, err
	}

	trans.Outputs = []*tx.TxOutput{
		{
			AssetID:     *systemToken.Hash(),
			Value:       3300 * 10000 * 100000000,
			ProgramHash: foundationProgramHash,
		},
	}

	nonce := make([]byte, 8)
	binary.BigEndian.PutUint64(nonce, rand.Uint64())
	txAttr := tx.NewTxAttribute(tx.Nonce, nonce)
	trans.Attributes = append(trans.Attributes, &txAttr)
	//block
	genesisBlock := &Block{
		Blockdata:    genesisBlockdata,
		Transactions: []*tx.Transaction{trans, systemToken},
	}
	txHashes := []*Uint256{}
	for _, tx := range genesisBlock.Transactions {
		txHashes = append(txHashes, tx.Hash())
	}
	merkleRoot, err := crypto.ComputeRoot(txHashes)
	if err != nil {
		return nil, errors.New("[GenesisBlock], merkle root error")
	}
	genesisBlock.Blockdata.TransactionsRoot = *merkleRoot

	return genesisBlock, nil
}

func (b *Block) RebuildMerkleRoot() error {
	txs := b.Transactions
	transactionHashes := []*Uint256{}
	for _, tx := range txs {
		transactionHashes = append(transactionHashes, tx.Hash())
	}
	hash, err := crypto.ComputeRoot(transactionHashes)
	if err != nil {
		return errors.New("[Block] , RebuildMerkleRoot ComputeRoot failed.")
	}
	b.Blockdata.TransactionsRoot = *hash
	return nil

}

func (bd *Block) SerializeUnsigned(w io.Writer) error {
	return bd.Blockdata.SerializeUnsigned(w)
}
