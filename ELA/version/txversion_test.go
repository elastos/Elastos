package version

import (
	"testing"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/stretchr/testify/assert"
)

func TestCheckOutputProgramHash(t *testing.T) {
	programHash := common.Uint168{}

	txVCurrent := TxVersionMain{}
	// empty program hash should pass
	assert.Equal(t, true, txVCurrent.CheckOutputProgramHash(programHash))

	// prefix standard program hash should pass
	programHash[0] = common.PrefixStandard
	assert.Equal(t, true, txVCurrent.CheckOutputProgramHash(programHash))

	// prefix multisig program hash should pass
	programHash[0] = common.PrefixMultisig
	assert.Equal(t, true, txVCurrent.CheckOutputProgramHash(programHash))

	// prefix crosschain program hash should pass
	programHash[0] = common.PrefixCrossChain
	assert.Equal(t, true, txVCurrent.CheckOutputProgramHash(programHash))

	// other prefix program hash should not pass
	programHash[0] = 0x34
	assert.Equal(t, false, txVCurrent.CheckOutputProgramHash(programHash))

	t.Log("[TestCheckOutputProgramHash] PASSED")
}
