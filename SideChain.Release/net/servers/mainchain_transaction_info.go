package servers

import (
	. "Elastos.ELA.SideChain/common"
	"Elastos.ELA.SideChain/common/serialization"
	"Elastos.ELA.SideChain/core/contract/program"
	. "Elastos.ELA.SideChain/core/transaction"
	"Elastos.ELA.SideChain/core/transaction/payload"
	"bytes"
	"errors"
	"io"
)

func (i *CoinbaseInfo) Data(version byte) string {
	return ""
}

func (i *CoinbaseInfo) Serialize(w io.Writer, version byte) error {
	return nil
}

func (i *CoinbaseInfo) Deserialize(r io.Reader, version byte) error {
	return nil
}

func (i *IssueTokenInfo) Data(version byte) string {
	return i.Proof
}

func (i *IssueTokenInfo) Serialize(w io.Writer, version byte) error {
	if err := serialization.WriteVarString(w, i.Proof); err != nil {
		return errors.New("Transaction IssueTokenInfo serialization failed.")
	}

	return nil
}

func (i *IssueTokenInfo) Deserialize(r io.Reader, version byte) error {
	value, err := serialization.ReadVarString(r)
	if err != nil {
		return errors.New("Transaction IssueTokenInfo deserialization failed.")
	}
	i.Proof = value

	return nil
}

func (i RegisterAssetInfo) Data(version byte) string {
	return ""
}

func (i RegisterAssetInfo) Serialize(w io.Writer, version byte) error {
	return nil
}

func (i RegisterAssetInfo) Deserialize(r io.Reader, version byte) error {
	return nil
}

func (i TransferAssetInfo) Data(version byte) string {
	return ""
}

func (i TransferAssetInfo) Serialize(w io.Writer, version byte) error {
	return nil
}

func (i TransferAssetInfo) Deserialize(r io.Reader, version byte) error {
	return nil
}

func (a *TransferCrossChainAssetInfo) Data(version byte) string {
	return ""
}

func (a *TransferCrossChainAssetInfo) Serialize(w io.Writer, version byte) error {
	if a.AddressesMap == nil {
		return errors.New("Invalid publickey map")
	}

	if err := serialization.WriteVarUint(w, uint64(len(a.AddressesMap))); err != nil {
		return errors.New("publicKey map's length serialize failed")
	}

	for k, v := range a.AddressesMap {
		if err := serialization.WriteVarString(w, k); err != nil {
			return errors.New("address map's key serialize failed")
		}

		if err := serialization.WriteVarUint(w, v); err != nil {
			return errors.New("address map's value serialize failed")
		}
	}

	return nil
}

func (a *TransferCrossChainAssetInfo) Deserialize(r io.Reader, version byte) error {
	length, err := serialization.ReadVarUint(r, 0)
	if err != nil {
		return errors.New("address map's length deserialize failed")
	}

	a.AddressesMap = nil
	a.AddressesMap = make(map[string]uint64)
	for i := uint64(0); i < length; i++ {
		k, err := serialization.ReadVarString(r)
		if err != nil {
			return errors.New("address map's key deserialize failed")
		}

		v, err := serialization.ReadVarUint(r, 0)
		if err != nil {
			return errors.New("address map's value deserialize failed")
		}

		a.AddressesMap[k] = v
	}

	return nil
}

func (a *TxAttributeInfo) Serialize(w io.Writer) error {
	if err := serialization.WriteUint8(w, byte(a.Usage)); err != nil {
		return errors.New("Transaction attribute Usage serialization failed.")
	}
	if !IsValidAttributeType(a.Usage) {
		return errors.New("[TxAttribute] error: Unsupported attribute Description.")
	}
	if err := serialization.WriteVarString(w, a.Data); err != nil {
		return errors.New("Transaction attribute Data serialization failed.")
	}
	return nil
}

func (a *TxAttributeInfo) Deserialize(r io.Reader) error {
	usage, err := serialization.ReadBytes(r, 1)
	if err != nil {
		return errors.New("Transaction attribute Usage deserialization failed.")
	}
	a.Usage = TransactionAttributeUsage(usage[0])
	if !IsValidAttributeType(a.Usage) {
		return errors.New("[TxAttribute] error: Unsupported attribute Description.")
	}

	data, err := serialization.ReadVarString(r)
	if err != nil {
		return errors.New("Transaction attribute Data deserialization failed.")
	}
	a.Data = data

	return nil
}

