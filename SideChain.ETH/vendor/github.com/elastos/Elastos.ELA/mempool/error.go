package mempool

import (
	"github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/p2p/msg"
)

// extractRejectCode attempts to return a relevant reject code for a given error
// by examining the error for known types.  It will return true if a code
// was successfully extracted.
func extractRejectCode(err error) (msg.RejectCode, bool) {
	// Pull the underlying error out of a RuleError.
	errCode, ok := err.(errors.ErrCode)
	if !ok {
		return msg.RejectInvalid, false
	}

	var code msg.RejectCode
	switch errCode {
	case errors.ErrTransactionDuplicate:
		code = msg.RejectDuplicate

	case errors.ErrTransactionBalance:
		code = msg.RejectInsufficientFee

	default:
		return msg.RejectInvalid, false
	}

	return code, true
}

// ErrToRejectErr examines the underlying type of the error and returns a reject
// code and string appropriate to be sent in a wire.MsgReject message.
func ErrToRejectErr(err error) (msg.RejectCode, string) {
	// Return the reject code along with the error text if it can be
	// extracted from the error.
	rejectCode, found := extractRejectCode(err)
	if found {
		return rejectCode, err.Error()
	}

	// Return a generic rejected string if there is no error.  This really
	// should not happen unless the code elsewhere is not setting an error
	// as it should be, but it's best to be safe and simply return a generic
	// string rather than allowing the following code that dereferences the
	// err to panic.
	if err == nil {
		return msg.RejectInvalid, "rejected"
	}

	// When the underlying error is not one of the above cases, just return
	// wire.RejectInvalid with a generic rejected string plus the error
	// text.
	return msg.RejectInvalid, "rejected: " + err.Error()
}
