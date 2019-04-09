package main

import (
	"fmt"

	"github.com/elastos/Elastos.ELA.SPV/wallet"

	"github.com/elastos/Elastos.ELA/common/config"
)

var Version string

func main() {
	url := fmt.Sprint("http://127.0.0.1:", cfg.JsonRpcPort, "/spvwallet")
	wallet.RunClient(Version, dataDir, url, config.ELAAssetID)
}
