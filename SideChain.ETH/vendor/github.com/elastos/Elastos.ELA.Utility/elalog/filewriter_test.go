package elalog

import (
	"io"
	"os"
	"testing"
	"time"
)

func TestFileWriter(t *testing.T) {
	w := NewFileWriter("./logs", 2*MBSize, 10*MBSize)

	backend := NewBackend(io.MultiWriter(w, os.Stdout), Llongfile)
	logger := backend.Logger("test", LevelInfo)

	var total int64
	for {
		str := time.Now().String()
		logger.Info(str)

		time.Sleep(time.Microsecond*5)
		total += int64(len(str))

		if total >= 10*MBSize {
			break
		}
	}
}
