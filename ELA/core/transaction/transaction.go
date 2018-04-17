package transaction

import (
	"bytes"
	"errors"
	"fmt"
	"io"
	"sort"

	. "github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/serialize"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/transaction/payload"
)

//for different transaction types with different payload format
//and transaction process methods
type TransactionType byte

const (
	CoinBase                TransactionType = 0x00
	RegisterAsset           TransactionType = 0x01
	TransferAsset           TransactionType = 0x02
	Record                  TransactionType = 0x03
	Deploy                  TransactionType = 0x04
	SideMining              TransactionType = 0x05
	IssueToken              TransactionType = 0x06
	TransferCrossChainAsset TransactionType = 0x07
)

func (self TransactionType) Name() string {
	switch self {
	case CoinBase:
		return "CoinBase"
	case RegisterAsset:
		return "RegisterAsset"
	case TransferAsset:
		return "TransferAsset"
	case Record:
		return "Record"
	case Deploy:
		return "Deploy"
	case SideMining:
		return "SideMining"
	case IssueToken:
		return "IssueToken"
	case TransferCrossChainAsset:
		return "TransferCrossChainAsset"
	default:
		return "Unknown"
	}
}

const (
	InvalidTransactionSize = -1
)

//Payload define the func for loading the payload data
//base on payload type which have different struture
type Payload interface {
	//  Get payload data
	Data(version byte) []byte

	//Serialize payload data
	Serialize(w io.Writer, version byte) error

	Deserialize(r io.Reader, version byte) error
}

func GetPayload(txType TransactionType) (Payload, error) {
	var p Payload
	switch txType {
	case CoinBase:
		p = new(payload.CoinBase)
	case RegisterAsset:
		p = new(payload.RegisterAsset)
	case TransferAsset:
		p = new(payload.TransferAsset)
	case Record:
		p = new(payload.Record)
	case Deploy:
		p = new(payload.DeployCode)
	case SideMining:
		p = new(payload.SideMining)
	case IssueToken:
		p = new(payload.IssueToken)
	case TransferCrossChainAsset:
		p = new(payload.TransferCrossChainAsset)
	default:
		return nil, errors.New("[Transaction], invalid transaction type.")
	}
	return p, nil
}

var TxStore ILedgerStore

type Transaction struct {
	TxType         TransactionType
	PayloadVersion byte
	Payload        Payload
	Attributes     []*Attribute
	Inputs         []*Input
	Outputs        []*Output
	LockTime       uint32
	Programs       []*program.Program
	Fee            Fixed64
	FeePerKB       Fixed64

	hash *Uint256
}

func (tx *Transaction) String() string {
	hash := tx.Hash()
	return fmt.Sprint("Transaction: {\n\t",
		"Hash: ", hash.String(), "\n\t",
		"TxType: ", tx.TxType.Name(), "\n\t",
		"PayloadVersion: ", tx.PayloadVersion, "\n\t",
		"Payload: ", BytesToHexString(tx.Payload.Data(tx.PayloadVersion)), "\n\t",
		"Attributes: ", tx.Attributes, "\n\t",
		"Inputs: ", tx.Inputs, "\n\t",
		"Outputs: ", tx.Outputs, "\n\t",
		"LockTime: ", tx.LockTime, "\n\t",
		"Programs: ", tx.Programs, "\n\t",
		"}\n")
}

//Serialize the Transaction
func (tx *Transaction) Serialize(w io.Writer) error {

	err := tx.SerializeUnsigned(w)
	if err != nil {
		return errors.New("Transaction txSerializeUnsigned Serialize failed.")
	}
	//Serialize  Transaction's programs
	lens := uint64(len(tx.Programs))
	err = serialize.WriteVarUint(w, lens)
	if err != nil {
		return errors.New("Transaction WriteVarUint failed.")
	}
	if lens > 0 {
		for _, p := range tx.Programs {
			err = p.Serialize(w)
			if err != nil {
				return errors.New("Transaction Programs Serialize failed.")
			}
		}
	}
	return nil
}