func (u *UTXOTxInputInfo) Serialize(w io.Writer) error {
	if err := serialization.WriteVarString(w, u.ReferTxID); err != nil {
		return errors.New("Transaction UTXOTxInputInfo ReferTxID serialization failed.")
	}
	if err := serialization.WriteUint16(w, u.ReferTxOutputIndex); err != nil {
		return errors.New("Transaction UTXOTxInputInfo ReferTxOutputIndex serialization failed.")
	}
	if err := serialization.WriteUint32(w, u.Sequence); err != nil {
		return errors.New("Transaction UTXOTxInputInfo Sequence serialization failed.")
	}
	if err := serialization.WriteVarString(w, u.Address); err != nil {
		return errors.New("Transaction UTXOTxInputInfo Address serialization failed.")
	}
	if err := serialization.WriteVarString(w, u.Value); err != nil {
		return errors.New("Transaction UTXOTxInputInfo Value serialization failed.")
	}
	return nil
}

func (u *UTXOTxInputInfo) Deserialize(r io.Reader) error {
	refer, err := serialization.ReadVarString(r)
	if err != nil {
		return errors.New("Transaction UTXOTxInputInfo ReferTxID deserialization failed.")
	}
	u.ReferTxID = refer

	index, err := serialization.ReadUint16(r)
	if err != nil {
		return errors.New("Transaction UTXOTxInputInfo ReferTxOutputIndex deserialization failed.")
	}
	u.ReferTxOutputIndex = index

	sequence, err := serialization.ReadUint32(r)
	if err != nil {
		return errors.New("Transaction UTXOTxInputInfo Sequence deserialization failed.")
	}
	u.Sequence = sequence

	addr, err := serialization.ReadVarString(r)
	if err != nil {
		return errors.New("Transaction UTXOTxInputInfo Address deserialization failed.")
	}
	u.Address = addr

	value, err := serialization.ReadVarString(r)
	if err != nil {
		return errors.New("Transaction UTXOTxInputInfo Value deserialization failed.")
	}
	u.Value = value

	return nil
}

func (b *BalanceTxInputInfo) Serialize(w io.Writer) error {
	if err := serialization.WriteVarString(w, b.AssetID); err != nil {
		return errors.New("Transaction BalanceTxInputInfo AssetID serialization failed.")
	}
	if err := serialization.WriteVarUint(w, uint64(b.Value)); err != nil {
		return errors.New("Transaction BalanceTxInputInfo Value serialization failed.")
	}
	if err := serialization.WriteVarString(w, b.ProgramHash); err != nil {
		return errors.New("Transaction BalanceTxInputInfo ProgramHash serialization failed.")
	}
	return nil
}

func (b *BalanceTxInputInfo) Deserialize(r io.Reader) error {
	assetid, err := serialization.ReadVarString(r)
	if err != nil {
		return errors.New("Transaction BalanceTxInputInfo AssetID deserialization failed.")
	}
	b.AssetID = assetid

	value, err := serialization.ReadVarUint(r, 0)
	if err != nil {
		return errors.New("Transaction BalanceTxInputInfo Value deserialization failed.")
	}
	b.Value = Fixed64(value)

	programHash, err := serialization.ReadVarString(r)
	if err != nil {
		return errors.New("Transaction BalanceTxInputInfo ProgramHash deserialization failed.")
	}
	b.ProgramHash = programHash

	return nil
}

func (o *TxoutputInfo) Serialize(w io.Writer) error {
	if err := serialization.WriteVarString(w, o.AssetID); err != nil {
		return errors.New("Transaction TxoutputInfo AssetID serialization failed.")
	}
	if err := serialization.WriteVarString(w, o.Value); err != nil {
		return errors.New("Transaction TxoutputInfo Value serialization failed.")
	}
	if err := serialization.WriteVarString(w, o.Address); err != nil {
		return errors.New("Transaction TxoutputInfo Address serialization failed.")
	}
	if err := serialization.WriteUint32(w, o.OutputLock); err != nil {
		return errors.New("Transaction TxoutputInfo OutputLock serialization failed.")
	}

	return nil
}

