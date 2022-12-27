package states

import (
	"io"
	"bytes"

	"github.com/elastos/Elastos.ELA/common"
)

type AccountState struct {
	StateBase
	ProgramHash common.Uint168
	IsFrozen    bool
	Balances    map[common.Uint256]common.Fixed64
}

func NewAccountState(programHash common.Uint168, balances map[common.Uint256]common.Fixed64) *AccountState {
	var accountState AccountState
	accountState.ProgramHash = programHash
	accountState.Balances = balances
	accountState.IsFrozen = false
	return &accountState
}

func (accountState *AccountState) Serialize(w io.Writer) error {
	accountState.StateBase.Serialize(w)
	accountState.ProgramHash.Serialize(w)
	if accountState.IsFrozen {
		common.WriteUint8(w, 1)
	} else {
		common.WriteUint8(w, 0)
	}
	common.WriteUint64(w, uint64(len(accountState.Balances)))
	for k, v := range accountState.Balances {
		k.Serialize(w)
		v.Serialize(w)
	}
	return nil
}

func (accountState *AccountState) Deserialize(r io.Reader) error {
	stateBase := new(StateBase)
	err := stateBase.Deserialize(r)
	if err != nil {
		return err
	}
	accountState.StateBase = *stateBase
	accountState.ProgramHash.Deserialize(r)
	isFrozen, err := common.ReadUint8(r)
	if err != nil {
		return err
	}
	accountState.IsFrozen = false
	if isFrozen > 0 {
		accountState.IsFrozen = true
	}
	l, err := common.ReadUint64(r)
	if err != nil {
		return err
	}
	balances := make(map[common.Uint256]common.Fixed64, 0)
	u := new(common.Uint256)
	f := new(common.Fixed64)
	for i := 0; i < int(l); i++ {
		err = u.Deserialize(r)
		if err != nil {
			return err
		}
		err = f.Deserialize(r)
		if err != nil {
			return err
		}
		balances[*u] = *f
	}
	accountState.Balances = balances
	return nil
}

func (accountState *AccountState) Bytes() []byte {
	b := new(bytes.Buffer)
	accountState.Serialize(b)
	return b.Bytes()
}
