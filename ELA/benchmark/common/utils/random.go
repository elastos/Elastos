// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package utils

import "crypto/rand"

func RandomBytes(len int) []byte {
	a := make([]byte, len)
	rand.Read(a)
	return a
}
