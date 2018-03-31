package auxpow

import (
	"bytes"
	"crypto/sha256"
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA.SideChain/common"
	"github.com/elastos/Elastos.ELA.SideChain/common/serialization"
	"github.com/elastos/Elastos.ELA.SideChain/core/contract/program"
)

//for different transaction types with different payload format
//and transaction process methods
type TransactionType byte

const (
	CoinBase      TransactionType = 0x00
	RegisterAsset TransactionType = 0x01
	TransferAsset TransactionType = 0x02
	Record        TransactionType = 0x03
	Deploy        TransactionType = 0x04
	SideMining    TransactionType = 0x05
)

const SideMiningPayloadVersion byte = 0x00

type SideMiningPayload struct {
	SideBlockHash   common.Uint256
	SideGenesisHash common.Uint256
}

func (a *SideMiningPayload) Data(version byte) []byte {
	data := make([]uint8, 0)
	data = append(data, a.SideBlockHash[:]...)
	data = append(data, a.SideGenesisHash[:]...)

	return data[:]
}

func (a *SideMiningPayload) Serialize(w io.Writer, version byte) error {
	_, err := a.SideBlockHash.Serialize(w)
	if err != nil {
		return err
	}
	_, err = a.SideGenesisHash.Serialize(w)
	if err != nil {
		return err
	}
	return nil
}

func (a *SideMiningPayload) Deserialize(r io.Reader, version byte) error {
	err := a.SideBlockHash.Deserialize(r)
	if err != nil {
		return err
	}
	err = a.SideGenesisHash.Deserialize(r)
	if err != nil {
		return err
	}
	return nil
}

type TransactionAttributeUsage byte

const (
	Nonce          TransactionAttributeUsage = 0x00
	Script         TransactionAttributeUsage = 0x20
	DescriptionUrl TransactionAttributeUsage = 0x81
	Description    TransactionAttributeUsage = 0x90
)

type TxAttribute struct {
	Usage TransactionAttributeUsage
	Data  []byte
	Size  uint32
}

func IsValidAttributeType(usage TransactionAttributeUsage) bool {
	return usage == Nonce || usage == Script ||
		usage == DescriptionUrl || usage == Description
}

func (tx *TxAttribute) Serialize(w io.Writer) error {
	if err := serialization.WriteUint8(w, byte(tx.Usage)); err != nil {
		return errors.New("ElaTx attribute Usage serialization error.")
	}
	if !IsValidAttributeType(tx.Usage) {
		return errors.New("[TxAttribute error] Unsupported attribute Description.")
	}
	if err := serialization.WriteVarBytes(w, tx.Data); err != nil {
		return errors.New("ElaTx attribute Data serialization error.")
	}
	return nil
}

func (tx *TxAttribute) Deserialize(r io.Reader) error {
	val, err := serialization.ReadBytes(r, 1)
	if err != nil {
		return errors.New("ElaTx attribute Usage deserialization error.")
	}
	tx.Usage = TransactionAttributeUsage(val[0])
	if !IsValidAttributeType(tx.Usage) {
		return errors.New("[TxAttribute error] Unsupported attribute Description.")
	}
	tx.Data, err = serialization.ReadVarBytes(r)
	if err != nil {
		return errors.New("ElaTx attribute Data deserialization error.")
	}
	return nil
}

type UTXOTxInput struct {

	//Indicate the previous Tx which include the UTXO output for usage
	ReferTxID common.Uint256

	//The index of output in the referTx output list
	ReferTxOutputIndex uint16

	// Sequence number
	Sequence uint32
}

func (ui *UTXOTxInput) Serialize(w io.Writer) {
	ui.ReferTxID.Serialize(w)
	serialization.WriteUint16(w, ui.ReferTxOutputIndex)
	serialization.WriteUint32(w, ui.Sequence)
}

