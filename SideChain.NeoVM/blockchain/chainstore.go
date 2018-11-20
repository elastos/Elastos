package blockchain

import (
	"errors"
	"math/big"
	"fmt"

	sb "github.com/elastos/Elastos.ELA.SideChain/blockchain"
	side "github.com/elastos/Elastos.ELA.SideChain/types"
	"github.com/elastos/Elastos.ELA.SideChain/database"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/types"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/smartcontract"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/smartcontract/service"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/smartcontract/states"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type AVMChainStore struct {
	*sb.ChainStore
	Chain *sb.BlockChain
}

func NewChainStore(path string, genesisBlock *side.Block) (*AVMChainStore, error) {
	chainStore, err := sb.NewChainStore(path, genesisBlock)
	if err != nil {
		return nil, err
	}

	store := &AVMChainStore{
		ChainStore: chainStore,
		Chain: nil,
	}

	store.RegisterFunctions(true, sb.StoreFuncNames.PersistTransactions, store.persistTransactions)

	return store, nil
}

func (c *AVMChainStore) persistTransactions(batch database.Batch, b *side.Block) error {
	for _, txn := range b.Transactions {
		if err := c.PersistTransaction(batch, txn, b.Header.Height); err != nil {
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
			c.PersistDeployTransaction(b, txn, batch)
		}

		if txn.TxType == types.Invoke {

		}
	}
	return nil
}

func (c *AVMChainStore) PersistDeployTransaction(block *side.Block, tx *side.Transaction, batch database.Batch) error {
	payloadDeploy, ok := tx.Payload.(*types.PayloadDeploy)
	if !ok {
		return errors.New("invalid deploy payload")
	}
	dbCache := NewDBCache(c)
	smartcontract, err := smartcontract.NewSmartContract(&smartcontract.Context{
		Caller:       payloadDeploy.ProgramHash,
		StateMachine: *service.NewStateMachine(dbCache, NewDBCache(c)),
		DBCache:      batch,
		Code:         payloadDeploy.Code.Code,
		Time:         big.NewInt(int64(block.Timestamp)),
		BlockNumber:  big.NewInt(int64(block.Height)),
		Gas:          payloadDeploy.Gas,
		Trigger:      avm.Application,
	})
	if err != nil {
		return err
	}
	ret, err := smartcontract.DeployContract(payloadDeploy)
	if err != nil {
		return err
	}
	codeHash, err := avm.ToCodeHash(ret)
	if err != nil {
		return err
	}
	//because neo compiler use [AppCall(hash)] ，will change hash168 to hash160,so we deploy contract use hash160
	data := avm.UInt168ToUInt160(codeHash)
	dbCache.GetOrAdd(states.ST_Contract, string(data), &states.ContractState{
		Code:        payloadDeploy.Code,
		Name:        payloadDeploy.Name,
		Version:     payloadDeploy.CodeVersion,
		Author:      payloadDeploy.Author,
		Email:       payloadDeploy.Email,
		Description: payloadDeploy.Description,
		ProgramHash: payloadDeploy.ProgramHash,
	})
	return nil
}

func (c *AVMChainStore) persisInvokeTransaction(block *side.Block, tx *side.Transaction, batch database.Batch) error {
	payloadInvoke := tx.Payload.(*types.PayloadInvoke)
	constractState := states.NewContractState()
	if !payloadInvoke.CodeHash.IsEqual(common.Uint168{}) {
		contract, err := c.GetContract(&payloadInvoke.CodeHash)
		if err != nil {
			return err
		}
		state, err := states.GetStateValue(states.ST_Contract, contract)
		if err != nil {
			return err
		}
		constractState = state.(*states.ContractState)
	}
	dbCache := NewDBCache(c)
	stateMachine := service.NewStateMachine(dbCache, NewDBCache(c))
	////stateMachine.StateReader.NotifyEvent.Subscribe(events.EventRunTimeNotify, c.onContractNotify)
	////stateMachine.StateReader.LogEvent.Subscribe(events.EventRunTimeLog, onContractLog)
	smartcontract, err := smartcontract.NewSmartContract(&smartcontract.Context{
		Caller:         payloadInvoke.ProgramHash,
		StateMachine:   *stateMachine,
		DBCache:        batch,
		CodeHash:       payloadInvoke.CodeHash,
		Input:          payloadInvoke.Code,
		SignableData:   tx,
		CacheCodeTable: NewCacheCodeTable(dbCache),
		Time:           big.NewInt(int64(block.Timestamp)),
		BlockNumber:    big.NewInt(int64(block.Height)),
		Gas:            payloadInvoke.Gas,
		ReturnType:     constractState.Code.ReturnType,
		ParameterTypes: constractState.Code.ParameterTypes,
		Trigger:      	avm.Application,
	})
	if err != nil {
		//log.Error(err.Error(), tx.Hash())
		//httpwebsocket.PushResult(tx.Hash(), int64(SmartCodeError), INVOKE_TRANSACTION, err)
		fmt.Println(err.Error(), tx.Hash())
	}

	ret, err := smartcontract.InvokeContract()
	if err != nil {
		//log.Info("contract failed:", err)
		//httpwebsocket.PushResult(tx.Hash(), int64(OutOfGas), INVOKE_TRANSACTION, ret)
		return err
	}
	fmt.Println("InvokeContract ret=", ret)
	stateMachine.CloneCache.Commit()
	dbCache.Commit()
	//httpwebsocket.PushResult(tx.Hash(), int64(Success), INVOKE_TRANSACTION, ret)
	return nil
}

func (c *AVMChainStore) GetContract(codeHash *common.Uint168) ([]byte, error) {
	prefix := []byte{byte(states.ST_Contract)}

	hashBytes := avm.UInt168ToUInt160(codeHash)
	bData, err_get := c.Get(append(prefix, hashBytes...))
	if err_get != nil {
		return nil, err_get
	}
	return bData, nil
}

func (c *AVMChainStore) Close() error {
	c.ChainStore.Close()
	return nil
}