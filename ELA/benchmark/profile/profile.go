// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package profile

import "os"

type Item struct {
	Enable bool
	Path   string
}

type Params struct {
	CPU    Item
	Memory Item
	Trace  Item
}

type Files struct {
	CPU    *os.File
	Memory *os.File
	Trace  *os.File
}

func Default() *Params {
	return &Params{
		CPU: Item{
			Enable: true,
			Path:   cpuDefaultPath,
		},
		Memory: Item{
			Enable: false,
		},
		Trace: Item{
			Enable: true,
			Path:   traceDefaultPath,
		},
	}
}

func BeginProfile(params *Params) (files *Files, err error) {
	files = &Files{}

	var file *os.File
	if params.Trace.Enable {
		if file, err = BeginTrace(params.Trace.Path); err != nil {
			return
		}
		files.Trace = file
	}

	if params.CPU.Enable {
		if file, err = BeginCPUProfile(params.CPU.Path); err != nil {
			return
		}
		files.CPU = file
	}

	return
}

func EndProfile(params *Params, files *Files) error {
	if params.Trace.Enable {
		if err := EndTrace(files.Trace); err != nil {
			return err
		}
	}
	if params.CPU.Enable {
		if err := EndCPUProfile(files.CPU); err != nil {
			return err
		}
	}

	return nil
}
