package contract

import "errors"

func CreateCRDIDContractByCode(code []byte) (*Contract, error) {
	if len(code) == 0 {
		return nil, errors.New("code is nil")
	}
	return &Contract{
		Code:   code,
		Prefix: PrefixCRDID,
	}, nil
}
