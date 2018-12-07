package states

import (
	"io"
	"bytes"
	"errors"
	"fmt"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
)

type IStateValueInterface interface {
	Serialize(w io.Writer) error
	Deserialize(r io.Reader) error
}

type IStateKeyInterface interface {
	Serialize(w io.Writer) error
	Deserialize(r io.Reader) error
}

var (
	StatesMap = map[blockchain.EntryPrefix]IStateValueInterface{
		blockchain.ST_Contract:   new(ContractState),
		blockchain.ST_Account:    new(AccountState),
		blockchain.ST_AssetState: new(AssetState),
		blockchain.ST_Storage:    new(StorageItem),
	}
)

func GetStateValue(prefix blockchain.EntryPrefix, data []byte) (IStateValueInterface, error) {
	r := bytes.NewBuffer(data)
	state := StatesMap[prefix]
	if state == nil {
		fmt.Println("StatesMap not has this key", prefix)
		return nil, errors.New("StatesMap not has key")
	}
	err := state.Deserialize(r)
	if err != nil {
		return nil, err
	}
	return state, nil
}
