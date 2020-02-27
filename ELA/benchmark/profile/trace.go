// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package profile

import (
	"os"
	"runtime/trace"
)

const (
	traceDefaultPath = "trace.out"
)

func BeginTrace(path string) (file *os.File, err error) {
	if file, err = os.Create(path); err != nil {
		return
	}

	err = trace.Start(file)
	return
}

func EndTrace(file *os.File) error {
	trace.Stop()
	return file.Close()
}
