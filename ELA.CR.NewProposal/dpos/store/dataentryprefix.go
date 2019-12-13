// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package store

// DataEntryPrefix
type DataEntryPrefix byte

const (
	// DPOS
	DPOSCheckPointHeights  DataEntryPrefix = 0x10
	DPOSSingleCheckPoint   DataEntryPrefix = 0x11
)
