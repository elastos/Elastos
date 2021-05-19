package store

import (
	"errors"
	"bytes"
	"math/big"
	"fmt"

	"github.com/elastos/Elastos.ELA.SideChain/database"
	sb "github.com/elastos/Elastos.ELA.SideChain/blockchain"
	side "github.com/elastos/Elastos.ELA.SideChain/types"

	"github.com/elastos/Elastos.ELA/common"

	"github.com/elastos/Elastos.ELA.SideChain/events"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/contract/states"
	nc "github.com/elastos/Elastos.ELA.SideChain.NeoVM/common"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/types"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/smartcontract"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/smartcontract/service"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/event"
)

const AccountPersisFlag = "AccountPersisFlag"

const DEPLOY_TRANSACTION = "DeployTransaction"
const INVOKE_TRANSACTION = "InvokeTransaction"
const RunTime_Notify = "RunTime_Notify"
const RunTime_Log = "RunTime_Log"

// Response represent the response data structure.
type ResponseExt struct {
	Action   string
	Result   bool
	Error    int
	Desc     interface{}
	TxID     string
	CodeHash string
}

func (c *LedgerStore) PersisAccount(batch database.Batch, block *side.Block) error {
	accounts := make(map[common.Uint168]*states.AccountState, 0)
	for _, txn := range block.Transactions {
		for index := 0; index < len(txn.Outputs); index++ {
			output := txn.Outputs[index]
			programHash := output.ProgramHash
			assetId := output.AssetID
			if account, ok := accounts[programHash]; ok {
				account.Balances[assetId] += output.Value
			} else {
				account, err := c.GetAccount(&programHash)
				if err != nil && err.Error() != ErrDBNotFound.Error() {
					return err
				}
				if account != nil {
					account.Balances[assetId] += output.Value
				} else {
					balances := make(map[common.Uint256]common.Fixed64, 0)
					balances[assetId] = output.Value
					account = states.NewAccountState(programHash, balances)
				}
				accounts[programHash] = account
			}
		}

		for index := 0; index < len(txn.Inputs); index++ {
			if txn.TxType == side.CoinBase {
				continue
			}
			input := txn.Inputs[index]
			transaction, _, err := c.GetTransaction(input.Previous.TxID)
			if err != nil {
				return err
			}
			index := input.Previous.Index
			output := transaction.Outputs[index]
			programHash := output.ProgramHash
			assetID := output.AssetID
			if account, ok := accounts[programHash]; ok {
				account.Balances[assetID] -= output.Value
			} else {
				account, err := c.GetAccount(&programHash)
				if err != nil && err.Error() != ErrDBNotFound.Error() {
					return err
				}
				account.Balances[assetID] -= output.Value
				accounts[programHash] = account
			}
			if accounts[programHash].Balances[assetID] < 0 {
				return errors.New(fmt.Sprintf("account programHash:%v, assetId:%v insufficient of balance", programHash, assetID))
			}
		}
	}
	for programHash, value := range accounts {
		accountKey := new(bytes.Buffer)
		accountKey.WriteByte(byte(sb.ST_Account))
		programHash.Serialize(accountKey)

		accountValue := new(bytes.Buffer)
		value.Serialize(accountValue)
		batch.Put(accountKey.Bytes(), accountValue.Bytes())
	}
	return nil
}

