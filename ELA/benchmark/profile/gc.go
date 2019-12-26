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
