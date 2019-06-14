package service

import (
	"math/big"
	"errors"
	"io"
	"bytes"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"

	st "github.com/elastos/Elastos.ELA.SideChain/types"
	"github.com/elastos/Elastos.ELA.SideChain/database"
	"github.com/elastos/Elastos.ELA.SideChain/events"
	"github.com/elastos/Elastos.ELA.SideChain/interfaces"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/contract"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/contract/states"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/smartcontract/enumerators"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/datatype"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/types"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/event"
	nc "github.com/elastos/Elastos.ELA.SideChain.NeoVM/common"
)

type StateReader struct {
	serviceMap map[string]func(engine *avm.ExecutionEngine) bool
	notifications []NotifyEventArgs
}

func NewStateReader() *StateReader {
	var stateReader StateReader

	stateReader.serviceMap = make(map[string]func(*avm.ExecutionEngine) bool, 0)
	stateReader.notifications = make([]NotifyEventArgs, 0)

	stateReader.Register("Neo.Runtime.GetTrigger", stateReader.RuntimeGetTrigger)
	stateReader.Register("Neo.Runtime.CheckWitness", stateReader.RuntimeCheckWitness)
	stateReader.Register("Neo.Runtime.Notify", stateReader.RuntimeNotify)
	stateReader.Register("Neo.Runtime.Log", stateReader.RuntimeLog)
	stateReader.Register("Neo.Runtime.GetTime", stateReader.RuntimeGetTime)
	stateReader.Register("Neo.Runtime.Serialize", stateReader.RuntimeSerialize)
	stateReader.Register("Neo.Runtime.Deserialize", stateReader.RuntimeDerialize)

	stateReader.Register("Neo.Blockchain.GetHeight", stateReader.BlockChainGetHeight)
	stateReader.Register("Neo.Blockchain.GetHeader", stateReader.BlockChainGetHeader)
	stateReader.Register("Neo.Blockchain.GetBlock", stateReader.BlockChainGetBlock)
	stateReader.Register("Neo.Blockchain.GetTransaction", stateReader.BlockChainGetTransaction)
	stateReader.Register("Neo.Blockchain.GetTransactionHeight", stateReader.BlockchainGetTransactionHeight)
	stateReader.Register("Neo.Blockchain.GetAccount", stateReader.BlockChainGetAccount)
	stateReader.Register("Neo.Blockchain.GetValidators", stateReader.BlockChainGetValidators)
	stateReader.Register("Neo.Blockchain.GetAsset", stateReader.BlockChainGetAsset)

	stateReader.Register("Neo.Header.GetIndex", stateReader.HeaderGetHeight)
	stateReader.Register("Neo.Header.GetHash", stateReader.HeaderGetHash)
	stateReader.Register("Neo.Header.GetVersion", stateReader.HeaderGetVersion)
	stateReader.Register("Neo.Header.GetPrevHash", stateReader.HeaderGetPrevHash)
	stateReader.Register("Neo.Header.GetMerkleRoot", stateReader.HeaderGetMerkleRoot)
	stateReader.Register("Neo.Header.GetTimestamp", stateReader.HeaderGetTimestamp)
	stateReader.Register("Neo.Header.GetConsensusData", stateReader.HeaderGetConsensusData)
	stateReader.Register("Neo.Header.GetNextConsensus", stateReader.HeaderGetNextConsensus)

	stateReader.Register("Neo.Block.GetTransactionCount", stateReader.BlockgetTransactionCount)
	stateReader.Register("Neo.Block.GetTransactions", stateReader.BlockGetTransactions)
	stateReader.Register("Neo.Block.GetTransaction", stateReader.BlockGetTransaction)

	stateReader.Register("Neo.Transaction.GetHash", stateReader.TransactionGetHash)
	stateReader.Register("Neo.Transaction.GetType", stateReader.TransactionGetType)
	stateReader.Register("Neo.Transaction.GetAttributes", stateReader.TransactionGetAttributes)
	stateReader.Register("Neo.Transaction.GetInputs", stateReader.TransactionGetInputs)
	stateReader.Register("Neo.Transaction.GetOutputs", stateReader.TransactionGetOutputs)
	stateReader.Register("Neo.Transaction.GetReferences", stateReader.TransactionGetReferences)
	stateReader.Register("Neo.Transaction.GetUnspentCoins", stateReader.TransactionGetUnspentCoins)
	stateReader.Register("Neo.InvocationTransaction.GetScript", stateReader.InvocationTransactionGetScript)

	stateReader.Register("Neo.Attribute.GetUsage", stateReader.AttributeGetUsage)
	stateReader.Register("Neo.Attribute.GetData", stateReader.AttributeGetData)

	stateReader.Register("Neo.Input.GetHash", stateReader.InputGetHash)
	stateReader.Register("Neo.Input.GetIndex", stateReader.InputGetIndex)

	stateReader.Register("Neo.Output.GetAssetId", stateReader.OutputGetAssetId)
	stateReader.Register("Neo.Output.GetValue", stateReader.OutputGetValue)
	stateReader.Register("Neo.Output.GetScriptHash", stateReader.OutputGetCodeHash)

	stateReader.Register("Neo.Account.GetScriptHash", stateReader.AccountGetCodeHash)
	stateReader.Register("Neo.Account.GetBalance", stateReader.AccountGetBalance)
	stateReader.Register("Neo.Account.GetVotes", stateReader.AccountGetVotes)

	stateReader.Register("Neo.Asset.GetAssetId", stateReader.AssetGetAssetId)
	stateReader.Register("Neo.Asset.GetAssetType", stateReader.AssetGetAssetType)
	stateReader.Register("Neo.Asset.GetAmount", stateReader.AssetGetAmount)
	stateReader.Register("Neo.Asset.GetAvailable", stateReader.AssetGetAvailable)
	stateReader.Register("Neo.Asset.GetPrecision", stateReader.AssetGetPrecision)
	stateReader.Register("Neo.Asset.GetOwner", stateReader.AssetGetOwner)
	stateReader.Register("Neo.Asset.GetAdmin", stateReader.AssetGetAdmin)
	stateReader.Register("Neo.Asset.GetIssuer", stateReader.AssetGetIssuer)

	stateReader.Register("Neo.Contract.GetScript", stateReader.ContractGetCode)
	stateReader.Register("Neo.Contract.IsPayable", stateReader.ContractIsPayable)

	stateReader.Register("Neo.Storage.GetContext", stateReader.StorageGetContext)
	stateReader.Register("Neo.Storage.GetReadOnlyContext", stateReader.StorageGetReadOnlyContext)
	stateReader.Register("Neo.StorageContext.AsReadOnly", stateReader.StorageContextAsReadOnly)

	stateReader.Register("Neo.Iterator.Key", stateReader.IteratorKey)
	stateReader.Register("Neo.Iterator.Next", stateReader.EnumeratorNext)
	stateReader.Register("Neo.Iterator.Value", stateReader.EnumeratorValue)
	stateReader.Register("Neo.Iterator.Keys", stateReader.IteratorKeys)
	stateReader.Register("Neo.Iterator.Values", stateReader.IteratorValues)


	return &stateReader
}

