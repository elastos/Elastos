package elalog

import (
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"sync/atomic"
	"time"
)

const (
	KBSize = 1024 << (10 * iota)
	MBSize
	GBSize
)

const (
	defaultMaxFileSize   int64 = 20 * MBSize // 20MB
	defaultMaxFolderSize int64 = 5 * GBSize  // 5GM
)

type fileWriter struct {
	// path is the folder path to put all log files.
	path          string
	maxFileSize   int64
	maxFolderSize int64

	writeChan  chan []byte
	writeReply chan struct{}
}

func (w *fileWriter) Write(buf []byte) (int, error) {
	w.writeChan <- buf
	<-w.writeReply
	return len(buf), nil
}

func (w *fileWriter) writeHandler() {
	var current *os.File
	var fileSize int64
	var folderSize int64

	files, _ := ioutil.ReadDir(w.path)
	for _, f := range files {
		folderSize += f.Size()
	}

	for {
		buf := <-w.writeChan
		var bufLen = int64(len(buf))

		// create new log file if current file is nil or reach max size.
		if atomic.AddInt64(&fileSize, bufLen) >= w.maxFileSize ||
			current == nil {

			// force create new log file
			var err error
			var file *os.File
			for file, err = newLogFile(w.path); err != nil; {
				fmt.Printf("New log file %s, err %v\n", w.path, err)
				file, err = newLogFile(w.path)
			}

			// close previous log file in an other goroutine.
			go func(file *os.File) {
				if file != nil {
					// force close file
					for err := file.Close(); err != nil; {
						err = file.Close()
					}
				}
			}(current)

			current = file
			atomic.StoreInt64(&fileSize, 0)
		}

		// force write buffer to file.
		for _, err := current.Write(buf); err != nil; {
			_, err = current.Write(buf)
		}
		w.writeReply <- struct{}{}

		// check folder size by check buf interval, if folder size
		// reach max size, remove oldest log file.
		if atomic.AddInt64(&folderSize, bufLen) >= w.maxFolderSize {
			var total int64
			files, _ := ioutil.ReadDir(w.path)
			for _, f := range files {
				total += f.Size()
			}

			// Get the oldest log file
			file := files[0]
			// Remove it
			err := os.Remove(filepath.Join(w.path, file.Name()))
			if err != nil {
				fmt.Println("remove file error:", err)
				atomic.StoreInt64(&folderSize, total)
			} else {
				atomic.StoreInt64(&folderSize, total-file.Size())
			}
		}
	}
}

func newLogFile(path string) (*os.File, error) {
	if dir, err := os.Stat(path); err == nil {
		if !dir.IsDir() {
			return nil, fmt.Errorf("open %s: not a directory", path)
		}

	} else if os.IsNotExist(err) {
		if err := os.MkdirAll(path, 0775); err != nil {
			return nil, err
		}

	} else {
		return nil, err
	}

	return os.OpenFile(filepath.Join(path,
		time.Now().Format("2006-01-02_15.04.05"))+".log",
		os.O_RDWR|os.O_CREATE, 0664)
}

func NewFileWriter(path string, maxFileSize, maxFolderSize int64) *fileWriter {
	w := fileWriter{
		path:          path,
		maxFileSize:   defaultMaxFileSize,
		maxFolderSize: defaultMaxFolderSize,
		writeChan:     make(chan []byte, 1),
		writeReply:    make(chan struct{}),
	}

	if maxFileSize > 0 {
		w.maxFileSize = maxFileSize
	}
	if maxFolderSize > 0 {
		w.maxFolderSize = maxFolderSize
	}

	go w.writeHandler()

	return &w
}
