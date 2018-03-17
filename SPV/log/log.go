package log

import (
	"io"
	"os"
	"log"
	"fmt"
	"time"
)

const (
	PATH = "./Log/"

	CallDepth = 4

	BLUE  = "0;34"
	RED   = "0;31"
	GREEN = "0;32"
)

var logger *log.Logger

func Init() {
	writers := []io.Writer{}
	//logFile, err := OpenLogFile()
	//if err != nil {
	//	fmt.Println("error: open log file failed")
	//	os.Exit(1)
	//}
	//writers = append(writers, logFile)
	writers = append(writers, os.Stdout)
	logger = log.New(io.MultiWriter(writers...), "", log.Ldate|log.Lmicroseconds)
}

func OpenLogFile() (*os.File, error) {
	if fi, err := os.Stat(PATH); err == nil {
		if !fi.IsDir() {
			return nil, fmt.Errorf("open %s: not a directory", PATH)
		}
	} else if os.IsNotExist(err) {
		if err := os.MkdirAll(PATH, 0766); err != nil {
			return nil, err
		}
	} else {
		return nil, err
	}

	current := time.Now().Format("2006-01-02_15.04.05")
	logfile, err := os.OpenFile(PATH+current+"_LOG.log", os.O_RDWR|os.O_CREATE, 0666)
	if err != nil {
		return nil, err
	}
	return logfile, nil
}

func Info(msg ...interface{}) {
	logger.Output(CallDepth, color(GREEN, fmt.Sprint(msg)))
}

func Infof(format string, msg ...interface{}) {
	logger.Output(CallDepth, color(GREEN, fmt.Sprintf(format, msg)))
}

func Trace(msg ...interface{}) {
	logger.Output(CallDepth, color(BLUE, fmt.Sprint(msg)))
}

func Tracef(format string, msg ...interface{}) {
	logger.Output(CallDepth, color(BLUE, fmt.Sprintf(format, msg)))
}

func Error(msg ...interface{}) {
	logger.Output(CallDepth, color(RED, fmt.Sprint(msg)))
}

func Errorf(format string, msg ...interface{}) {
	logger.Output(CallDepth, color(RED, fmt.Sprintf(format, msg)))
}

func color(code, msg string) string {
	return fmt.Sprintf("\033[%sm%s\033[m", code, msg)
}
