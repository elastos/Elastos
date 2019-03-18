package main

import (
	"math/rand"
	"time"

	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
)

func init() {
	log.NewDefault(
		config.Template.PrintLevel,
		config.Template.MaxPerLogSize,
		config.Template.MaxLogsSize,
	)

	//seed transaction nonce
	rand.Seed(time.Now().UnixNano())
}
