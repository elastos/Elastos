// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//
package process

import (
	"fmt"
	"path"
	"testing"

	"github.com/elastos/Elastos.ELA/benchmark/profile"
	genchain "github.com/elastos/Elastos.ELA/benchmark/tools/generator/chain"
	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/elanet/pact"
	"github.com/elastos/Elastos.ELA/utils/test"
)

type ProcessParams struct {
	Ledger            *blockchain.Ledger
	FoundationAddress common.Uint168
	MaxTxPerBlock     uint32
}

// benchmark about processing single block and add tx into tx pool
func Benchmark_SingleBlock_Normal_GenerateBlock(b *testing.B) {
	benchProc(b, func(b *testing.B) {
		singleBlockGen := newBlockChain()
		currentHeight := singleBlockGen.GetChain().GetHeight()
		// set pressure with max block size
		singleBlockGen.SetPressure(true, 8000000)
		singleBlockGen.SetGenerateMode(genchain.Normal)

		files, _ := beginProfile(b)
		if err := singleBlockGen.Generate(currentHeight + 1); err != nil {
			b.Error(err)
			return
		}
		endProfile(b, files)
	})
}

// benchmark about processing single block
//func Benchmark_SingleBlock_Normal_ProcessBlock(b *testing.B) {
//	benchProc(b, func(b *testing.B) {
//		var files *profile.Files
//
//		singleBlockGen := newBlockChain()
//		currentHeight := singleBlockGen.GetChain().GetHeight()
//		// set pressure with max block size
//		singleBlockGen.SetPressure(true, 8000000)
//		singleBlockGen.SetGenerateMode(genchain.Normal)
//		singleBlockGen.EnableProcessDataTimer(&genchain.TimeCounter{
//			StartTimer: func() { files, _ = beginProfile(b) },
//			StopTimer:  func() { endProfile(b, files) },
//		})
//
//		if err := singleBlockGen.Generate(currentHeight + 1); err != nil {
//			b.Error(err)
//		}
//	})
//}

// benchmark about storing single block data into database
//func Benchmark_SingleBlock_Fast_StoreBlockOnly(b *testing.B) {
//	benchProc(b, func(b *testing.B) {
//		var files *profile.Files
//
//		singleBlockGen := newBlockChain()
//		currentHeight := singleBlockGen.GetChain().GetHeight()
//		// set pressure with max block size
//		singleBlockGen.SetPressure(true, 8000000)
//		singleBlockGen.SetGenerateMode(genchain.Fast)
//		singleBlockGen.EnableProcessDataTimer(&genchain.TimeCounter{
//			StartTimer: func() { files, _ = beginProfile(b) },
//			StopTimer:  func() { endProfile(b, files) },
//		})
//
//		if err := singleBlockGen.Generate(currentHeight + 1); err != nil {
//			b.Error(err)
//		}
//	})
//}

// benchmark about storing single block data into database without add into
// tx pool
//func Benchmark_SingleBlock_Minimal_StoreBlockOnly(b *testing.B) {
//	benchProc(b, func(b *testing.B) {
//		var files *profile.Files
//
//		singleBlockGen := newBlockChain()
//		currentHeight := singleBlockGen.GetChain().GetHeight()
//		// set pressure with max block size
//		singleBlockGen.SetPressure(true, 8000000)
//		singleBlockGen.SetGenerateMode(genchain.Minimal)
//		singleBlockGen.SetPrevBlockHash(
//			*singleBlockGen.GetChain().BestChain.Hash)
//		singleBlockGen.EnableProcessDataTimer(&genchain.TimeCounter{
//			StartTimer: func() { files, _ = beginProfile(b) },
//			StopTimer:  func() { endProfile(b, files) },
//		})
//
//		if err := singleBlockGen.Generate(currentHeight + 1); err != nil {
//			b.Error(err)
//		}
//	})
//}

// benchmark about add to tx pool only
//func Benchmark_SingleBlock_AddToTxPool(b *testing.B) {
//	benchProc(b, func(b *testing.B) {
//		var files *profile.Files
//		singleBlockGen := newBlockChain()
//		currentHeight := singleBlockGen.GetChain().GetHeight()
//		// set pressure with max block size
//		singleBlockGen.SetPressure(true, 8000000)
//		singleBlockGen.SetGenerateMode(genchain.Normal)
//		singleBlockGen.EnableAddToTxPoolTimer(&genchain.TimeCounter{
//			StartTimer: func() { files, _ = beginProfile(b) },
//			StopTimer:  func() { endProfile(b, files) },
//		})
//
//		if err := singleBlockGen.Generate(currentHeight + 1); err != nil {
//			b.Error(err)
//			return
//		}
//	})
//}

func newBlockChain() *genchain.DataGen {
	gen, err := genchain.LoadDataGen(
		path.Join(test.DataDir, genchain.TxRepoFile))
	if err != nil {
		fmt.Println(err.Error())
		return nil
	}
	return gen
}

func benchProc(b *testing.B, action func(b *testing.B)) {
	params := beginBench()
	for i := 0; i < b.N; i++ {
		action(b)
	}
	blockchain.DefaultLedger.Store.Close()
	endBench(params)
}

func beginBench() *ProcessParams {
	params := &ProcessParams{
		Ledger:            blockchain.DefaultLedger,
		FoundationAddress: blockchain.FoundationAddress,
		MaxTxPerBlock:     pact.MaxTxPerBlock,
	}
	pact.MaxTxPerBlock = 100000
	return params
}

func endBench(params *ProcessParams) {
	blockchain.DefaultLedger = params.Ledger
	blockchain.FoundationAddress = params.FoundationAddress
	pact.MaxTxPerBlock = params.MaxTxPerBlock
}

func beginProfile(b *testing.B) (files *profile.Files, err error) {
	files, err = profile.BeginProfile(profile.Default())
	if err != nil {
		return
	}

	b.ResetTimer()
	return
}

func endProfile(b *testing.B, files *profile.Files) error {
	if err := profile.EndProfile(profile.Default(), files); err != nil {
		return err
	}

	b.StopTimer()
	profile.DumpGC()
	return nil
}
