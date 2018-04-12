package log

import (
	"io"
	"os"
	"log"
	"fmt"
	"time"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/config"
)

const (
	PATH = "./Log/"

	CallDepth = 4

	WHITE  = "0;0"
	BLUE   = "0;34"
	RED    = "0;31"
	GREEN  = "0;32"
	YELLOW = "0;33"
)

const (
	LevelTrace = 1
	LevelWarn  = 2
	LevelError = 3
	LevelDebug = 4
	LevelFile  = 5
)

var level uint8
var logger *log.Logger

func Init() {
	writers := []io.Writer{}
	level = config.Values().PrintLevel
	if level >= LevelFile {
		logFile, err := OpenLogFile()
		if err != nil {
			fmt.Println("error: open log file failed")
			os.Exit(1)
		}
		writers = append(writers, logFile)
	}
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
	Infof("%s", fmt.Sprint(msg...))
}

func Infof(format string, msg ...interface{}) {
	logger.Output(CallDepth, color(WHITE, "[INFO]", fmt.Sprintf(format, msg...)))
}

func Trace(msg ...interface{}) {
	Tracef("%s", fmt.Sprint(msg...))
}

func Tracef(format string, msg ...interface{}) {
	if level >= LevelTrace {
		logger.Output(CallDepth, color(BLUE, "[TRACE]", fmt.Sprintf(format, msg...)))
	}
}

func Warn(msg ...interface{}) {
	Warnf("%s", fmt.Sprint(msg...))
}

func Warnf(format string, msg ...interface{}) {
	if level >= LevelWarn {
		logger.Output(CallDepth, color(YELLOW, "[WARN]", fmt.Sprintf(format, msg...)))
	}
}

func Error(msg ...interface{}) {
	Errorf("%s", fmt.Sprint(msg...))
}

func Errorf(format string, msg ...interface{}) {
	if level >= LevelError {
		logger.Output(CallDepth, color(RED, "[ERROR]", fmt.Sprintf(format, msg...)))
	}
}

func Debug(msg ...interface{}) {
	Debugf("%s", fmt.Sprint(msg...))
}

func Debugf(format string, msg ...interface{}) {
	if level >= LevelDebug {
		logger.Output(CallDepth, color(GREEN, "[DEBUG]", fmt.Sprintf(format, msg...)))
	}
}

func color(color, level, msg string) string {
	return fmt.Sprintf("\033[%sm%-7s\033[m %s", color, level, msg)
}
