// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//
package process

import (
	"fmt"
	"path"
	"testing"

	genchain "github.com/elastos/Elastos.ELA/benchmark/generator/chain"
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

		b.ResetTimer()
		err := singleBlockGen.Generate(currentHeight + 1)
		b.StopTimer()

		if err != nil {
			b.Error(err)
		}
	})
}

// benchmark about processing single block
//func Benchmark_SingleBlock_Normal_ProcessBlock(b *testing.B) {
//	benchProc(b, func(b *testing.B) {
//		singleBlockGen := newBlockChain()
//		currentHeight := singleBlockGen.GetChain().GetHeight()
//		// set pressure with max block size
//		singleBlockGen.SetPressure(true, 8000000)
//		singleBlockGen.SetGenerateMode(genchain.Normal)
//		singleBlockGen.EnableProcessDataTimer(&genchain.ProcessDataTimeCounter{
//			StartTimer: func() { b.ResetTimer() },
//			StopTimer:  func() { b.StopTimer() },
//		})
//
//		err := singleBlockGen.Generate(currentHeight + 1)
//
//		if err != nil {
//			b.Error(err)
//		}
//	})
//}

// benchmark about storing single block data into database
//func Benchmark_SingleBlock_Fast_StoreBlockOnly(b *testing.B) {
//	benchProc(b, func(b *testing.B) {
//		singleBlockGen := newBlockChain()
//		currentHeight := singleBlockGen.GetChain().GetHeight()
//		// set pressure with max block size
//		singleBlockGen.SetPressure(true, 8000000)
//		singleBlockGen.SetGenerateMode(genchain.Fast)
//		singleBlockGen.EnableProcessDataTimer(&genchain.ProcessDataTimeCounter{
//			StartTimer: func() { b.ResetTimer() },
//			StopTimer:  func() { b.StopTimer() },
//		})
//
//		err := singleBlockGen.Generate(currentHeight + 1)
//
//		if err != nil {
//			b.Error(err)
//		}
//	})
//}

// benchmark about storing single block data into database without add into
// tx pool
//func Benchmark_SingleBlock_Minimal_StoreBlockOnly(b *testing.B) {
//	benchProc(b, func(b *testing.B) {
//		singleBlockGen := newBlockChain()
//		currentHeight := singleBlockGen.GetChain().GetHeight()
//		// set pressure with max block size
//		singleBlockGen.SetPressure(true, 8000000)
//		singleBlockGen.SetGenerateMode(genchain.Minimal)
//		singleBlockGen.SetPrevBlockHash(
//			*singleBlockGen.GetChain().BestChain.Hash)
//		singleBlockGen.EnableProcessDataTimer(&genchain.ProcessDataTimeCounter{
//			StartTimer: func() { b.ResetTimer() },
//			StopTimer:  func() { b.StopTimer() },
//		})
//
//		err := singleBlockGen.Generate(currentHeight + 1)
//
//		if err != nil {
//			b.Error(err)
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