func (b *TxoutputInfo) Deserialize(r io.Reader) error {
	assetid, err := serialization.ReadVarString(r)
	if err != nil {
		return errors.New("Transaction TxoutputInfo AssetID deserialization failed.")
	}
	b.AssetID = assetid

	value, err := serialization.ReadVarString(r)
	if err != nil {
		return errors.New("Transaction TxoutputInfo Value deserialization failed.")
	}
	b.Value = value

	addr, err := serialization.ReadVarString(r)
	if err != nil {
		return errors.New("Transaction TxoutputInfo Address deserialization failed.")
	}
	b.Address = addr

	lock, err := serialization.ReadUint32(r)
	if err != nil {
		return errors.New("Transaction TxoutputInfo OutputLock deserialization failed.")
	}
	b.OutputLock = lock

	return nil
}

func (p *ProgramInfo) Serialize(w io.Writer) error {
	if err := serialization.WriteVarString(w, p.Code); err != nil {
		return errors.New("Transaction ProgramInfo Code serialization failed.")
	}
	if err := serialization.WriteVarString(w, p.Parameter); err != nil {
		return errors.New("Transaction ProgramInfo Parameter serialization failed.")
	}
	return nil
}

func (p *ProgramInfo) Deserialize(r io.Reader) error {
	code, err := serialization.ReadVarString(r)
	if err != nil {
		return errors.New("Transaction ProgramInfo Code deserialization failed.")
	}
	p.Code = code

	param, err := serialization.ReadVarString(r)
	if err != nil {
		return errors.New("Transaction ProgramInfo Parameter deserialization failed.")
	}
	p.Parameter = param

	return nil
}

func (m *TxoutputMap) Serialize(w io.Writer) error {
	if _, err := m.Key.Serialize(w); err != nil {
		return errors.New("Transaction TxoutputMap Key serialization failed.")
	}
	if err := serialization.WriteVarUint(w, uint64(len(m.Txout))); err != nil {
		return errors.New("Transaction TxoutputMap Txout length serialization failed.")
	}
	for _, txout := range m.Txout {
		if err := txout.Serialize(w); err != nil {
			return err
		}
	}

	return nil
}

func (m *TxoutputMap) Deserialize(r io.Reader) error {
	if err := m.Key.Deserialize(r); err != nil {
		return errors.New("Transaction TxoutputMap Key deserialization failed.")
	}
	length, err := serialization.ReadVarUint(r, 0)
	if err != nil {
		return errors.New("Transaction TxoutputMap Txout length deserialization failed.")
	}
	for i := uint64(0); i < length; i++ {
		txout := TxoutputInfo{}
		if err = txout.Deserialize(r); err != nil {
			return err
		}
		m.Txout = append(m.Txout, txout)
	}
	return nil
}

func (m *AmountMap) Serialize(w io.Writer) error {
	if _, err := m.Key.Serialize(w); err != nil {
		return errors.New("Transaction AmountMap Key serialization failed.")
	}
	if err := serialization.WriteVarUint(w, uint64(m.Value)); err != nil {
		return errors.New("Transaction AmountMap Value serialization failed.")
	}
	return nil
}

func (m *AmountMap) Deserialize(r io.Reader) error {
	if err := m.Key.Deserialize(r); err != nil {
		return errors.New("Transaction AmountMap Key deserialization failed.")
	}
	val, err := serialization.ReadVarUint(r, 0)
	if err != nil {
		return errors.New("Transaction AmountMap Value deserialization failed.")
	}
	m.Value = Fixed64(val)
	return nil
}