func (ui *UTXOTxInput) Deserialize(r io.Reader) error {
	//referTxID
	err := ui.ReferTxID.Deserialize(r)
	if err != nil {
		return err
	}

	//Output Index
	temp, err := serialization.ReadUint16(r)
	ui.ReferTxOutputIndex = uint16(temp)
	if err != nil {
		return err
	}

	temp2, err := serialization.ReadUint32(r)
	ui.Sequence = uint32(temp2)
	if err != nil {
		return err
	}

	return nil
}

type BalanceTxInput struct {
	AssetID     common.Uint256
	Value       common.Fixed64
	ProgramHash common.Uint168
}

func (bi *BalanceTxInput) Serialize(w io.Writer) {
	bi.AssetID.Serialize(w)
	bi.Value.Serialize(w)
	bi.ProgramHash.Serialize(w)
}

func (bi *BalanceTxInput) Deserialize(r io.Reader) error {
	err := bi.AssetID.Deserialize(r)
	if err != nil {
		return err
	}

	err = bi.Value.Deserialize(r)
	if err != nil {
		return err
	}

	err = bi.ProgramHash.Deserialize(r)
	if err != nil {
		return err
	}

	return nil
}

type TxOutput struct {
	AssetID     common.Uint256
	Value       common.Fixed64
	OutputLock  uint32
	ProgramHash common.Uint168
}

func (o *TxOutput) Serialize(w io.Writer) error {
	_, err := o.AssetID.Serialize(w)
	if err != nil {
		return err
	}

	err = o.Value.Serialize(w)
	if err != nil {
		return err
	}

	serialization.WriteUint32(w, o.OutputLock)

	_, err = o.ProgramHash.Serialize(w)
	if err != nil {
		return err
	}

	return nil
}

func (o *TxOutput) Deserialize(r io.Reader) error {
	err := o.AssetID.Deserialize(r)
	if err != nil {
		return err
	}

	err = o.Value.Deserialize(r)
	if err != nil {
		return err
	}

	temp, err := serialization.ReadUint32(r)
	if err != nil {
		return err
	}
	o.OutputLock = uint32(temp)

	err = o.ProgramHash.Deserialize(r)
	if err != nil {
		return err
	}

	return nil
}

type ElaTx struct {
	TxType         TransactionType
	PayloadVersion byte
	Payload        SideMiningPayload
	Attributes     []*TxAttribute
	UTXOInputs     []*UTXOTxInput
	BalanceInputs  []*BalanceTxInput
	Outputs        []*TxOutput
	LockTime       uint32
	Programs       []*program.Program
}

func NewSideMiningTx(txPayload SideMiningPayload, currentHeight uint32) *ElaTx {
	return &ElaTx{
		TxType:  SideMining,
		Payload: txPayload,
		UTXOInputs: []*UTXOTxInput{
			{
				ReferTxID:          common.Uint256{},
				ReferTxOutputIndex: 0x0000,
				Sequence:           0x00000000,
			},
		},
		BalanceInputs: []*BalanceTxInput{},
		Attributes:    []*TxAttribute{},
		LockTime:      currentHeight,
		Programs:      []*program.Program{},
	}
}

//Serialize the ElaTx
func (tx *ElaTx) Serialize(w io.Writer) error {

	err := tx.SerializeUnsigned(w)
	if err != nil {
		return errors.New("ElaTx txSerializeUnsigned Serialize failed.")
	}
	//Serialize  ElaTx's programs
	lens := uint64(len(tx.Programs))
	err = serialization.WriteVarUint(w, lens)
	if err != nil {
		return errors.New("ElaTx WriteVarUint failed.")
	}
	if lens > 0 {
		for _, p := range tx.Programs {
			err = p.Serialize(w)
			if err != nil {
				return errors.New("ElaTx Programs Serialize failed.")
			}
		}
	}
	return nil
}