func (s *StateReader) Register(methodName string, handler func(engine *avm.ExecutionEngine) bool) bool {
	if _, ok := s.serviceMap[methodName]; ok {
		return false
	}
	s.serviceMap[methodName] = handler
	return true
}

func (s *StateReader) GetServiceMap() map[string]func(*avm.ExecutionEngine) bool {
	return s.serviceMap
}

func (s *StateReader) GetNotifyEvents() []NotifyEventArgs{
	return s.notifications
}

func (s *StateReader) RuntimeGetTrigger(e *avm.ExecutionEngine) bool {
	avm.PushData(e, int(e.GetTrigger()))
	return true
}

func (s *StateReader) RuntimeNotify(e *avm.ExecutionEngine) bool {
	item := avm.PopStackItem(e)
	events.Notify(event.ETRunTimeNotify, item)
	scriptHash, err := common.Uint168FromBytes(e.CurrentContext().GetCodeHash())
	if err != nil {
		return false
	}
	notify := NotifyEventArgs{
		e.GetDataContainer(),
		*scriptHash,
		item,
	}
	s.notifications = append(s.notifications, notify)
	return true
}

func (s *StateReader) RuntimeLog(e *avm.ExecutionEngine) bool {
	data := avm.PopStackItem(e)
	events.Notify(event.ETRunTimeLog, data)
	return true
}

