package mempool

import (
	"testing"

	"github.com/elastos/Elastos.ELA/p2p/msg"
	"github.com/stretchr/testify/assert"
)

func TestErrToRejectErr(t *testing.T) {
	err := ruleError(ErrTxHashDuplicate, ErrTxHashDuplicate.String())
	code, reason := ErrToRejectErr(err)
	assert.Equal(t, msg.RejectDuplicate, code)
	t.Log(reason)

	err = ruleError(ErrTransactionBalance, ErrTransactionBalance.String())
	code, reason = ErrToRejectErr(err)
	assert.Equal(t, msg.RejectInsufficientFee, code)
	t.Log(reason)
}
