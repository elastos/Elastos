package service

import (
	"math"
	"bytes"
	"fmt"

	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/common"

	"github.com/elastos/Elastos.ELA.SideChain/types"
	sb "github.com/elastos/Elastos.ELA.SideChain/blockchain"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/smartcontract/storage"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/smartcontract/errors"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/contract/states"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm"
	nc "github.com/elastos/Elastos.ELA.SideChain.NeoVM/common"
	nt "github.com/elastos/Elastos.ELA.SideChain.NeoVM/types"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/blockchain"

)

type StateMachine struct {
	*StateReader
	CloneCache *storage.CloneCache
}

func NewStateMachine(dbCache storage.DBCache, innerCache storage.DBCache) *StateMachine {
	var stateMachine StateMachine
	stateMachine.CloneCache = storage.NewCloneDBCache(innerCache, dbCache)
	stateMachine.StateReader = NewStateReader()

	stateMachine.StateReader.Register("Neo.Asset.Create", stateMachine.CreateAsset)
	stateMachine.StateReader.Register("Neo.Contract.Create", stateMachine.CreateContract)
	stateMachine.StateReader.Register("Neo.Contract.Migrate", stateMachine.ContractMigrate)
	stateMachine.StateReader.Register("Neo.Blockchain.GetContract", stateMachine.GetContract)
	stateMachine.StateReader.Register("Neo.Asset.Renew", stateMachine.AssetRenew)
	stateMachine.StateReader.Register("Neo.Storage.Get", stateMachine.StorageGet)
	stateMachine.StateReader.Register("Neo.Contract.Destroy", stateMachine.ContractDestory)
	stateMachine.StateReader.Register("Neo.Storage.Put", stateMachine.StoragePut)
	stateMachine.StateReader.Register("Neo.Storage.Delete", stateMachine.StorageDelete)
	stateMachine.StateReader.Register("Neo.Storage.Find", stateMachine.StorageFind)
	stateMachine.StateReader.Register("Neo.Contract.GetStorageContext", stateMachine.GetStorageContext)
	stateMachine.StateReader.Register("Neo.Account.IsStandard", stateMachine.AccountIsStandard)

	return &stateMachine
}

func (s *StateMachine) CreateAsset(engine *avm.ExecutionEngine) bool {
	tx := engine.GetDataContainer().(*types.Transaction)
	assetID := tx.Hash()
	assetType := types.AssetType(avm.PopInt(engine))
	name := avm.PopByteArray(engine)
	if len(name) > 1024 {
		fmt.Println(errors.ErrAssetNameInvalid)
		return false
	}
	amount := avm.PopBigInt(engine)
	if amount.Int64() == 0 {
		fmt.Println(errors.ErrAssetAmountInvalid)
		return false
	}
	precision := avm.PopBigInt(engine)
	if precision.Int64() > 8 {
		fmt.Println(errors.ErrAssetPrecisionInvalid)
		return false
	}
	if amount.Int64()%int64(math.Pow(10, 8-float64(precision.Int64()))) != 0 {
		fmt.Println(errors.ErrAssetAmountInvalid)
		return false
	}
	ownerByte := avm.PopByteArray(engine)
	owner, err := crypto.DecodePoint(ownerByte)
	if err != nil {
		fmt.Println(err)
		return false
	}
	if result, err := s.StateReader.CheckWitnessPublicKey(engine, owner); !result {
		fmt.Println(err)
		return false
	}
	adminByte := avm.PopByteArray(engine)
	admin, err := common.Uint168FromBytes(adminByte)
	if err != nil {
		fmt.Println(err)
		return false
	}
	issueByte := avm.PopByteArray(engine)
	issue, err := common.Uint168FromBytes(issueByte)
	if err != nil {
		fmt.Println(err)
		return false
	}
	assetState := &states.AssetState{
		AssetId:    assetID,
		AssetType:  types.AssetType(assetType),
		Name:       string(name),
		Amount:     common.Fixed64(amount.Int64()),
		Precision:  byte(precision.Int64()),
		Admin:      *admin,
		Issuer:     *issue,
		Owner:      owner,
		Expiration: blockchain.DefaultChain.BestChain.Height + 1 + 2000000,
		IsFrozen:   false,
	}
	s.CloneCache.GetInnerCache().GetWriteSet().Add(sb.ST_AssetState, string(assetID.Bytes()), assetState)
	avm.PushData(engine, assetState)
	return true
}

