package states

import (
	"io"
	"bytes"
	"errors"
	"fmt"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
)

const (
	// ASSET
	ST_Info       blockchain.EntryPrefix = 0xc0
	ST_Contract   blockchain.EntryPrefix = 0xc2
	ST_Storage    blockchain.EntryPrefix = 0xc3
	ST_Account    blockchain.EntryPrefix = 0xc4
	ST_AssetState blockchain.EntryPrefix = 0xc5
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
		ST_Contract:   new(ContractState),
		ST_Account:    new(AccountState),
		ST_AssetState: new(AssetState),
		ST_Storage:    new(StorageItem),
	}
)

func GetStateValue(prefix blockchain.EntryPrefix, data []byte) (IStateValueInterface, error) {
	r := bytes.NewBuffer(data)
	state := StatesMap[prefix]
	if state == nil {
		fmt.Println("StatesMap not has this key", prefix)
		return  nil, errors.New("StatesMap not has key")
	}
	err := state.Deserialize(r)
	if err != nil {
		return nil, err
	}
	return state, nil
}