func (s *StateReader) RuntimeGetTime(e *avm.ExecutionEngine) bool {
	if blockchain.DefaultChain == nil {
		return false
	}
	timeStamp := blockchain.DefaultChain.BestChain.Timestamp
	avm.PushData(e, timeStamp)
	return true
}

func (s *StateReader) RuntimeSerialize(e *avm.ExecutionEngine) bool {
	buf := new(bytes.Buffer)
	item := avm.PopStackItem(e)
	s.SerializeStackItem(item, buf)
	avm.PushData(e, buf.Bytes())
	return true
}

func (s *StateReader) RuntimeDerialize (e *avm.ExecutionEngine) bool {
	data := avm.PopStackItem(e).GetByteArray()
	reader := bytes.NewReader(data)
	item, err := s.DerializeStackItem(reader)
	if err != nil {
		return false
	}
	avm.PushData(e, item)
	return true
}

func (s *StateReader) DerializeStackItem(r io.Reader) (datatype.StackItem, error) {
	var itemType = make([]byte, 1)
	_, err := r.Read(itemType)
	if err != nil {
		return nil, err
	}

	switch (datatype.StackItemType(itemType[0])) {
	case datatype.TYPE_ByteArray:
		bytes, err := common.ReadVarBytes(r, avm.MaxItemSize, "StateReader DerializeStackItem TYPE_ByteArray")
		if err != nil {
			return nil, err
		}
		return datatype.NewByteArray(bytes), nil
	case datatype.TYPE_Boolean:
		data, err := common.ReadUint8(r)
		if err != nil {
			return nil, err
		}
		bytes := true
		if data == 0 {
			bytes = false
		}
		if err != nil {
			return nil, err
		}
		return datatype.NewBoolean(bytes), nil
	case datatype.TYPE_Integer:
		bytes, err := common.ReadVarBytes(r, avm.MAX_BIGINTEGER, "StateReader DerializeStackItem TYPE_Integer")
		if err != nil {
			return nil, err
		}
		bint := new(big.Int)
		bint.SetBytes(bytes)
		return datatype.NewInteger(bint), nil
	case datatype.TYPE_Array, datatype.TYPE_Struct:
		len, err := common.ReadVarUint(r, 0x00)
		if err != nil {
			return nil, err
		}
		var items = make([]datatype.StackItem, len)
		for i := 0; i < int(len); i++ {
			items[i], err = s.DerializeStackItem(r)
			if err != nil {
				return nil, err
			}
		}
		return datatype.NewArray(items), nil
	case datatype.TYPE_Map:
		dictionary := datatype.NewDictionary()
		len, err := common.ReadVarUint(r, 0x00)
		if err != nil {
			return nil, err
		}
		for i := 0; i < int(len); i++ {
			key, err := s.DerializeStackItem(r)
			if err != nil {
				return nil, err
			}
			value, err := s.DerializeStackItem(r)
			if err != nil {
				return nil, err
			}
			dictionary.PutStackItem(key, value)
		}
		return dictionary, nil
	}
	return nil, errors.New("error type")
}

func (s *StateReader) SerializeStackItem(item datatype.StackItem, w io.Writer) {
	switch item.(type) {
	case *datatype.Boolean:
		w.Write([]byte{byte(datatype.TYPE_Boolean)})
		w.Write(item.GetByteArray())
	case *datatype.Integer:
		w.Write([]byte{byte(datatype.TYPE_Integer)})
		common.WriteVarBytes(w, item.GetBigInteger().Bytes())
	case *datatype.ByteArray:
		w.Write([]byte{byte(datatype.TYPE_ByteArray)})
		common.WriteVarBytes(w, item.GetByteArray())
	case *datatype.GeneralInterface:
		w.Write([]byte{byte(datatype.TYPE_InteropInterface)})
		w.Write(item.GetByteArray())
	case *datatype.Array:
		w.Write([]byte{byte(datatype.TYPE_Array)})
		items := item.GetArray()
		common.WriteVarUint(w, (uint64(len(items))))
		for i := 0; i < len(items); i++ {
			s.SerializeStackItem(items[i], w)
		}
	case *datatype.Dictionary:
		dict := item.(*datatype.Dictionary)
		w.Write([]byte{byte(datatype.TYPE_Map)})
		dictMap := dict.GetMap()
		common.WriteVarUint(w, (uint64(len(dictMap))))
		for key := range dictMap {
			s.SerializeStackItem(key, w)
			s.SerializeStackItem(dictMap[key], w)
		}
	}
}

