// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package common

import (
"testing"
)

func TestUint168FromAddress(t *testing.T) {
	addr1 := ""
	addr2 := "1234567890"
	addr3 := "ELu7YtvBnZdhPHyLyrCxj7XtF7Dfy3neXv"
	uaddrBytes := [21]uint8{33, 41, 20, 84, 22, 115, 80, 220, 110, 100, 5, 154, 52, 53, 130, 37, 190, 132, 189, 129, 113}

	_, err :=  Uint168FromAddress(addr1)
	if err == nil {
		t.Error("Uint168FromAddress failed")
	}
	_, err =  Uint168FromAddress(addr2)
	if err == nil {
		t.Error("Uint168FromAddress failed")
	}
	uaddr, err :=  Uint168FromAddress(addr3)
	if err != nil {
		t.Error("Uint168FromAddress failed")
	}
	if *uaddr != Uint168(uaddrBytes) {
		t.Error("Uint168FromAddress failed")
	}
}