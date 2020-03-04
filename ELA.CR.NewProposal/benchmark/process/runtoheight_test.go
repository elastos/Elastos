// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package process

import (
	"fmt"
	"os"
	"testing"

	genchain "github.com/elastos/Elastos.ELA/benchmark/tools/generator/chain"
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
)

func Benchmark_RunToHeight_RunToHeight(b *testing.B) {
	benchProc(b, func(b *testing.B) {
		runToHeightGen := initRunToHeightGen()

		b.ResetTimer()
		if err := runToHeightGen.Generate(300); err != nil {
			fmt.Println(err)
		}
		b.StopTimer()

		runToHeightGen.Exit()
		os.RemoveAll(tempPath)
	})
}

func initRunToHeightGen() *genchain.DataGen {
	var interrupt = signal.NewInterrupt()
	gen, err := genchain.NewDataGen(tempPath, interrupt.C, &txRepoParams)
	if err != nil {
		fmt.Println(err.Error())
		return nil
	}
	return gen
}
