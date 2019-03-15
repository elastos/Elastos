package bloom

import (
	"bytes"
	"fmt"

	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/elanet/filter"
	"github.com/elastos/Elastos.ELA/p2p/msg"
)

type TxFilter struct {
	filter *Filter
}

func (f *TxFilter) Load(filter []byte) error {
	var fl msg.FilterLoad
	err := fl.Deserialize(bytes.NewReader(filter))
	if err != nil {
		return err
	}

	f.filter = LoadFilter(&fl)

	return nil
}

func (f *TxFilter) Add(filter []byte) error {
	if f.filter == nil || !f.filter.IsLoaded() {
		return fmt.Errorf("filter not loaded")
	}

	f.filter.Add(filter)

	return nil
}

func (f *TxFilter) MatchConfirmed(tx *types.Transaction) bool {
	return f.filter.MatchTxAndUpdate(tx)
}

func (f *TxFilter) MatchUnconfirmed(tx *types.Transaction) bool {
	return f.filter.MatchTxAndUpdate(tx)
}

func NewTxFilter() filter.TxFilter {
	return &TxFilter{}
}
