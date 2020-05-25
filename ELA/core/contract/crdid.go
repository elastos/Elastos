// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package contract

import "errors"

func CreateCRIDContractByCode(code []byte) (*Contract, error) {
	if len(code) == 0 {
		return nil, errors.New("code is nil")
	}
	return &Contract{
		Code:   code,
		Prefix: PrefixCRDID,
	}, nil
}
