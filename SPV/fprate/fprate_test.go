package fprate

import (
	"math/rand"
	"testing"

	"github.com/elastos/Elastos.ELA.SPV/util"
)

func TestFpRate_Update(t *testing.T) {

	fpRate := NewFpRate()

	block := &util.Block{
		Header: util.Header{
			NumTxs: 1,
		},
	}
	for i := 0; i < 5000; i++ {
		fp := uint32(rand.Intn(3))
		t.Logf("FpRate %f on height %d, fp %d", fpRate.Update(block, fp), i+1, fp)
	}
}
