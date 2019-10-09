// Copyright (c) 2016 The btcsuite developers
// Copyright (c) 2017-2019 Elastos Foundation
// Use of this source code is governed by an ISC
// license that can be found in the LICENSE file.

/*
Package indexers implements optional block chain indexes.
*/
package indexers

import (
	"encoding/binary"
)

var (
	// byteOrder is the preferred byte order used for serializing numeric
	// fields for storage in the database.
	byteOrder = binary.LittleEndian
)
