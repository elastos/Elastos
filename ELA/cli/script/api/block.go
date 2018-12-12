package api

import (
	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/pow"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/yuin/gopher-lua"
)

const (
	luaBlockTypeName = "block"
	minerAddress     = "8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta"
)

func RegisterBlockType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaBlockTypeName)
	L.SetGlobal("block", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newBlock))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), blockMethods))
}

// Constructor
func newBlock(L *lua.LState) int {

	tx, _ := pow.CreateCoinbaseTx(minerAddress)
	block := &types.Block{
		Transactions: []*types.Transaction{tx},
	}

	ud := L.NewUserData()
	ud.Value = block
	L.SetMetatable(ud, L.GetTypeMetatable(luaBlockTypeName))
	L.Push(ud)

	return 1
}

// Checks whether the first lua argument is a *LUserData with *Attribute and returns this *Attribute.
func checkBlock(L *lua.LState, idx int) *types.Block {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*types.Block); ok {
		return v
	}
	L.ArgError(1, "DPosProposal expected")
	return nil
}

var blockMethods = map[string]lua.LGFunction{
	"hash":       blockHash,
	"get_header": blockGetHeader,
	"set_header": blockSetHeader,
	"append_tx":  blockAppendTx,
	"update":     blockUpdate,
}

func blockHash(L *lua.LState) int {
	p := checkBlock(L, 1)
	h := p.Hash()

	L.Push(lua.LString(h.String()))

	return 1
}

func blockSetHeader(L *lua.LState) int {
	p := checkBlock(L, 1)
	h := checkHeader(L, 2)
	p.Header = *h

	return 0
}

func blockGetHeader(L *lua.LState) int {
	p := checkBlock(L, 1)
	h := &p.Header

	ud := L.NewUserData()
	ud.Value = h
	L.SetMetatable(ud, L.GetTypeMetatable(luaHeaderTypeName))
	L.Push(ud)

	return 1
}

func blockAppendTx(L *lua.LState) int {
	p := checkBlock(L, 1)
	tx := checkTransaction(L, 2)
	p.Transactions = append(p.Transactions, tx)

	return 0
}

func blockUpdate(L *lua.LState) int {
	b := checkBlock(L, 1)
	updateDposRewards(b)
	updateMerkleRoot(b)

	return 0
}

func updateDposRewards(b *types.Block) {

	totalTxFee := common.Fixed64(0)
	for _, tx := range b.Transactions {
		fee := blockchain.GetTxFee(tx, blockchain.DefaultLedger.Blockchain.AssetID)
		if fee != tx.Fee {
			continue
		}
		totalTxFee += fee
	}

	blockReward := blockchain.RewardAmountPerBlock
	totalReward := totalTxFee + blockReward
	blockchain.DefaultLedger.HeightVersions.AssignCoinbaseTxRewards(b, totalReward)
}

func updateMerkleRoot(b *types.Block) {
	txHash := make([]common.Uint256, 0, len(b.Transactions))
	for _, tx := range b.Transactions {
		txHash = append(txHash, tx.Hash())
	}
	txRoot, _ := crypto.ComputeRoot(txHash)
	b.Header.MerkleRoot = txRoot
}
