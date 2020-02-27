// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package profile

import (
	"fmt"
	"runtime/debug"
)

func DumpGC() {
	status := debug.GCStats{}
	debug.ReadGCStats(&status)
	fmt.Printf("GC count %d, latst time %d\n",
		status.NumGC, status.LastGC.Nanosecond())
}