func (s *StateMachine) CreateContract(engine *avm.ExecutionEngine) bool {
	codeByte := avm.PopByteArray(engine)
	if len(codeByte) > int(avm.MaxItemSize) {
		return false
	}
	parameters := avm.PopByteArray(engine)
	if len(parameters) > avm.MaxParameterSize {
		return false
	}
	parameterList := make([]nt.ContractParameterType, 0)
	for _, v := range parameters {
		parameterList = append(parameterList, nt.ContractParameterType(v))
	}
	returnType := avm.PopInt(engine)
	nameByte := avm.PopByteArray(engine)
	if len(nameByte) > avm.MaxParameterSize {
		return false
	}
	versionByte := avm.PopByteArray(engine)
	if len(versionByte) > avm.MaxParameterSize {
		return false
	}
	authorByte := avm.PopByteArray(engine)
	if len(authorByte) > avm.MaxParameterSize {
		return false
	}
	emailByte := avm.PopByteArray(engine)
	if len(emailByte) > avm.MaxParameterSize {
		return false
	}
	descByte := avm.PopByteArray(engine)
	if len(descByte) > avm.MAXContractDescript {
		return false
	}
	funcCode := nt.FunctionCode{
		Code:           codeByte,
		ParameterTypes: parameterList,
		ReturnType:     nt.ContractParameterType(returnType),
	}
	contractState := &states.ContractState{
		Code:        &funcCode,
		Name:        common.BytesToHexString(nameByte),
		Version:     common.BytesToHexString(versionByte),
		Author:      common.BytesToHexString(authorByte),
		Email:       common.BytesToHexString(emailByte),
		Description: common.BytesToHexString(descByte),
	}
	codeHash := funcCode.CodeHash()
	key := nc.UInt168ToUInt160(&codeHash)
	s.CloneCache.GetInnerCache().GetOrAdd(sb.ST_Contract, string(key), contractState)
	avm.PushData(engine, contractState)
	return true
}

func (s *StateMachine) GetContract(engine *avm.ExecutionEngine) bool {
	hashByte := avm.PopByteArray(engine)
	var keyStr []byte
	if len(hashByte) == 20 {
		keyStr = hashByte
	} else {
		hash, err := common.Uint168FromBytes(hashByte)
		if err != nil {
			return false
		}
		keyStr = nc.UInt168ToUInt160(hash)
	}

	item, err := s.CloneCache.TryGet(sb.ST_Contract, string(keyStr))
	if err != nil {
		return false
	}
	avm.PushData(engine, item.(*states.ContractState))
	return true
}

