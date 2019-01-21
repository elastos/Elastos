package api

import (
	"encoding/json"
	"fmt"
	"strconv"
	"time"

	account2 "github.com/elastos/Elastos.ELA/account"
	"github.com/elastos/Elastos.ELA/cli/script/api/mock"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/dpos/account"
	"github.com/elastos/Elastos.ELA/dpos/log"
	. "github.com/elastos/Elastos.ELA/dpos/manager"
	"github.com/elastos/Elastos.ELA/dpos/store"

	"github.com/yuin/gopher-lua"
)

type RelayType byte

const (
	luaDposManagerTypeName = "dpos_manager"

	luaConsensusIsOnDutyName  = "IsOnDuty"
	luaConsensusIsReadyName   = "IsReady"
	luaConsensusIsRunningName = "IsRunning"

	relayTx           = RelayType(0x00)
	relayBlockConfirm = RelayType(0x01)
)

func RegisterDposManagerType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaDposManagerTypeName)
	L.SetGlobal("dpos_manager", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newDposManager))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), dposManagerMethods))
}

// Constructor
func newDposManager(L *lua.LState) int {
	n := checkDposNetwork(L, 1)
	if n == nil {
		fmt.Println("Network nil, create manager error.")
		return 0
	}
	a := checkArbitrators(L, 2)
	index := uint32(L.ToInt(3))
	if index >= 5 {
		L.ArgError(1, "Index invalid.")
		return 0
	}

	pub, _ := common.HexStringToBytes(arbitratorsPublicKeys[index])
	dposManager := NewManager(DposManagerConfig{PublicKey: pub, Arbitrators: a})
	mockManager := &manager{
		DposManager: dposManager,
	}

	priKey, _ := common.HexStringToBytes(arbitratorsPrivateKeys[index])
	pubKey, _ := crypto.DecodePoint(pub)
	mockManager.Account = account.NewDposAccountFromExisting(&account2.Account{
		PrivateKey: priKey,
		PublicKey:  pubKey,
	})

	mockManager.EventMonitor = log.NewEventMoniter()
	mockManager.EventMonitor.RegisterListener(&log.EventLogs{})

	mockManager.Handler = NewHandler(n, dposManager, mockManager.EventMonitor)

	mockManager.Consensus = NewConsensus(dposManager, time.Duration(config.Parameters.ArbiterConfiguration.SignTolerance)*time.Second, mockManager.Handler)
	mockManager.Dispatcher, mockManager.IllegalMonitor = NewDispatcherAndIllegalMonitor(ProposalDispatcherConfig{
		EventMonitor: mockManager.EventMonitor,
		Consensus:    mockManager.Consensus,
		Network:      n,
		Manager:      dposManager,
		Account:      mockManager.Account,
		EventStoreAnalyzerConfig: store.EventStoreAnalyzerConfig{
			InactivePercentage: config.Parameters.ArbiterConfiguration.InactivePercentage,
			Store:              nil,
			Arbitrators:        a,
		},
	})
	mockManager.Handler.Initialize(mockManager.Dispatcher, mockManager.Consensus)

	mockManager.Peer = mock.NewPeerMock()
	dposManager.Initialize(mockManager.Handler, mockManager.Dispatcher, mockManager.Consensus, n, mockManager.IllegalMonitor, mockManager.Peer.GetBlockPool(), mockManager.Peer.GetTxPool(), mockManager.Peer.Broadcast)
	n.Initialize(DposNetworkConfig{
		ProposalDispatcher: mockManager.Dispatcher,
		Store:              nil,
	})
	n.SetListener(dposManager)

	ud := L.NewUserData()
	ud.Value = mockManager
	L.SetMetatable(ud, L.GetTypeMetatable(luaDposManagerTypeName))
	L.Push(ud)

	return 1
}

// Checks whether the first lua argument is a *LUserData with *Attribute and returns this *Attribute.
func checkDposManager(L *lua.LState, idx int) *manager {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*manager); ok {
		return v
	}
	L.ArgError(1, "DPosManager expected")
	return nil
}

var dposManagerMethods = map[string]lua.LGFunction{
	"public_key":       dposManagerPublicKey,
	"dump_consensus":   dposManagerDumpConsensus,
	"dump_node_relays": dposManagerDumpRelays,

	"is_on_duty":        dposManagerCheckOnDuty,
	"is_status_ready":   dposManagerCheckStatusReady,
	"is_status_running": dposManagerCheckStatusRunning,

	"set_on_duty": dposManagerSetOnDuty,

	"sign_proposal": dposManagerSignProposal,
	"sign_vote":     dposManagerSignVote,

	"check_last_relay": dposManagerCheckLastRelay,
}

func dposManagerDumpRelays(L *lua.LState) int {
	m := checkDposManager(L, 1)
	relays := m.Peer.DumpRelays(0)
	L.Push(lua.LString(relays))

	return 1
}