func (s *StateReader) CheckWitnessHash160(engine *avm.ExecutionEngine, programHash []byte) (bool, error) {
	if engine.GetDataContainer() == nil {
		return false, errors.New("CheckWitnessHash getDataContainer is null")
	}

	if len(programHash) != 20 {
		return false, errors.New("CheckWitnessHash programHash length is not 20")
	}
	hash, _ := common.Uint160FromBytes(programHash)

	tx := engine.GetDataContainer().(*st.Transaction)
	hashForVerify, err := blockchain.DefaultChain.Validator.TxProgramHashes(tx)
	if err != nil {
		return false, err
	}

	for _, v := range hashForVerify {
		if v.ToCodeHash().IsEqual(hash) {
			return true, nil
		}
	}

	return false, errors.New("can't find programhash" + common.BytesToHexString(programHash))
}

func (s *StateReader) CheckWitnessHash(engine *avm.ExecutionEngine, programHash common.Uint168) (bool, error) {
	if engine.GetDataContainer() == nil {
		return false, errors.New("CheckWitnessHash getDataContainer is null")
	}
	tx := engine.GetDataContainer().(*st.Transaction)
	hashForVerify, err := blockchain.DefaultChain.Validator.TxProgramHashes(tx)
	if err != nil {
		return false, err
	}
	ret := contains(hashForVerify, programHash)
	if !ret {
		return ret, errors.New("can't find programhash" + programHash.String())
	}
	return ret, nil
}

func (s *StateReader) CheckWitnessPublicKey(engine *avm.ExecutionEngine, publicKey *crypto.PublicKey) (bool, error) {
	c, err := contract.CreateSignatureRedeemScript(publicKey)
	if err != nil {
		return false, err
	}
	h, err := nc.ToProgramHash(c)
	if err != nil {
		return false, err
	}
	return s.CheckWitnessHash(engine, *h)
}

func (s *StateReader) RuntimeCheckWitness(e *avm.ExecutionEngine) bool {
	data := avm.PopByteArray(e)
	var (
		result bool
		err    error
	)
	if len(data) == 21 {
		program, err := common.Uint168FromBytes(data)
		if err != nil {
			return false
		}
		result, err = s.CheckWitnessHash(e, *program)
	} else if len(data) == 20 {
		result, err = s.CheckWitnessHash160(e, data)
	} else if len(data) == 33 {
		publickKey, err := crypto.DecodePoint(data)
		if err != nil {
			return false
		}
		result, err = s.CheckWitnessPublicKey(e, publickKey)
	} else {
		return false
	}
	if err != nil {
		log.Error(err)
		return false
	}
	avm.PushData(e, result)
	return true
}

func (s *StateReader) BlockChainGetHeight(e *avm.ExecutionEngine) bool {
	var i uint32 = 0
	if blockchain.DefaultChain != nil {
		i = blockchain.DefaultChain.BestChain.Height
	}
	avm.PushData(e, i)
	return true
}

func (s *StateReader) BlockChainGetHeader(e *avm.ExecutionEngine) bool {
	var (
		header interfaces.Header
		err    error
	)
	data := avm.PopByteArray(e)
	l := len(data)

	if l <= 5 {
		b := new(big.Int)
		height := b.SetBytes(common.BytesReverse(data)).Int64()
		if blockchain.DefaultChain != nil {
			hash, err := blockchain.DefaultChain.GetBlockHash((uint32(height)))
			if err != nil {
				return false
			}
			header, err = blockchain.DefaultChain.GetHeader(hash)
		}
	} else if l == 32 {
		hash, _ := common.Uint256FromBytes(data)
		if blockchain.DefaultChain != nil {
			header, err = blockchain.DefaultChain.GetHeader(*hash)
		}
	} else {
		return false
	}
	if err != nil {
		return false
	}
	avm.PushData(e, header)
	return true
}