func (s *StateMachine) ContractMigrate(engine *avm.ExecutionEngine) bool {
	codeByte := avm.PopByteArray(engine)

	if len(codeByte) > int(avm.MaxItemSize) {
		return false
	}
	parameters := avm.PopByteArray(engine)
	if len(parameters) > 252 {
		return false
	}
	parameterList := make([]nt.ContractParameterType, 0)
	for _, v := range parameters {
		parameterList = append(parameterList, nt.ContractParameterType(v))
	}
	returnType := avm.PopInt(engine)
	needStorage := avm.PopBoolean(engine)
	nameByte := avm.PopByteArray(engine)
	if len(nameByte) > 252 {
		return false
	}
	versionByte := avm.PopByteArray(engine)
	if len(versionByte) > 252 {
		return false
	}
	authorByte := avm.PopByteArray(engine)
	if len(authorByte) > 252 {
		return false
	}
	emailByte := avm.PopByteArray(engine)
	if len(emailByte) > 252 {
		return false
	}
	descByte := avm.PopByteArray(engine)
	if len(descByte) > 65536 {
		return false
	}

	funcCode := &nt.FunctionCode{
		Code:           codeByte,
		ParameterTypes: parameterList,
		ReturnType:     nt.ContractParameterType(returnType),
	}
	codeHash := funcCode.CodeHash()
	keyStr := nc.UInt168ToUInt160(&codeHash)
	item, err := s.CloneCache.TryGet(sb.ST_Contract, string(keyStr))
	if err != nil {
		item = &states.ContractState{
			Code:        funcCode,
			Name:        common.BytesToHexString(nameByte),
			Version:     common.BytesToHexString(versionByte),
			Author:      common.BytesToHexString(authorByte),
			Email:       common.BytesToHexString(emailByte),
			Description: common.BytesToHexString(descByte),
		}
		if !engine.IsTestMode() {
			s.CloneCache.GetInnerCache().GetOrAdd(sb.ST_Contract, string(keyStr), item)
		}

		if needStorage {
			data := engine.ExecutingScript()
			if data == nil {
				return false
			}
			oldHash, err := nc.ToProgramHash(data)
			if err != nil {
				return false
			}
			storageKey := states.NewStorageKey(oldHash, []byte{})
			datas := s.CloneCache.Find(sb.ST_Storage, storage.KeyToStr(storageKey))
			for datas.Next() {
				key := datas.Key()
				value := datas.Value()
				reader := bytes.NewReader(key[1:len(key)])
				storageKey.Deserialize(reader)
				storageKey.CodeHash = &codeHash
				s.CloneCache.GetInnerCache().GetWriteSet().Add(sb.ST_Storage, storage.KeyToStr(storageKey), states.NewStorageItem(value))
			}
		}
	}
	avm.PushData(engine, item.(*states.ContractState))
	return s.ContractDestory(engine)
}

func (s *StateMachine) AssetRenew(engine *avm.ExecutionEngine) bool {
	data := avm.PopInteropInterface(engine)
	years := avm.PopInt(engine)
	at := data.(*states.AssetState)
	height := blockchain.DefaultChain.BestChain.Height + 1
	b := new(bytes.Buffer)
	at.AssetId.Serialize(b)
	state, err := s.CloneCache.TryGet(sb.ST_AssetState, b.String())
	if err != nil {
		fmt.Println(err)
		return false
	}
	assetState := state.(*states.AssetState)
	if assetState.Expiration < height {
		assetState.Expiration = height
	}
	expiration := assetState.Expiration
	assetState.Expiration += uint32(years) * 2000000
	if assetState.Expiration - expiration !=  uint32(years) * 2000000 {
		assetState.Expiration = math.MaxInt32
	}
	avm.PushData(engine, assetState)
	return true
}

func (s *StateMachine) ContractDestory(engine *avm.ExecutionEngine) bool {
	data := engine.ExecutingScript()
	if data == nil {
		return false
	}
	hash, err := nc.ToCodeHash(data)
	if err != nil {
		return false
	}
	keyStr := string(nc.UInt168ToUInt160(hash))
	item, err := s.CloneCache.TryGet(sb.ST_Contract, keyStr)
	if err != nil || item == nil {
		log.Error("ContractDestory:", err.Error())
		return false
	}
	if !engine.IsTestMode() {
		s.CloneCache.GetInnerCache().TryDelete(sb.ST_Contract, keyStr)
	}

	return true
}

func (s *StateMachine) CheckStorageContext(context *StorageContext) (bool, error) {
	hashStr := string(nc.UInt168ToUInt160(context.codeHash))
	item, err := s.CloneCache.TryGet(sb.ST_Contract, hashStr)
	if err != nil {
		return false, err
	}
	if item == nil {
		return false, fmt.Errorf("check storage context fail, codehash=%v", context.codeHash)
	}
	return true, nil
}

