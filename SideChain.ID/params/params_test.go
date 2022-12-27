package params

import (
	"fmt"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/stretchr/testify/assert"
)

func TestParams(t *testing.T) {

	address, _ := common.Uint168FromAddress("8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta")

	str := ""
	for _, b := range address {
		str += fmt.Sprintf("0x%x, ", b)
	}
	t.Log(str)

	address, _ = common.Uint168FromAddress("8NRxtbMKScEWzW8gmPDGUZ8LSzm688nkZZ")

	str = ""
	for _, b := range address {
		str += fmt.Sprintf("0x%x, ", b)
	}
	t.Log(str)

	addr, err := mainNetFoundation.ToAddress()
	assert.NoError(t, err)
	assert.Equal(t, "8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta", addr)
	t.Log(addr)

	addr, err = testNetFoundation.ToAddress()
	assert.NoError(t, err)
	assert.Equal(t, "8NRxtbMKScEWzW8gmPDGUZ8LSzm688nkZZ", addr)
	t.Log(addr)
}
