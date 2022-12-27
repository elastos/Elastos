package spv

import (
	"fmt"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/stretchr/testify/assert"
)

func TestSpvInit(t *testing.T) {
	var programHash common.Uint168
	programHash[0] = byte(contract.PrefixCrossChain)
	for i := 1; i < len(programHash); i++ {
		programHash[i] = 0xff
	}
	fmt.Println(programHash[:])
	address, err := programHash.ToAddress()
	if !assert.NoError(t, err) {
		t.FailNow()
	}

	fmt.Println(address)
	fmt.Println(len(address))

	hash, err := common.Uint168FromAddress(address)
	if !assert.NoError(t, err) {
		t.FailNow()
	}

	addr, err := hash.ToAddress()
	if !assert.NoError(t, err) {
		t.FailNow()
	}

	assert.Equal(t, address, addr)
}
