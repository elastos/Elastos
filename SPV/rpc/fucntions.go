package rpc

import (
	"bytes"
	"encoding/hex"

	tx "SPVWallet/core/transaction"
)

func AddToFilter(req Req) Resp {
	data, ok := req.Params["address"]
	if !ok {
		return FunctionError("Address parameter not exist")
	}
	switch data := data.(type) {
	case string:
		addr, err := hex.DecodeString(data)
		if err != nil {
			return FunctionError(err.Error())
		}
		err = listeners.AddToFilter(addr)
		if err != nil {
			return FunctionError(err.Error())
		}
		return Success("Address has add to filter")
	default:
		return FunctionError("Invalid data type")
	}
}

func SendTransaction(req Req) Resp {
	data, ok := req.Params["data"]
	if !ok {
		return FunctionError("Data parameter not exist")
	}
	switch data := data.(type) {
	case string:
		txBytes, err := hex.DecodeString(data)
		if err != nil {
			return FunctionError(err.Error())
		}
		var tx tx.Transaction
		err = tx.Deserialize(bytes.NewReader(txBytes))
		if err != nil {
			return FunctionError("Deserialize transaction failed")
		}
		err = listeners.SendTransaction(tx)
		if err != nil {
			return FunctionError(err.Error())
		}
		return Success(tx.Hash().String())
	default:
		return FunctionError("Invalid data type")
	}
}
