// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package outputpayload

import (
	"io"
)

type DefaultOutput struct {
}

func (o *DefaultOutput) Data() []byte {
	return nil
}

func (o *DefaultOutput) Serialize(w io.Writer) error {
	return nil
}

func (o *DefaultOutput) Deserialize(r io.Reader) error {
	return nil
}

func (o *DefaultOutput) GetVersion() byte {
	return 0
}

func (o *DefaultOutput) Validate() error {
	return nil
}
