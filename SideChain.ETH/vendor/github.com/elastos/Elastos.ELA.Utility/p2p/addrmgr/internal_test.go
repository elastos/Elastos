package addrmgr

import (
	"time"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
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
