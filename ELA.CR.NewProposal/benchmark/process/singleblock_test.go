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
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/utils/test"
)

var (
	datagen = newBlockChain()
)

func BenchmarkBlockChain_ProcessBlock(b *testing.B) {
	currentHeight := datagen.GetChain().GetHeight()
	err := datagen.Generate(currentHeight + 1)
	if err != nil {
		b.Error(err)
	}
}

func newBlockChain() *genchain.DataGen {
	log.NewDefault(test.NodeLogPath, 0, 0, 0)
	gen, err := genchain.LoadDataGen(
		path.Join(test.DataDir, genchain.TxRepoFile))
	if err != nil {
		fmt.Println(err.Error())
		return nil
	}
	return gen
}