func (t *Transactions) Serialize(w io.Writer) error {
	var err error
	//txType
	w.Write([]byte{byte(t.TxType)})
	//PayloadVersion
	w.Write([]byte{t.PayloadVersion})
	if t.Payload == nil {
		return errors.New("Transaction Payload is nil.")
	}
	//Serialize Payload
	t.Payload.Serialize(w, t.PayloadVersion)
	//[]TxAttributeInfo
	if err = serialization.WriteVarUint(w, uint64(len(t.Attributes))); err != nil {
		return errors.New("Transaction item txAttribute length serialization failed.")
	}
	for _, attr := range t.Attributes {
		if err = attr.Serialize(w); err != nil {
			return err
		}
	}
	//[]UTXOTxInputInfo
	if err = serialization.WriteVarUint(w, uint64(len(t.UTXOInputs))); err != nil {
		return errors.New("Transaction item UTXOInputs length serialization failed.")
	}
	for _, utxo := range t.UTXOInputs {
		if err = utxo.Serialize(w); err != nil {
			return err
		}
	}
	//[]BalanceTxInputInfo
	if err = serialization.WriteVarUint(w, uint64(len(t.BalanceInputs))); err != nil {
		return errors.New("Transaction item BalanceInputs length serialization failed.")
	}
	for _, balance := range t.BalanceInputs {
		if err = balance.Serialize(w); err != nil {
			return err
		}
	}
	//[]TxoutputInfo
	if err = serialization.WriteVarUint(w, uint64(len(t.Outputs))); err != nil {
		return errors.New("Transaction item BalanceInputs length serialization failed.")
	}
	for _, output := range t.Outputs {
		if err = output.Serialize(w); err != nil {
			return err
		}
	}
	//LockTime
	if err = serialization.WriteUint32(w, t.LockTime); err != nil {
		return errors.New("Transaction item LockTime length serialization failed.")
	}
	//[]ProgramInfo
	if err = serialization.WriteVarUint(w, uint64(len(t.Outputs))); err != nil {
		return errors.New("Transaction item ProgramInfo length serialization failed.")
	}
	for _, program := range t.Programs {
		if err = program.Serialize(w); err != nil {
			return err
		}
	}
	//[]TxoutputMap
	if err = serialization.WriteVarUint(w, uint64(len(t.AssetOutputs))); err != nil {
		return errors.New("Transaction item TxoutputMap length serialization failed.")
	}
	for _, m := range t.AssetOutputs {
		if err = m.Serialize(w); err != nil {
			return err
		}
	}
	//[]AmountMap
	if err = serialization.WriteVarUint(w, uint64(len(t.AssetInputAmount))); err != nil {
		return errors.New("Transaction item AssetInputAmount length serialization failed.")
	}
	for _, m := range t.AssetInputAmount {
		if err = m.Serialize(w); err != nil {
			return err
		}
	}
	//[]AmountMap
	if err = serialization.WriteVarUint(w, uint64(len(t.AssetOutputAmount))); err != nil {
		return errors.New("Transaction item AssetOutputAmount length serialization failed.")
	}
	for _, m := range t.AssetOutputAmount {
		if err = m.Serialize(w); err != nil {
			return err
		}
	}
	//Timestamp uint32 `json:",omitempty"`
	if err = serialization.WriteUint32(w, t.Timestamp); err != nil {
		return errors.New("Transaction item Timestamp serialization failed.")
	}
	//Confirminations uint32 `json:",omitempty"`
	if err = serialization.WriteUint32(w, t.Confirmations); err != nil {
		return errors.New("Transaction item Confirminations serialization failed.")
	}
	//TxSize uint32 `json:",omitempty"`
	if err = serialization.WriteUint32(w, t.TxSize); err != nil {
		return errors.New("Transaction item TxSize serialization failed.")
	}
	//Hash string
	if err = serialization.WriteVarString(w, t.Hash); err != nil {
		return errors.New("Transaction item Hash serialization failed.")
	}

	return nil
}