func (s *StateReader) BlockChainGetBlock(e *avm.ExecutionEngine) bool {
	data := avm.PopByteArray(e)
	var (
		block *st.Block
		err   error
	)
	l := len(data)
	if l <= 5 {
		b := new(big.Int)
		height := uint32(b.SetBytes(common.BytesReverse(data)).Int64())
		if blockchain.DefaultChain != nil {
			hash, err := blockchain.DefaultChain.GetBlockHash(height)
			if err != nil {
				return false
			}
			block, err = blockchain.DefaultChain.GetBlockByHash(hash)
		}
	} else if l == 32 {
		hash, err := common.Uint256FromBytes(data)
		if err != nil {
			return false
		}
		if blockchain.DefaultChain != nil {
			block, err = blockchain.DefaultChain.GetBlockByHash(*hash)
		}
	} else {
		return false
	}
	if err != nil {
		return false
	}
	avm.PushData(e, block)
	return true
}

func (s *StateReader) BlockChainGetTransaction(e *avm.ExecutionEngine) bool {
	d := avm.PopByteArray(e)
	hash, err := common.Uint256FromBytes(d)
	if err != nil {
		return false
	}
	if blockchain.DefaultChain != nil {
		tx, _, err := blockchain.DefaultChain.GetTransaction(*hash)
		if err != nil {
			return false
		}
		avm.PushData(e, tx)
	}
	return true
}

func (s *StateReader) BlockchainGetTransactionHeight(e *avm.ExecutionEngine) bool {
	d := avm.PopByteArray(e)
	hash, err := common.Uint256FromBytes(d)
	if err != nil {
		return false
	}
	if blockchain.DefaultChain != nil {
		_, height, err := blockchain.DefaultChain.GetTransaction(*hash)
		if err != nil {
			return false
		}
		avm.PushData(e, height)
	}
	return true
}

func (s *StateReader) BlockChainGetAccount(e *avm.ExecutionEngine) bool {
	d := avm.PopByteArray(e)
	hash, err := common.Uint168FromBytes(d)
	if err != nil {
		return false
	}

	if blockchain.DefaultChain != nil {
		account, err := blockchain.DefaultChain.Store.GetAccount(hash)
		if err != nil {
			return false
		}
		avm.PushData(e, account)
	}
	return true
}

func (s *StateReader) BlockChainGetValidators(e *avm.ExecutionEngine) bool {
	//note ela chain is not have Validators data. because consensus is pow
	pkList := make([]datatype.StackItem, 0)
	avm.PushData(e, pkList)
	return true
}


func (s *StateReader) BlockChainGetAsset(e *avm.ExecutionEngine) bool {
	d := avm.PopByteArray(e)
	hash, err := common.Uint256FromBytes(d)
	if err != nil {
		return false
	}

	asset, err := blockchain.DefaultChain.GetAsset(*hash)
	if err != nil {
		return false
	}
	avm.PushData(e, asset)
	return true
}

func (s *StateReader) HeaderGetHeight(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}

	var height uint32
	switch d.(type) {
	case interfaces.Header:
		height = d.(interfaces.Header).GetHeight()
	case *st.Block:
		height = d.(*st.Block).Header.GetHeight()
	default:
		return false
	}

	avm.PushData(e, height)
	return true
}

func (s *StateReader) HeaderGetHash(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}
	var hash common.Uint256
	switch d.(type) {
	case interfaces.Header:
		hash = d.(interfaces.Header).Hash()
	case *st.Block:
		hash = d.(*st.Block).Header.Hash()
	default:
		return false
	}
	avm.PushData(e, hash.Bytes())
	return true
}

func (s *StateReader) HeaderGetVersion(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}
	var version uint32
	switch d.(type) {
	case interfaces.Header:
		version = d.(interfaces.Header).GetVersion()
	case *st.Block:
		version = d.(*st.Block).Header.GetVersion()
	default:
		return false
	}

	avm.PushData(e, version)
	return true
}

func (s *StateReader) HeaderGetPrevHash(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}

	var preHash common.Uint256
	switch d.(type) {
	case interfaces.Header:
		preHash = d.(interfaces.Header).GetPrevious()
	case *st.Block:
		preHash = d.(*st.Block).Header.GetPrevious()
	default:
		return false
	}

	avm.PushData(e, preHash.Bytes())
	return true
}

func (s *StateReader) HeaderGetMerkleRoot(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}

	var root common.Uint256
	switch d.(type) {
	case interfaces.Header:
		root = d.(interfaces.Header).GetMerkleRoot()
	case *st.Block:
		root = d.(*st.Block).Header.GetMerkleRoot()
	default:
		return false
	}

	avm.PushData(e, root.Bytes())
	return true
}