func dposManagerSetOnDuty(L *lua.LState) int {
	m := checkDposManager(L, 1)
	onDuty := L.ToBool(2)
	m.Handler.SwitchTo(onDuty)

	return 0
}

func dposManagerCheckLastRelay(L *lua.LState) int {
	m := checkDposManager(L, 1)
	t := RelayType(L.ToInt(2))

	result := false
	switch t {
	case relayTx:
		if relayedTx, ok := m.Peer.GetLastRelay().(*types.Transaction); ok {
			if tx := checkTransaction(L, 3); tx != nil {
				result = tx.Hash().IsEqual(relayedTx.Hash())
			}
		}
	case relayBlockConfirm:
		if relayedConfirm, ok := m.Peer.GetLastRelay().(*types.DposBlock); ok {
			if relayedConfirm.BlockFlag && relayedConfirm.ConfirmFlag {
				b := checkBlock(L, 3)
				c := checkConfirm(L, 4)
				if b != nil && c != nil {
					result = b.Hash().IsEqual(relayedConfirm.Block.Hash()) && confirmsEqual(c, relayedConfirm.Confirm)
				}
			} else if relayedConfirm.BlockFlag {
				b := checkBlock(L, 3)
				if b != nil {
					result = b.Hash().IsEqual(relayedConfirm.Block.Hash())
				}
			} else if relayedConfirm.ConfirmFlag {
				c := checkConfirm(L, 3)
				if c != nil {
					result = confirmsEqual(c, relayedConfirm.Confirm)
				}
			}
		}
	}
	L.Push(lua.LBool(result))

	return 1
}

func confirmsEqual(con1 *types.DPosProposalVoteSlot, con2 *types.DPosProposalVoteSlot) bool {
	if !con1.Hash.IsEqual(con2.Hash) {
		return false
	}

	if !con1.Proposal.Hash().IsEqual(con2.Proposal.Hash()) {
		return false
	}

	votes1 := make(map[common.Uint256]interface{}, 0)
	for _, v := range con1.Votes {
		votes1[v.Hash()] = nil
	}

	votes2 := make(map[common.Uint256]interface{}, 0)
	for _, v := range con2.Votes {
		votes2[v.Hash()] = nil
	}

	if len(votes1) != len(votes2) {
		return false
	}

	for k := range votes2 {
		if _, ok := votes1[k]; !ok {
			return false
		}
	}

	return true
}

func dposManagerPublicKey(L *lua.LState) int {
	m := checkDposManager(L, 1)
	L.Push(lua.LString(m.GetPublicKey()))

	return 1
}

func dposManagerDumpConsensus(L *lua.LState) int {
	m := checkDposManager(L, 1)
	consensus := m.Consensus

	content := map[string]string{
		luaConsensusIsOnDutyName:  strconv.FormatBool(consensus.IsOnDuty()),
		luaConsensusIsReadyName:   strconv.FormatBool(consensus.IsReady()),
		luaConsensusIsRunningName: strconv.FormatBool(consensus.IsRunning()),
	}
	jsonStr, _ := json.Marshal(content)
	L.Push(lua.LString(jsonStr))

	return 1
}

func dposManagerCheckOnDuty(L *lua.LState) int {
	m := checkDposManager(L, 1)
	L.Push(lua.LBool(m.Consensus.IsOnDuty()))
	return 1
}

func dposManagerCheckStatusReady(L *lua.LState) int {
	m := checkDposManager(L, 1)
	L.Push(lua.LBool(m.Consensus.IsReady()))
	return 1
}

func dposManagerCheckStatusRunning(L *lua.LState) int {
	m := checkDposManager(L, 1)
	L.Push(lua.LBool(m.Consensus.IsRunning()))
	return 1
}

func dposManagerSignProposal(L *lua.LState) int {
	m := checkDposManager(L, 1)
	p := checkProposal(L, 2)

	result := false
	if sign, err := m.Account.SignProposal(p); err == nil {
		p.Sign = sign
		result = true
	}
	L.Push(lua.LBool(result))

	return 1
}

func dposManagerSignVote(L *lua.LState) int {
	m := checkDposManager(L, 1)
	v := checkVote(L, 2)

	result := false
	if sign, err := m.Account.SignVote(v); err == nil {
		v.Sign = sign
		result = true
	}
	L.Push(lua.LBool(result))

	return 1
}

//mock object of dpos manager
type manager struct {
	DposManager
	Account        account.DposAccount
	Consensus      Consensus
	EventMonitor   *log.EventMonitor
	Handler        DposHandlerSwitch
	Dispatcher     ProposalDispatcher
	IllegalMonitor IllegalBehaviorMonitor
	Peer           mock.PeerMock
}
