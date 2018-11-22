package service

import (
	"github.com/elastos/Elastos.ELA.SideChain/database"

	"github.com/elastos/Elastos.ELA.SideChain/types"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/interfaces"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/smartcontract/service"
)

var Store database.Database
var Table interfaces.IScriptTable

func RunScript(script []byte) *avm.ExecutionEngine {
	container := types.Transaction{Inputs:[]*types.Input{}, Outputs:[]*types.Output{}}
	dbCache := blockchain.NewDBCache(Store)
	stateMachine := service.NewStateMachine(dbCache, dbCache)
	e := avm.NewExecutionEngine(
		&container,
		new(avm.CryptoECDsa),
		avm.MAXSTEPS,
		Table,
		stateMachine,
		9999999 * 100000000,
		avm.Application,
		true,
	)
	e.LoadScript(script, false)
	e.Execute()
	return e
}