func (s *StateReader) HeaderGetTimestamp(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}

	var timeStamp uint32
	switch d.(type) {
	case interfaces.Header:
		timeStamp = d.(interfaces.Header).GetTimeStamp()
	case *st.Block:
		timeStamp = d.(*st.Block).Header.GetTimeStamp()
	default:
		return false
	}

	avm.PushData(e, timeStamp)
	return true
}

func (s *StateReader) HeaderGetConsensusData(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}

	var consensusData uint32
	switch d.(type) {
	case interfaces.Header:
		consensusData = d.(interfaces.Header).GetAuxPow().MainBlockHeader.AuxPow.ParBlockHeader.Nonce
	case *st.Block:
		consensusData = d.(*st.Block).Header.GetAuxPow().MainBlockHeader.AuxPow.ParBlockHeader.Nonce
	default:
		return false
	}

	avm.PushData(e, consensusData)
	return true
}

func (s *StateReader) HeaderGetNextConsensus(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}
	//note ela chain is not have NextConsensus data. because consensus is pow
	avm.PushData(e, 0)
	return true
}

func (s *StateReader) BlockgetTransactionCount(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}
	transactions := d.(*st.Block).Transactions
	avm.PushData(e, len(transactions))
	return true
}

func (s *StateReader) BlockGetTransactions(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}
	transactions := d.(*st.Block).Transactions
	list := make([]datatype.StackItem, 0)
	for _, v := range transactions {
		list = append(list, datatype.NewGeneralInterface(v))
	}

	avm.PushData(e, datatype.NewArray(list))
	return true
}

func (s *StateReader) BlockGetTransaction(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}
	index := avm.PopInt(e)
	if index < 0 {
		return false
	}
	transactions := d.(*st.Block).Transactions
	if index >= len(transactions) {
		return false
	}
	avm.PushData(e, transactions[index])
	return true
}

func (s *StateReader) TransactionGetHash(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}
	txHash := d.(*st.Transaction).Hash()
	avm.PushData(e, txHash.Bytes())
	return true
}

func (s *StateReader) TransactionGetType(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}
	txType := d.(*st.Transaction).TxType
	avm.PushData(e, int(txType))
	return true
}

func (s *StateReader) TransactionGetAttributes(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}
	attributes := d.(*st.Transaction).Attributes
	list := make([]datatype.StackItem, 0)
	for _, v := range attributes {
		list = append(list, datatype.NewGeneralInterface(v))
	}
	avm.PushData(e, list)
	return true
}

func (s *StateReader) TransactionGetInputs(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}
	inputs := d.(*st.Transaction).Inputs
	list := make([]datatype.StackItem, 0)
	for _, v := range inputs {
		list = append(list, datatype.NewGeneralInterface(v))
	}
	avm.PushData(e, list)
	return true
}

func (s *StateReader) TransactionGetOutputs(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}
	outputs := d.(*st.Transaction).Outputs
	list := make([]datatype.StackItem, 0)
	for _, v := range outputs {
		list = append(list, datatype.NewGeneralInterface(v))
	}
	avm.PushData(e, list)
	return true
}

func (s *StateReader) TransactionGetReferences(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}

	references, err := blockchain.DefaultChain.Store.GetTxReference(d.(*st.Transaction))
	if err != nil {
		return false
	}
	list := make([]datatype.StackItem, 0)
	for _, v := range references {
		list = append(list, datatype.NewGeneralInterface(v))
	}
	avm.PushData(e, list)
	return true
}

func (s *StateReader) TransactionGetUnspentCoins(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}
	tx := d.(*st.Transaction)
	unspentCoins, err := blockchain.DefaultChain.Store.GetUnspents(tx.Hash())
	if err != nil {
		return false
	}

	list := make([]datatype.StackItem, 0)
	for _, v := range unspentCoins {
		list = append(list, datatype.NewGeneralInterface(v))
	}
	avm.PushData(e, list)
	return true
}

func (s *StateReader) InvocationTransactionGetScript(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}
	txtype := d.(*st.Transaction).TxType
	if txtype != st.Invoke{
		return false
	}
	payload := d.(*st.Transaction).Payload
	script := payload.(*types.PayloadInvoke).Code
	avm.PushData(e, script)
	return true
}

