package process

import (
	"fmt"
	"os"
	"testing"

	genchain "github.com/elastos/Elastos.ELA/benchmark/generator/chain"
	"github.com/elastos/Elastos.ELA/utils/signal"
)

var (
	tempPath     = "run-to-height"
	txRepoParams = genchain.GenerationParams{
		Mode:               genchain.Normal,
		PrepareStartHeight: 100,
		RandomStartHeight:  200,
		InputsPerBlock:     200,
		MaxRefersCount:     200,
		MinRefersCount:     100,
		AddressCount:       100,
	}
	runToHeightGen    = initRunToHeightGen()
	runToHeightParams ProcessParams
)

func Benchmark_RunToHeight_RunToHeight(b *testing.B) {
	LoadParams(&runToHeightParams)

	if err := runToHeightGen.Generate(300); err != nil {
		fmt.Println(err)
	}

	runToHeightGen.Exit()
	os.RemoveAll(tempPath)
}

func initRunToHeightGen() *genchain.DataGen {
	var interrupt = signal.NewInterrupt()
	gen, err := genchain.NewDataGen(tempPath, interrupt.C, &txRepoParams)
	if err != nil {
		fmt.Println(err.Error())
		return nil
	}
	SaveParams(&runToHeightParams)
	return gen
}