func (s *StateMachine) StorageGet(engine *avm.ExecutionEngine) bool {
	opInterface := avm.PopInteropInterface(engine)
	if opInterface == nil {
		return false
	}
	context := opInterface.(*StorageContext)
	if exist, err := s.CheckStorageContext(context); !exist && err.Error() != "leveldb: not found" {
		fmt.Println(err)
		return false
	}
	key := avm.PopByteArray(engine)
	storageKey := states.NewStorageKey(context.codeHash, key)
	item, err := s.CloneCache.TryGet(sb.ST_Storage, storage.KeyToStr(storageKey))
	if err != nil && err.Error() != "leveldb: not found" {
		return false
	}
	if item == nil {
		avm.PushData(engine, []byte{})
	} else {
		avm.PushData(engine, item.(*states.StorageItem).Value)
	}
	return true
}

func (s *StateMachine) StoragePut(engine *avm.ExecutionEngine) bool {
	opInterface := avm.PopInteropInterface(engine)
	if opInterface == nil {
		return false
	}
	context := opInterface.(*StorageContext)
	if context.IsReadOnly {
		return false
	}
	if exist, err := s.CheckStorageContext(context); !exist && err.Error() != "leveldb: not found" {
		return false
	}
	key := avm.PopByteArray(engine)
	if len(key) > 1024 {
		return false
	}
	value := avm.PopByteArray(engine)
	storageKey := states.NewStorageKey(context.codeHash, key)
	s.CloneCache.GetInnerCache().GetWriteSet().Add(sb.ST_Storage, storage.KeyToStr(storageKey), states.NewStorageItem(value))
	return true
}

func (s *StateMachine) StorageDelete(engine *avm.ExecutionEngine) bool {
	opInterface := avm.PopInteropInterface(engine)
	context := opInterface.(*StorageContext)
	if context.IsReadOnly {
		return false
	}
	if exist, err := s.CheckStorageContext(context); !exist && err.Error() != "leveldb: not found" {
		return false
	}
	key := avm.PopByteArray(engine)
	storageKey := states.NewStorageKey(context.codeHash, key)
	s.CloneCache.GetInnerCache().GetWriteSet().Delete(sb.ST_Storage, storage.KeyToStr(storageKey))
	return true
}

func (s *StateMachine) StorageFind(engine *avm.ExecutionEngine) bool {
	opInterface := avm.PopInteropInterface(engine)
	context := opInterface.(*StorageContext)
	if exist, err := s.CheckStorageContext(context); !exist && err.Error() != "leveldb: not found" {
		return false
	}
	key := avm.PopByteArray(engine)

	storageKey := states.NewStorageKey(context.codeHash, key)
	datas := s.CloneCache.Find(sb.ST_Storage, storage.KeyToStr(storageKey))
	avm.PushData(engine, datas)
	return true
}

func (s *StateMachine) GetStorageContext(engine *avm.ExecutionEngine) bool {
	data := engine.CurrentContext().GetCodeHash()
	codeHash, err := common.Uint168FromBytes(data)
	if err != nil {
		return false
	}
	avm.PushData(engine, NewStorageContext(codeHash))
	return true
}

func (s *StateMachine) AccountIsStandard(e *avm.ExecutionEngine) bool {
	d := avm.PopByteArray(e)
	hash, err := common.Uint168FromBytes(d)
	if err != nil {
		return false
	}
	keyStr := nc.UInt168ToUInt160(hash)
	item, err := s.CloneCache.TryGet(sb.ST_Contract, string(keyStr))
	if err != nil {
		return false
	}
	contractState := item.(*states.ContractState)
	if contractState == nil {
		return false
	}
	if contractState.IsSignatureCotract() || contractState.IsMultiSigContract() {
		avm.PushData(e, true)
	} else {
		avm.PushData(e, false)
	}

	return true
}

func contains(programHashes []common.Uint168, programHash common.Uint168) bool {
	for _, v := range programHashes {
		if v.ToCodeHash().IsEqual(programHash.ToCodeHash()) {
			return true
		}
	}
	return false
}