//Serialize the Transaction data without contracts
func (tx *Transaction) SerializeUnsigned(w io.Writer) error {
	//txType
	w.Write([]byte{byte(tx.TxType)})
	//PayloadVersion
	w.Write([]byte{tx.PayloadVersion})
	//Payload
	if tx.Payload == nil {
		return errors.New("Transaction Payload is nil.")
	}
	tx.Payload.Serialize(w, tx.PayloadVersion)
	//[]*txAttribute
	err := serialize.WriteVarUint(w, uint64(len(tx.Attributes)))
	if err != nil {
		return errors.New("Transaction item txAttribute length serialization failed.")
	}
	if len(tx.Attributes) > 0 {
		for _, attr := range tx.Attributes {
			attr.Serialize(w)
		}
	}
	//[]*Inputs
	err = serialize.WriteVarUint(w, uint64(len(tx.Inputs)))
	if err != nil {
		return errors.New("Transaction item Inputs length serialization failed.")
	}
	if len(tx.Inputs) > 0 {
		for _, utxo := range tx.Inputs {
			utxo.Serialize(w)
		}
	}
	// TODO BalanceInputs
	//[]*Outputs
	err = serialize.WriteVarUint(w, uint64(len(tx.Outputs)))
	if err != nil {
		return errors.New("Transaction item Outputs length serialization failed.")
	}
	if len(tx.Outputs) > 0 {
		for _, output := range tx.Outputs {
			err = output.Serialize(w)
			if err != nil {
				return err
			}
		}
	}

	serialize.WriteUint32(w, tx.LockTime)

	return nil
}

//deserialize the Transaction
func (tx *Transaction) Deserialize(r io.Reader) error {
	// tx deserialize
	err := tx.DeserializeUnsigned(r)
	if err != nil {
		return errors.New("transaction Deserialize error")
	}

	// tx program
	lens, err := serialize.ReadVarUint(r, 0)
	if err != nil {
		return errors.New("transaction tx program Deserialize error")
	}

	programHashes := []*program.Program{}
	if lens > 0 {
		for i := 0; i < int(lens); i++ {
			outputHashes := new(program.Program)
			err = outputHashes.Deserialize(r)
			if err != nil {
				return err
			}
			programHashes = append(programHashes, outputHashes)
		}
		tx.Programs = programHashes
	}
	return nil
}

func (tx *Transaction) DeserializeUnsigned(r io.Reader) error {
	var txType = make([]byte, 1)
	_, err := r.Read(txType)
	if err != nil {
		return err
	}
	tx.TxType = TransactionType(txType[0])

	var payloadVersion = make([]byte, 1)
	_, err = r.Read(payloadVersion)
	tx.PayloadVersion = payloadVersion[0]
	if err != nil {
		return err
	}

	tx.Payload, err = GetPayload(tx.TxType)
	if err != nil {
		return err
	}

	err = tx.Payload.Deserialize(r, tx.PayloadVersion)
	if err != nil {
		return errors.New("Payload Parse error")
	}
	//attributes
	Len, err := serialize.ReadVarUint(r, 0)
	if err != nil {
		return err
	}
	if Len > uint64(0) {
		for i := uint64(0); i < Len; i++ {
			attr := new(Attribute)
			err = attr.Deserialize(r)
			if err != nil {
				return err
			}
			tx.Attributes = append(tx.Attributes, attr)
		}
	}
	//Inputs
	Len, err = serialize.ReadVarUint(r, 0)
	if err != nil {
		return err
	}
	if Len > uint64(0) {
		for i := uint64(0); i < Len; i++ {
			utxo := new(Input)
			err = utxo.Deserialize(r)
			if err != nil {
				return err
			}
			tx.Inputs = append(tx.Inputs, utxo)
		}
	}
	//TODO balanceInputs
	//Outputs
	Len, err = serialize.ReadVarUint(r, 0)
	if err != nil {
		return err
	}
	if Len > uint64(0) {
		for i := uint64(0); i < Len; i++ {
			output := new(Output)
			err = output.Deserialize(r)
			if err != nil {
				return err
			}
			tx.Outputs = append(tx.Outputs, output)
		}
	}

	temp, err := serialize.ReadUint32(r)
	tx.LockTime = uint32(temp)
	if err != nil {
		return err
	}

	return nil
}

func (tx *Transaction) GetSize() int {
	var buffer bytes.Buffer
	if err := tx.Serialize(&buffer); err != nil {
		return InvalidTransactionSize
	}

	return buffer.Len()
}

func (tx *Transaction) GetFee(assetID Uint256) int64 {
	res, err := tx.GetTransactionResults()
	if err != nil {
		return 0
	}

	return int64(res[assetID])
}