//Serialize the ElaTx data without contracts
func (tx *ElaTx) SerializeUnsigned(w io.Writer) error {
	//txType
	w.Write([]byte{byte(tx.TxType)})
	//PayloadVersion
	w.Write([]byte{tx.PayloadVersion})
	//Payload
	tx.Payload.Serialize(w, tx.PayloadVersion)
	//[]*txAttribute
	err := serialization.WriteVarUint(w, uint64(len(tx.Attributes)))
	if err != nil {
		return errors.New("ElaTx item txAttribute length serialization failed.")
	}
	if len(tx.Attributes) > 0 {
		for _, attr := range tx.Attributes {
			attr.Serialize(w)
		}
	}
	//[]*UTXOInputs
	err = serialization.WriteVarUint(w, uint64(len(tx.UTXOInputs)))
	if err != nil {
		return errors.New("ElaTx item UTXOInputs length serialization failed.")
	}
	if len(tx.UTXOInputs) > 0 {
		for _, utxo := range tx.UTXOInputs {
			utxo.Serialize(w)
		}
	}
	// TODO BalanceInputs
	//[]*Outputs
	err = serialization.WriteVarUint(w, uint64(len(tx.Outputs)))
	if err != nil {
		return errors.New("ElaTx item Outputs length serialization failed.")
	}
	if len(tx.Outputs) > 0 {
		for _, output := range tx.Outputs {
			err = output.Serialize(w)
			if err != nil {
				return err
			}
		}
	}

	serialization.WriteUint32(w, tx.LockTime)

	return nil
}

//deserialize the ElaTx
func (tx *ElaTx) Deserialize(r io.Reader) error {
	// tx deserialize
	err := tx.DeserializeUnsigned(r)
	if err != nil {
		return errors.New("transaction Deserialize error")
	}

	// tx program
	lens, err := serialization.ReadVarUint(r, 0)
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

func (tx *ElaTx) DeserializeUnsigned(r io.Reader) error {
	var txType [1]byte
	_, err := io.ReadFull(r, txType[:])
	if err != nil {
		return err
	}
	tx.TxType = TransactionType(txType[0])
	return tx.DeserializeUnsignedWithoutType(r)
}

func (tx *ElaTx) DeserializeUnsignedWithoutType(r io.Reader) error {
	var payloadVersion [1]byte
	_, err := io.ReadFull(r, payloadVersion[:])
	tx.PayloadVersion = payloadVersion[0]
	if err != nil {
		return err
	}
	tx.Payload = SideMiningPayload{}
	err = tx.Payload.Deserialize(r, tx.PayloadVersion)
	if err != nil {
		return errors.New("Payload Parse error")
	}
	//attributes
	Len, err := serialization.ReadVarUint(r, 0)
	if err != nil {
		return err
	}
	if Len > uint64(0) {
		for i := uint64(0); i < Len; i++ {
			attr := new(TxAttribute)
			err = attr.Deserialize(r)
			if err != nil {
				return err
			}
			tx.Attributes = append(tx.Attributes, attr)
		}
	}
	//UTXOInputs
	Len, err = serialization.ReadVarUint(r, 0)
	if err != nil {
		return err
	}
	if Len > uint64(0) {
		for i := uint64(0); i < Len; i++ {
			utxo := new(UTXOTxInput)
			err = utxo.Deserialize(r)
			if err != nil {
				return err
			}
			tx.UTXOInputs = append(tx.UTXOInputs, utxo)
		}
	}
	//TODO balanceInputs
	//Outputs
	Len, err = serialization.ReadVarUint(r, 0)
	if err != nil {
		return err
	}
	if Len > uint64(0) {
		for i := uint64(0); i < Len; i++ {
			output := new(TxOutput)
			err = output.Deserialize(r)
			if err != nil {
				return err
			}
			tx.Outputs = append(tx.Outputs, output)
		}
	}

	temp, err := serialization.ReadUint32(r)
	tx.LockTime = uint32(temp)
	if err != nil {
		return err
	}

	return nil
}

func (tx *ElaTx) Hash() common.Uint256 {
	buf := new(bytes.Buffer)
	tx.SerializeUnsigned(buf)

	temp := sha256.Sum256(buf.Bytes())
	f := sha256.Sum256(temp[:])
	hash := common.Uint256(f)

	return hash
}