func (s *StateReader) AttributeGetUsage(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}
	attribute := d.(*st.Attribute)
	avm.PushData(e, int(attribute.Usage))
	return true
}

func (s *StateReader) AttributeGetData(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}
	attribute := d.(*st.Attribute)
	avm.PushData(e, attribute.Data)
	return true
}

func (s *StateReader) InputGetHash(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}
	input := d.(*st.Input)
	avm.PushData(e, input.Previous.TxID.Bytes())
	return true
}

func (s *StateReader) InputGetIndex(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}
	input := d.(*st.Input)
	avm.PushData(e, input.Previous.Index)
	return true
}

func (s *StateReader) OutputGetAssetId(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}
	output := d.(*st.Output)
	avm.PushData(e, output.AssetID.Bytes())
	return true
}

func (s *StateReader) OutputGetValue(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}
	output := d.(*st.Output)
	bytes, _ := output.Value.Bytes()
	avm.PushData(e, bytes)
	return true
}

func (s *StateReader) OutputGetCodeHash(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}
	output := d.(*st.Output)
	avm.PushData(e, output.ProgramHash.Bytes())
	return true
}

func (s *StateReader) AccountGetCodeHash(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
	//	log.Info("Get AccountState error in function AccountGetCodeHash")
		return false
	}
	accountState := d.(*states.AccountState).ProgramHash
	avm.PushData(e, accountState.Bytes())
	return true
}

func (s *StateReader) AccountGetBalance(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		//log.Info("Get AccountState error in function AccountGetCodeHash")
		return false
	}
	accountState := d.(*states.AccountState)
	if accountState == nil {
		//log.Info("Get AccountState error in function AccountGetCodeHash")
		return false
	}

	assetIdByte := avm.PopByteArray(e)
	assetId, err := common.Uint256FromBytes(assetIdByte)
	if err != nil {
		return false
	}

	balance := common.Fixed64(0)
	if v, ok := accountState.Balances[*assetId]; ok {
		balance = v
	}
	avm.PushData(e, balance.IntValue())
	return true
}

func (s *StateReader) AccountGetVotes(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		//log.Info("Get AccountState error in function AccountGetCodeHash")
		return false
	}
	accountState := d.(*states.AccountState)
	if accountState == nil {
		//log.Info("Get AccountState error in function AccountGetCodeHash")
		return false
	}
	//note ela chain is not have votes data. because consensus is pow
	pkList := make([]datatype.StackItem, 0)
	avm.PushData(e, pkList)
	return true
}

func (s *StateReader) AssetGetAssetId(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}
	switch d.(type) {
	case *st.Asset:
		asset := d.(*st.Asset)
		avm.PushData(e, asset.Hash().Bytes())
	case *states.AssetState:
		assetState := d.(*states.AssetState)
		avm.PushData(e, assetState.AssetId.Bytes())
	default:
		return false
	}
	return true
}

func (s *StateReader) AssetGetAssetType(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}
	switch d.(type) {
	case *st.Asset:
		asset := d.(*st.Asset)
		avm.PushData(e, int(asset.AssetType))
	case *states.AssetState:
		assetState := d.(*states.AssetState)
		avm.PushData(e, int(assetState.AssetType))
	default:
		return false
	}

	return true
}

func (s *StateReader) AssetGetAmount(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}
	switch d.(type) {
	case *st.Asset:
		avm.PushData(e, 0)
	case *states.AssetState:
		assetState := d.(*states.AssetState)
		avm.PushData(e, assetState.Amount.IntValue())
	default:
		return false
	}
	return true
}

func (s *StateReader) AssetGetAvailable(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}
	switch d.(type) {
	case *st.Asset:
		//asset := d.(*st.Asset)
		avm.PushData(e, 0)
	case *states.AssetState:
		assetState := d.(*states.AssetState)
		avm.PushData(e, assetState.Avaliable.IntValue())
	default:
		return false
	}

	return true
}

func (s *StateReader) AssetGetPrecision(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}
	switch d.(type) {
	case *st.Asset:
		asset := d.(*st.Asset)
		avm.PushData(e, int(asset.Precision))
	case *states.AssetState:
		assetState := d.(*states.AssetState)
		avm.PushData(e, int(assetState.Precision))
	default:
		return false
	}
	return true
}

