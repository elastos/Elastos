// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package addrmgr

import (
	"time"

	"github.com/elastos/Elastos.ELA/p2p"
)

func TstKnownAddressIsBad(ka *KnownAddress) bool {
	return ka.isBad()
}

func TstKnownAddressChance(ka *KnownAddress) float64 {
	return ka.chance()
}

func TstNewKnownAddress(na *p2p.NetAddress, attempts int,
	lastattempt, lastsuccess time.Time, tried bool, refs int) *KnownAddress {
	return &KnownAddress{na: na, attempts: attempts, lastattempt: lastattempt,
		lastsuccess: lastsuccess, tried: tried, refs: refs}
}
