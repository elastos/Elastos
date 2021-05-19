package mempool

import (
	"testing"

	"github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/p2p/msg"
	"github.com/stretchr/testify/assert"
)

func TestErrToRejectErr(t *testing.T) {
	code, reason := ErrToRejectErr(errors.ErrTransactionDuplicate)
	assert.Equal(t, msg.RejectDuplicate, code)
	t.Log(reason)

	code, reason = ErrToRejectErr(errors.ErrTransactionBalance)
	assert.Equal(t, msg.RejectInsufficientFee, code)
	t.Log(reason)
}