func (t *Transactions) Deserialize(r io.Reader) error {
	var err error
	tmpByte := make([]byte, 1)
	//txType
	if _, err = io.ReadFull(r, tmpByte); err != nil {
		return errors.New("Transaction type deserialize failed.")
	}
	t.TxType = TransactionType(tmpByte[0])
	switch t.TxType {
	case RegisterAsset:
		t.Payload = new(RegisterAssetInfo)
	case TransferAsset:
		t.Payload = new(TransferAssetInfo)
	case IssueToken:
		t.Payload = new(IssueTokenInfo)
	case TransferCrossChainAsset:
		t.Payload = new(TransferCrossChainAssetInfo)
	default:
		return errors.New("Invalid transaction type.")
	}
	//PayloadVersion
	if _, err = io.ReadFull(r, tmpByte); err != nil {
		return err
	}
	t.PayloadVersion = tmpByte[0]
	//Payload
	if err = t.Payload.Deserialize(r, t.PayloadVersion); err != nil {
		return err
	}
	//Attributes     []TxAttributeInfo
	length, err := serialization.ReadVarUint(r, 0)
	if err != nil {
		return errors.New("Attributes length deserialize failed.")
	}
	for i := uint64(0); i < length; i++ {
		attr := TxAttributeInfo{}
		if err = attr.Deserialize(r); err != nil {
			return err
		}
		t.Attributes = append(t.Attributes, attr)
	}
	//UTXOInputs     []UTXOTxInputInfo
	if length, err = serialization.ReadVarUint(r, 0); err != nil {
		return errors.New("UTXOInputs length deserialize failed.")
	}
	for i := uint64(0); i < length; i++ {
		utxo := UTXOTxInputInfo{}
		if err = utxo.Deserialize(r); err != nil {
			return err
		}
		t.UTXOInputs = append(t.UTXOInputs, utxo)
	}
	//BalanceInputs  []BalanceTxInputInfo
	if length, err = serialization.ReadVarUint(r, 0); err != nil {
		return errors.New("BalanceInputs length deserialize failed.")
	}
	for i := uint64(0); i < length; i++ {
		balance := BalanceTxInputInfo{}
		if err = balance.Deserialize(r); err != nil {
			return err
		}
		t.BalanceInputs = append(t.BalanceInputs, balance)
	}
	//Outputs        []TxoutputInfo
	if length, err = serialization.ReadVarUint(r, 0); err != nil {
		return errors.New("TxoutputInfo length deserialize failed.")
	}
	for i := uint64(0); i < length; i++ {
		output := TxoutputInfo{}
		if err = output.Deserialize(r); err != nil {
			return err
		}
		t.Outputs = append(t.Outputs, output)
	}
	//LockTime       uint32
	temp, err := serialization.ReadUint32(r)
	if err != nil {
		return errors.New("LockTime deserialize failed.")
	}
	t.LockTime = temp
	//Programs       []ProgramInfo
	if length, err = serialization.ReadVarUint(r, 0); err != nil {
		return errors.New("Programinfo length deserialize failed.")
	}
	for i := uint64(0); i < length; i++ {
		program := ProgramInfo{}
		if err = program.Deserialize(r); err != nil {
			return err
		}
		t.Programs = append(t.Programs, program)
	}
	//AssetOutputs      []TxoutputMap
	if length, err = serialization.ReadVarUint(r, 0); err != nil {
		return errors.New("AssetOutputs length deserialize failed.")
	}
	for i := uint64(0); i < length; i++ {
		output := TxoutputMap{}
		if err = output.Deserialize(r); err != nil {
			return err
		}
		t.AssetOutputs = append(t.AssetOutputs, output)
	}
	//AssetInputAmount  []AmountMap
	if length, err = serialization.ReadVarUint(r, 0); err != nil {
		return errors.New("AssetInputAmount length deserialize failed.")
	}
	for i := uint64(0); i < length; i++ {
		amount := AmountMap{}
		if err = amount.Deserialize(r); err != nil {
			return err
		}
		t.AssetInputAmount = append(t.AssetInputAmount, amount)
	}
	//AssetOutputAmount []AmountMap
	if length, err = serialization.ReadVarUint(r, 0); err != nil {
		return errors.New("AssetOutputAmount length deserialize failed.")
	}
	for i := uint64(0); i < length; i++ {
		amount := AmountMap{}
		if err = amount.Deserialize(r); err != nil {
			return err
		}
		t.AssetOutputAmount = append(t.AssetOutputAmount, amount)
	}
	//Timestamp uint32 `json:",omitempty"`
	timestamp, err := serialization.ReadUint32(r)
	if err != nil {
		return errors.New("Timestamp deserialize failed.")
	}
	t.Timestamp = timestamp
	//Confirminations uint32 `json:",omitempty"`
	confirm, err := serialization.ReadUint32(r)
	if err != nil {
		return errors.New("Confirminations deserialize failed.")
	}
	t.Confirmations = confirm
	//TxSize uint32 `json:",omitempty"`
	txSize, err := serialization.ReadUint32(r)
	if err != nil {
		return errors.New("TxSize deserialize failed.")
	}
	t.TxSize = txSize
	//Hash string
	hash, err := serialization.ReadVarString(r)
	if err != nil {
		return errors.New("Hash deserialize failed.")
	}
	t.Hash = hash

	return nil
}

