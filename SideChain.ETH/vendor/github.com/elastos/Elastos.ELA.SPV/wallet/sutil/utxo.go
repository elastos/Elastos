package sutil

import (
	"fmt"
	"sort"

	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/elastos/Elastos.ELA/common"
)

type UTXO struct {
	// Previous txid and output index
	Op *util.OutPoint

	// The higher the better
	Value common.Fixed64

	// The utxo locked height
	LockTime uint32

	// Block height where this tx was confirmed, 0 for unconfirmed
	AtHeight uint32

	// Address where this UTXO belongs to.
	Address common.Uint168
}

func (utxo *UTXO) String() string {
	return fmt.Sprint(
		"UTXO:{",
		"Op:{TxId:", utxo.Op.TxID.String(), ", Index:", utxo.Op.Index, "},",
		"Value:", utxo.Value.String(), ",",
		"LockTime:", utxo.LockTime, ",",
		"AtHeight:", utxo.AtHeight,
		"}")
}

func (utxo *UTXO) IsEqual(alt *UTXO) bool {
	if alt == nil {
		return utxo == nil
	}

	if !utxo.Op.TxID.IsEqual(alt.Op.TxID) {
		return false
	}

	if utxo.Op.Index != alt.Op.Index {
		return false
	}

	if utxo.Value != alt.Value {
		return false
	}

	if utxo.AtHeight != alt.AtHeight {
		return false
	}

	if !utxo.Address.IsEqual(alt.Address) {
		return false
	}

	return true
}

func NewUTXO(txId common.Uint256, height uint32, index int,
	value common.Fixed64, lockTime uint32, address common.Uint168) *UTXO {
	utxo := new(UTXO)
	utxo.Op = util.NewOutPoint(txId, uint16(index))
	utxo.Value = value
	utxo.LockTime = lockTime
	utxo.AtHeight = height
	utxo.Address = address
	return utxo
}

type SortByValueASC []*UTXO

func (utxos SortByValueASC) Len() int      { return len(utxos) }
func (utxos SortByValueASC) Swap(i, j int) { utxos[i], utxos[j] = utxos[j], utxos[i] }
func (utxos SortByValueASC) Less(i, j int) bool {
	if utxos[i].Value > utxos[j].Value {
		return false
	} else {
		return true
	}
}

func SortByValue(utxos []*UTXO) []*UTXO {
	sortableUTXOs := SortByValueASC(utxos)
	sort.Sort(sortableUTXOs)
	return sortableUTXOs
}
