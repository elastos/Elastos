package db

import (
	"sort"

	"SPVWallet/core"
	tx "SPVWallet/core/transaction"
	"fmt"
)

type UTXO struct {
	// Previous txid and output index
	Op tx.OutPoint

	// The higher the better
	Value core.Fixed64

	// The utxo locked height
	LockTime uint32

	// Block height where this tx was confirmed, 0 for unconfirmed
	AtHeight uint32
}

func (utxo *UTXO) String() string {
	return fmt.Sprint(
		"UTXO:{",
		"Op:{TxID:", utxo.Op.TxID.String(), ", Index:", utxo.Op.Index, "},",
		"Value:", utxo.Value.String(), ",",
		"LockTime:", utxo.LockTime, ",",
		"AtHeight:", utxo.AtHeight,
		"}")
}

func (utxo *UTXO) IsEqual(alt *UTXO) bool {
	if alt == nil {
		return utxo == nil
	}

	if !utxo.Op.TxID.IsEqual(&alt.Op.TxID) {
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

	return true
}

type SortableUTXOs []*UTXO

func (utxos SortableUTXOs) Len() int      { return len(utxos) }
func (utxos SortableUTXOs) Swap(i, j int) { utxos[i], utxos[j] = utxos[j], utxos[i] }
func (utxos SortableUTXOs) Less(i, j int) bool {
	if utxos[i].Value > utxos[j].Value {
		return false
	} else {
		return true
	}
}

func SortUTXOs(utxos []*UTXO) []*UTXO {
	sortableUTXOs := SortableUTXOs(utxos)
	sort.Sort(sortableUTXOs)
	return sortableUTXOs
}