func (trans *Transactions) ConvertFrom(txn *Transaction) error {
	return nil
}

func (trans *Transactions) ConvertTo() (*Transaction, error) {
	return nil, nil
}

func PayloadInfoToTransPayload(p PayloadInfo) (Payload, error) {
	switch object := p.(type) {
	case *RegisterAssetInfo:
		obj := new(payload.RegisterAsset)
		obj.Asset = object.Asset
		amount, err := StringToFixed64(object.Amount)
		if err != nil {
			return nil, err
		}
		obj.Amount = *amount
		bytes, err := HexStringToBytes(object.Controller)
		if err != nil {
			return nil, err
		}
		controller, err := Uint168FromBytes(bytes)
		obj.Controller = controller
		return obj, nil
	case *TransferAssetInfo:
		return new(payload.TransferAsset), nil
	case *IssueTokenInfo:
		obj := new(payload.IssueToken)
		proofBytes, err := HexStringToBytes(object.Proof)
		if err != nil {
			return nil, err
		}
		err = obj.Proof.Deserialize(bytes.NewReader(proofBytes))
		if err != nil {
			return nil, err
		}
		return obj, nil
	case *TransferCrossChainAssetInfo:
		obj := new(payload.TransferCrossChainAsset)
		obj.AddressesMap = object.AddressesMap
		return obj, nil
	}

	return nil, errors.New("Invalid payload type.")
}

func (txinfo *Transactions) ToTransaction() (*Transaction, error) {

	txPaload, err := PayloadInfoToTransPayload(txinfo.Payload)
	if err != nil {
		return nil, err
	}

	var txAttribute []*TxAttribute
	for _, att := range txinfo.Attributes {
		attData, err := HexStringToBytes(att.Data)
		if err != nil {
			return nil, err
		}
		txAttr := &TxAttribute{
			Usage: att.Usage,
			Data:  attData,
			Size:  0,
		}
		txAttribute = append(txAttribute, txAttr)
	}

	var txUTXOTxInput []*UTXOTxInput
	for _, input := range txinfo.UTXOInputs {
		txID, err := HexStringToBytes(input.ReferTxID)
		if err != nil {
			return nil, err
		}
		referID, err := Uint256ParseFromBytes(txID)
		if err != nil {
			return nil, err
		}
		utxoInput := &UTXOTxInput{
			ReferTxID:          referID,
			ReferTxOutputIndex: input.ReferTxOutputIndex,
			Sequence:           input.Sequence,
		}
		txUTXOTxInput = append(txUTXOTxInput, utxoInput)
	}

	var txOutputs []*TxOutput
	for _, output := range txinfo.Outputs {
		assetIdBytes, err := HexStringToBytes(output.AssetID)
		if err != nil {
			return nil, err
		}
		assetId, err := Uint256ParseFromBytes(assetIdBytes)
		if err != nil {
			return nil, err
		}
		value, err := StringToFixed64(output.Value)
		if err != nil {
			return nil, err
		}
		programHash, err := Uint168FromAddress(output.Address)
		if err != nil {
			return nil, err
		}
		output := &TxOutput{
			AssetID:     assetId,
			Value:       *value,
			OutputLock:  output.OutputLock,
			ProgramHash: programHash,
		}
		txOutputs = append(txOutputs, output)
	}

	var txPrograms []*program.Program
	for _, pgrm := range txinfo.Programs {
		code, err := HexStringToBytes(pgrm.Code)
		if err != nil {
			return nil, err
		}
		parameter, err := HexStringToBytes(pgrm.Parameter)
		if err != nil {
			return nil, err
		}
		txProgram := &program.Program{
			Code:      code,
			Parameter: parameter,
		}
		txPrograms = append(txPrograms, txProgram)
	}

	txTransaction := &Transaction{
		TxType:         txinfo.TxType,
		PayloadVersion: txinfo.PayloadVersion,
		Payload:        txPaload,
		Attributes:     txAttribute,
		UTXOInputs:     txUTXOTxInput,
		Outputs:        txOutputs,
		Programs:       txPrograms,
	}

	return txTransaction, nil
}