func (c *LedgerStore) PersistDeployTransaction(block *side.Block, tx *side.Transaction, batch database.Batch) error {
	payloadDeploy, ok := tx.Payload.(*types.PayloadDeploy)
	if !ok {
		return errors.New("invalid deploy payload")
	}
	dbCache := blockchain.NewDBCache(c)
	smartcontract, err := smartcontract.NewSmartContract(&smartcontract.Context{
		Caller:       payloadDeploy.ProgramHash,
		StateMachine: *service.NewStateMachine(dbCache, dbCache),
		DBCache:      batch,
		Code:         payloadDeploy.Code.Code,
		Time:         big.NewInt(int64(block.GetTimeStamp())),
		BlockNumber:  big.NewInt(int64(block.GetHeight())),
		Gas:          payloadDeploy.Gas,
		Trigger:      avm.Application,
	})
	if err != nil {
		events.Notify(event.ETDeployTransaction, &ResponseExt{
			Action:   DEPLOY_TRANSACTION,
			Result:   false,
			Desc:     err.Error(),
			TxID:     tx.Hash().String(),
			CodeHash: "",
		})
		return err
	}
	ret, err := smartcontract.DeployContract(payloadDeploy)
	if err != nil {
		events.Notify(event.ETDeployTransaction, &ResponseExt{
			Action:   DEPLOY_TRANSACTION,
			Result:   false,
			Desc:     err.Error(),
			TxID:     tx.Hash().String(),
			CodeHash: "",
		})
		return err
	}
	codeHash, err := nc.ToCodeHash(ret)
	if err != nil {
		events.Notify(event.ETDeployTransaction, &ResponseExt{
			Action:   DEPLOY_TRANSACTION,
			Result:   false,
			Desc:     err.Error(),
			TxID:     tx.Hash().String(),
			CodeHash: codeHash.String(),
		})
		return err
	}

	contract, err := c.GetContract(codeHash)
	if err != nil && contract != nil {
		events.Notify(event.ETDeployTransaction, &ResponseExt{
			Action:   DEPLOY_TRANSACTION,
			Result:   false,
			Desc:     err.Error(),
			TxID:     tx.Hash().String(),
			CodeHash: codeHash.String(),
		})
		return err
	}
	//because neo compiler use [AppCall(hash)] ，will change hash168 to hash160,so we deploy contract use hash160
	data := nc.UInt168ToUInt160(codeHash)

	dbCache.GetOrAdd(sb.ST_Contract, string(data), &states.ContractState{
		Code:        payloadDeploy.Code,
		Name:        payloadDeploy.Name,
		Version:     payloadDeploy.CodeVersion,
		Author:      payloadDeploy.Author,
		Email:       payloadDeploy.Email,
		Description: payloadDeploy.Description,
		ProgramHash: payloadDeploy.ProgramHash,
	})
	log.Info("deploy contract suc:", codeHash.String())
	events.Notify(event.ETDeployTransaction, &ResponseExt{
		Action:   DEPLOY_TRANSACTION,
		Result:   true,
		Desc:     "Success",
		TxID:     tx.Hash().String(),
		CodeHash: codeHash.String(),
	})
	dbCache.Commit()
	return nil
}

func (c *LedgerStore) PersisInvokeTransaction(block *side.Block, tx *side.Transaction, batch database.Batch) (*types.Receipt, error) {
	payloadInvoke := tx.Payload.(*types.PayloadInvoke)
	receipt, err := types.NewReceipt(payloadInvoke.CodeHash, tx, block)
	if err != nil {
		log.Errorf("NewReceipt failed, txid:%s, error:%s", tx.Hash(), err.Error())
	}
	constractState := states.NewContractState()
	if !payloadInvoke.CodeHash.IsEqual(common.Uint168{}) {
		contract, err := c.GetContract(&payloadInvoke.CodeHash)
		if err != nil {
			events.Notify(event.ETInvokeTransaction, &ResponseExt{
				Action:   INVOKE_TRANSACTION,
				Result:   false,
				Desc:     err.Error(),
				TxID:     tx.Hash().String(),
				CodeHash: payloadInvoke.CodeHash.String(),
			})

			c.onInvokeFailed(receipt, 0)
			log.Errorf("invoke transaction failed, txid:%s, error:%s", tx.Hash(), err.Error())
			return receipt, err
		}
		state, err := states.GetStateValue(sb.ST_Contract, contract)
		if err != nil {
			events.Notify(event.ETInvokeTransaction, &ResponseExt{
				Action:   INVOKE_TRANSACTION,
				Result:   false,
				Desc:     err.Error(),
				TxID:     tx.Hash().String(),
				CodeHash: payloadInvoke.CodeHash.String(),
			})
			c.onInvokeFailed(receipt, 0)
			log.Errorf("invoke transaction failed, txid:%s, error:%s", tx.Hash(), err.Error())
			return receipt, err
		}
		constractState = state.(*states.ContractState)
	}
	dbCache := blockchain.NewDBCache(c)
	stateMachine := service.NewStateMachine(dbCache, dbCache)
	smartcontract, err := smartcontract.NewSmartContract(&smartcontract.Context{
		Caller:         payloadInvoke.ProgramHash,
		StateMachine:   *stateMachine,
		DBCache:        batch,
		CodeHash:       payloadInvoke.CodeHash,
		Input:          payloadInvoke.Code,
		SignableData:   tx,
		CacheCodeTable: NewCacheCodeTable(dbCache),
		Time:           big.NewInt(int64(block.GetTimeStamp())),
		BlockNumber:    big.NewInt(int64(block.GetHeight())),
		Gas:            payloadInvoke.Gas,
		ReturnType:     constractState.Code.ReturnType,
		ParameterTypes: constractState.Code.ParameterTypes,
		Trigger:        avm.Application,
	})
	if err != nil {
		events.Notify(event.ETInvokeTransaction, &ResponseExt{
			Action:   INVOKE_TRANSACTION,
			Result:   false,
			Desc:     err.Error(),
			TxID:     tx.Hash().String(),
			CodeHash: payloadInvoke.CodeHash.String(),
		})
		c.onInvokeFailed(receipt, 0)
		log.Errorf("invoke transaction failed, txid:%s, error:%s", tx.Hash(), err.Error())
	}

	ret, err := smartcontract.InvokeContract()
	if err != nil {
		events.Notify(event.ETInvokeTransaction, &ResponseExt{
			Action:   INVOKE_TRANSACTION,
			Result:   false,
			Desc:     err.Error(),
			TxID:     tx.Hash().String(),
			CodeHash: payloadInvoke.CodeHash.String(),
		})
		c.onInvokeFailed(receipt, uint64((smartcontract.Engine.(*avm.ExecutionEngine)).GetGasConsumed()))
		log.Errorf("invoke transaction failed, txid:%s, error:%s", tx.Hash(), err.Error())
		return receipt, err
	}
	log.Info("InvokeContract ret=", ret)
	if batch != nil {
		stateMachine.CloneCache.Commit()
		dbCache.Commit()
	}
	events.Notify(event.ETInvokeTransaction, &ResponseExt{
		Action:   INVOKE_TRANSACTION,
		Result:   true,
		Desc:     ret,
		TxID:     tx.Hash().String(),
		CodeHash: payloadInvoke.CodeHash.String(),
	})
	var notifyevents []service.NotifyEventArgs
	notifyevents = stateMachine.StateReader.GetNotifyEvents()
	c.onInvokeSucssed(receipt, uint64((smartcontract.Engine.(*avm.ExecutionEngine)).GetGasConsumed()),
		notifyevents)
	return receipt, nil
}