func (tx *Transaction) GetProgramHashes() ([]Uint168, error) {
	if tx == nil {
		return []Uint168{}, errors.New("[Transaction],GetProgramHashes transaction is nil.")
	}
	hashs := []Uint168{}
	uniqHashes := []Uint168{}
	// add inputUTXO's transaction
	referenceWithUTXO_Output, err := tx.GetReference()
	if err != nil {
		return nil, errors.New("[Transaction], GetProgramHashes failed.")
	}
	for _, output := range referenceWithUTXO_Output {
		programHash := output.ProgramHash
		hashs = append(hashs, programHash)
	}
	for _, attribute := range tx.Attributes {
		if attribute.Usage == Script {
			dataHash, err := Uint168FromBytes(attribute.Data)
			if err != nil {
				return nil, errors.New("[Transaction], GetProgramHashes err.")
			}
			hashs = append(hashs, *dataHash)
		}
	}
	switch tx.TxType {
	case RegisterAsset:
	case TransferAsset:
	case Record:
	case Deploy:
	case SideMining:
	default:
	}

	//remove dupilicated hashes
	uniq := make(map[Uint168]bool)
	for _, v := range hashs {
		uniq[v] = true
	}
	for k := range uniq {
		uniqHashes = append(uniqHashes, k)
	}
	sort.Sort(byProgramHashes(uniqHashes))
	return uniqHashes, nil
}

func (tx *Transaction) SetPrograms(programs []*program.Program) {
	tx.Programs = programs
}

func (tx *Transaction) GetPrograms() []*program.Program {
	return tx.Programs
}

func (tx *Transaction) GetOutputHashes() ([]Uint168, error) {
	//TODO: implement Transaction.GetOutputHashes()

	return []Uint168{}, nil
}

func (tx *Transaction) GenerateAssetMaps() {
	//TODO: implement Transaction.GenerateAssetMaps()
}

func (tx *Transaction) Hash() Uint256 {
	if tx.hash == nil {
		buf := new(bytes.Buffer)
		tx.SerializeUnsigned(buf)
		hash := Uint256(Sha256D(buf.Bytes()))
		tx.hash = &hash
	}
	return *tx.hash
}
func (tx *Transaction) IsCoinBaseTx() bool {
	return tx.TxType == CoinBase
}

func (tx *Transaction) GetReference() (map[*Input]*Output, error) {
	if tx.TxType == RegisterAsset {
		return nil, nil
	}
	//UTXO input /  Outputs
	reference := make(map[*Input]*Output)
	// Key index，v UTXOInput
	for _, utxo := range tx.Inputs {
		transaction, _, err := TxStore.GetTransaction(utxo.Previous.TxID)
		if err != nil {
			return nil, errors.New("[Transaction], GetReference failed.")
		}
		index := utxo.Previous.Index
		if int(index) >= len(transaction.Outputs) {
			return nil, errors.New("[Transaction], GetReference failed, refIdx out of range.")
		}
		reference[utxo] = transaction.Outputs[index]
	}
	return reference, nil
}

func (tx *Transaction) GetTransactionResults() (TransactionResult, error) {
	result := make(map[Uint256]Fixed64)
	outputResult := tx.GetMergedAssetIDValueFromOutputs()
	InputResult, err := tx.GetMergedAssetIDValueFromReference()
	if err != nil {
		return nil, err
	}
	//calc the balance of input vs output
	for outputAssetid, outputValue := range outputResult {
		if inputValue, ok := InputResult[outputAssetid]; ok {
			result[outputAssetid] = inputValue - outputValue
		} else {
			result[outputAssetid] -= outputValue
		}
	}
	for inputAssetid, inputValue := range InputResult {
		if _, exist := result[inputAssetid]; !exist {
			result[inputAssetid] += inputValue
		}
	}
	return result, nil
}

func (tx *Transaction) GetMergedAssetIDValueFromOutputs() TransactionResult {
	var result = make(map[Uint256]Fixed64)
	for _, v := range tx.Outputs {
		amout, ok := result[v.AssetID]
		if ok {
			result[v.AssetID] = amout + v.Value
		} else {
			result[v.AssetID] = v.Value
		}
	}
	return result
}

func (tx *Transaction) GetMergedAssetIDValueFromReference() (TransactionResult, error) {
	reference, err := tx.GetReference()
	if err != nil {
		return nil, err
	}
	var result = make(map[Uint256]Fixed64)
	for _, v := range reference {
		amout, ok := result[v.AssetID]
		if ok {
			result[v.AssetID] = amout + v.Value
		} else {
			result[v.AssetID] = v.Value
		}
	}
	return result, nil
}

type byProgramHashes []Uint168

func (a byProgramHashes) Len() int      { return len(a) }
func (a byProgramHashes) Swap(i, j int) { a[i], a[j] = a[j], a[i] }
func (a byProgramHashes) Less(i, j int) bool {
	if a[i].Compare(a[j]) > 0 {
		return false
	} else {
		return true
	}
}

func NewTrimmed(hash *Uint256) *Transaction {
	tx := new(Transaction)
	tx.hash = hash
	return tx
}
