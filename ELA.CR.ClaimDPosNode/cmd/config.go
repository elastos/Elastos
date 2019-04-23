package main

import (
	"math/rand"
	"time"

	"github.com/elastos/Elastos.ELA/common/log"
)

func init() {
	log.NewDefault(0, 0, 0)

	//seed transaction nonce
	rand.Seed(time.Now().UnixNano())
}