func (c *LedgerStore) onInvokeFailed(receipt *types.Receipt, gas uint64) {
	receipt.Status = false
	receipt.GasUsed = gas
}

func (c *LedgerStore) onInvokeSucssed(receipt *types.Receipt, gas uint64,
	notifyevents []service.NotifyEventArgs) {
	receipt.Status = true
	receipt.GasUsed = gas
	for _, evt := range notifyevents {
		l := types.Nep5Log{}
		l.BlockNumber = receipt.BlockNumber
		l.TxHash = receipt.TxHash.String()
		l.TxIndex = receipt.TransactionIndex
		l.Index = uint32(len(receipt.Logs))
		l.Address = receipt.ContractAddress.String()
		l.Data = big.Int{}
		items := evt.GetStateItem().GetArray()
		if len(items) == 4 && string(items[0].GetByteArray()) == "transfer" {
			from, err := common.Uint160FromBytes(items[1].GetByteArray())
			if err != nil {
				log.Error("onInvokeSucssed Nep5 Transfer from error:", err, from)
			}
			to, err := common.Uint160FromBytes(items[2].GetByteArray())
			if err != nil {
				log.Error("onInvokeSucssed Nep5 Transfer to error:", err)
			}
			l.From = from
			l.To = to
			l.Data = *items[3].GetBigInteger()
		}
		receipt.Logs = append(receipt.Logs, &l)
	}
}

func (c *LedgerStore) WriteReceipts(block *side.Block, receipts types.Receipts) error {
	buf := new(bytes.Buffer)
	len := len(receipts)
	err := common.WriteVarUint(buf, uint64(len))
	if err != nil {
		return err
	}
	for _, receipt := range receipts {
		err := receipt.Serialize(buf, 0)
		if err != nil {
			log.Errorf("Failed to encode block receipts=", err)
			return err
		}
	}
	return c.Put(blockReceiptsKey(block.GetHeight(), block.Hash()), buf.Bytes())
}

