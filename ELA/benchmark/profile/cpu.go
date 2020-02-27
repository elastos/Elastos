// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package profile

import (
	"os"
	"runtime/pprof"
)

const (
	cpuDefaultPath = "cpu.out"
)

func BeginCPUProfile(path string) (file *os.File, err error) {
	if file, err = os.Create(path); err != nil {
		return
	}

	err = pprof.StartCPUProfile(file)
	return
}

func EndCPUProfile(file *os.File) error {
	pprof.StopCPUProfile()
	return file.Close()
}