func (s *StateReader) AssetGetOwner(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}
	switch d.(type) {
	case *st.Asset:
		//asset := d.(*st.Asset)
		avm.PushData(e, []byte{})
	case *states.AssetState:
		assetState := d.(*states.AssetState)
		owner, err := assetState.Owner.EncodePoint(true)
		if err != nil {
			return false
		}
		avm.PushData(e, owner)
	default:
		return false
	}

	return true
}

func (s *StateReader) AssetGetAdmin(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}
	switch d.(type) {
	case *st.Asset:
		//asset := d.(*st.Asset)
		avm.PushData(e, []byte{})
	case *states.AssetState:
		assetState := d.(*states.AssetState)
		avm.PushData(e, assetState.Admin.Bytes())
	default:
		return false
	}
	return true
}

func (s *StateReader) AssetGetIssuer(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}
	switch d.(type) {
	case *st.Asset:
		//asset := d.(*st.Asset)
		avm.PushData(e, []byte{})
	case *states.AssetState:
		assetState := d.(*states.AssetState)
		avm.PushData(e, assetState.Issuer.Bytes())
	default:
		return false
	}
	return true
}

func (s *StateReader) ContractGetCode(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}
	assetState := d.(*states.ContractState)
	if assetState == nil {
		return false
	}
	avm.PushData(e, assetState.Code.Code)
	return true
}

func (s *StateReader) ContractIsPayable(e *avm.ExecutionEngine) bool {
	d := avm.PopInteropInterface(e)
	if d == nil {
		return false
	}
	assetState := d.(*states.ContractState)
	if assetState == nil {
		return false
	}
	avm.PushData(e, true)
	return true
}

func (s *StateReader) StorageGetContext(e *avm.ExecutionEngine) bool {
	codeHash, err := common.Uint168FromBytes(e.Hash168(e.ExecutingScript()))
	if err != nil {
		return false
	}
	avm.PushData(e, NewStorageContext(codeHash))
	return true
}

func (s *StateReader) StorageGetReadOnlyContext(e *avm.ExecutionEngine) bool {

	codeHash, err := common.Uint168FromBytes(e.Hash168(e.ExecutingScript()))
	if err != nil {
		return false
	}
	context := NewStorageContext(codeHash)
	context.IsReadOnly = true
	avm.PushData(e, context)
	return true
}

func (s *StateReader) StorageContextAsReadOnly(e *avm.ExecutionEngine) bool {
	opInterface := avm.PopInteropInterface(e)
	if opInterface == nil {
		return false
	}
	context := opInterface.(*StorageContext)
	if !context.IsReadOnly {
		newContext := NewStorageContext(context.codeHash)
		newContext.IsReadOnly = true
		avm.PushData(e, newContext)
	}
	return true
}

func (s *StateReader) IteratorKey(e *avm.ExecutionEngine) bool {
	opInterface := avm.PopInteropInterface(e)
	if opInterface == nil {
		return false
	}
	iter := opInterface.(database.Iterator)
	avm.PushData(e, iter.Key())
	return true
}

func (s *StateReader) EnumeratorNext(e *avm.ExecutionEngine) bool {
	opInterface := avm.PopInteropInterface(e)
	if opInterface == nil {
		return false
	}
	iter := opInterface.(database.Iterator)
	avm.PushData(e, iter.Next())
	return true
}

func (s *StateReader) EnumeratorValue(e *avm.ExecutionEngine) bool {
	opInterface := avm.PopInteropInterface(e)
	if opInterface == nil {
		return false
	}
	iter := opInterface.(database.Iterator)
	avm.PushData(e, iter.Value())
	return true
}

func (s *StateReader) IteratorKeys(e *avm.ExecutionEngine) bool {

	opInterface := avm.PopInteropInterface(e)
	if opInterface == nil {
		return false
	}
	iter := opInterface.(database.Iterator)
	iterKeys := enumerators.NewIteratorKeys(iter)
	avm.PushData(e, iterKeys)
	return true
}

func (s *StateReader) IteratorValues(e *avm.ExecutionEngine) bool {

	opInterface := avm.PopInteropInterface(e)
	if opInterface == nil {
		return false
	}
	iter := opInterface.(database.Iterator)
	iterValues := enumerators.NewIteratorValues(iter)
	avm.PushData(e, iterValues)
	return true
}