func (c *LedgerStore) GetReceipts(height uint32, hash *common.Uint256) (types.Receipts, error) {
	data, err := c.Get(blockReceiptsKey(height, *hash))
	if err != nil {
		return nil, err
	}
	reader := bytes.NewReader(data)

	len, err := common.ReadVarUint(reader, 0)
	if err != nil {
		return nil, err
	}
	block, err := c.GetBlock(*hash)
	if err != nil {
		return nil, err
	}
	rs := make(types.Receipts, len)
	var i uint64
	for i = 0; i < len; i++ {
		rs[i] = &types.Receipt{}
		err := rs[i].Deserialize(reader, 0)
		rs[i].BlockNumber = height
		rs[i].TransactionIndex, _ = rs[i].GetTxIndex(rs[i].TxHash, block)
		rs[i].BlockHash = *hash
		if err != nil {
			return nil, err
		}
		for _, log := range rs[i].Logs {
			log.BlockHash = hash.String()
			log.BlockNumber = height
		}
	}
	return rs, nil
}

// WriteTxLookupEntries stores a positional metadata for every transaction from
// a block, enabling hash based transaction and receipt lookups.
func (c *LedgerStore) WriteTxLookupEntries(block *side.Block) {
	for _, tx := range block.Transactions {
		if err := c.Put(txLookupKey(tx.Hash()), block.Hash().Bytes()); err != nil {
			log.Errorf("Failed to store transaction lookup entry", err)
		}
	}
}

// DeleteTxLookupEntry removes all transaction data associated with a hash.
func (c *LedgerStore) DeleteTxLookupEntry(hash common.Uint256) error {
	return c.Delete(txLookupKey(hash))
}

// ReadTxLookupEntry retrieves the positional metadata associated with a transaction
// hash to allow retrieving the transaction or receipt by hash.
func (c *LedgerStore) GetTxLookupEntry(hash common.Uint256) common.Uint256 {
	data, _ := c.Get(txLookupKey(hash))
	blockHash := &common.Uint256{}
	if len(data) == common.UINT256SIZE {
		blockHash, _ = common.Uint256FromBytes(data)
	}
	return *blockHash
}

// ReadTransaction retrieves a specific transaction from the database, along with
// its added positional metadata.
func (c *LedgerStore) ReadTransaction(hash common.Uint256) (*side.Transaction, common.Uint256, uint32, uint32) {
	blockHash := c.GetTxLookupEntry(hash)
	if blockHash == (common.Uint256{}) {
		return nil, common.Uint256{}, 0, 0
	}
	block, err := c.GetBlock(blockHash)
	if err != nil {
		log.Error("not found block:", blockHash.String())
		return nil, blockHash, 0, 0
	}

	for txIndex, tx := range block.Transactions {
		if tx.Hash().IsEqual(hash) {
			return tx, blockHash, block.GetHeight(), uint32(txIndex)
		}
	}
	log.Error("Transaction not found", "number", block.GetHeight(), "hash", blockHash, "txhash", hash)
	return nil, blockHash, block.GetHeight(), 0
}

func (c *LedgerStore) GetContract(codeHash *common.Uint168) ([]byte, error) {
	prefix := []byte{byte(sb.ST_Contract)}

	hashBytes := nc.UInt168ToUInt160(codeHash)
	bData, err_get := c.Get(append(prefix, hashBytes...))
	if err_get != nil {
		return nil, err_get
	}
	return bData, nil
}

func (c *LedgerStore) GetAccount(programHash *common.Uint168) (*states.AccountState, error) {
	accountPrefix := []byte{byte(sb.ST_Account)}
	state, err := c.Get(append(accountPrefix, programHash.Bytes()...))
	if err != nil {
		return nil, err
	}

	accountState := new(states.AccountState)
	accountState.Deserialize(bytes.NewBuffer(state))

	return accountState, nil
}

func (c *LedgerStore) GetBlock(hash common.Uint256) (*side.Block, error) {
	var b = side.NewBlock()
	b.Header = types.NewHeader()
	prefix := []byte{byte(sb.DATA_Header)}
	bHash, err := c.Get(append(prefix, hash.Bytes()...))
	if err != nil {
		return nil, err
	}

	r := bytes.NewReader(bHash)

	// first 8 bytes is sys_fee
	_, err = common.ReadUint64(r)
	if err != nil {
		return nil, err
	}

	// Deserialize block data
	if err := b.FromTrimmedData(r); err != nil {
		return nil, err
	}

	// Deserialize transaction
	for i, txn := range b.Transactions {
		tmp, _, err := c.GetTransaction(txn.Hash())
		if err != nil {
			return nil, err
		}
		b.Transactions[i] = tmp
	}

	return b, nil
}

func (c *LedgerStore) Close() error {
	c.Close()
	return nil
}