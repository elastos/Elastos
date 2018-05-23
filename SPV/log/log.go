package log

import (
	"fmt"
	"io"
	"log"
	"os"
	"path/filepath"
	"runtime"
	"strconv"
	"time"

	"github.com/elastos/Elastos.ELA.SPV/spvwallet/config"
)

const (
	PATH = "./Logs/"

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
	if level >= LevelTrace {
		pc := make([]uintptr, 10)
		runtime.Callers(2, pc)
		f := runtime.FuncForPC(pc[0])
		file, line := f.FileLine(pc[0])
		fileName := filepath.Base(file)

		msg = append([]interface{}{f.Name(), fileName + ":" + strconv.Itoa(line)}, msg...)

		logger.Output(CallDepth, color(BLUE, "[TRACE]", fmt.Sprintln(msg...)))
	}
}

func Tracef(format string, msg ...interface{}) {
	if level >= LevelTrace {
		pc := make([]uintptr, 10)
		runtime.Callers(2, pc)
		f := runtime.FuncForPC(pc[0])
		file, line := f.FileLine(pc[0])
		fileName := filepath.Base(file)

		msg = append([]interface{}{f.Name(), fileName + ":" + strconv.Itoa(line)}, fmt.Sprintf(format, msg...))

		logger.Output(CallDepth, color(BLUE, "[TRACE]", fmt.Sprintln(msg...)))
	}
}

func Warn(msg ...interface{}) {
	if level >= LevelTrace {
		pc := make([]uintptr, 10)
		runtime.Callers(2, pc)
		f := runtime.FuncForPC(pc[0])
		file, line := f.FileLine(pc[0])
		fileName := filepath.Base(file)

		msg = append([]interface{}{f.Name(), fileName + ":" + strconv.Itoa(line)}, msg...)

		logger.Output(CallDepth, color(BLUE, "[TRACE]", fmt.Sprintln(msg...)))
	}
}

func Warnf(format string, msg ...interface{}) {
	if level >= LevelWarn {
		pc := make([]uintptr, 10)
		runtime.Callers(2, pc)
		f := runtime.FuncForPC(pc[0])
		file, line := f.FileLine(pc[0])
		fileName := filepath.Base(file)

		msg = append([]interface{}{f.Name(), fileName + ":" + strconv.Itoa(line)}, fmt.Sprintf(format, msg...))

		logger.Output(CallDepth, color(YELLOW, "[WARN]", fmt.Sprintln(msg...)))
	}
}

func Error(msg ...interface{}) {
	if level >= LevelError {
		pc := make([]uintptr, 10)
		runtime.Callers(2, pc)
		f := runtime.FuncForPC(pc[0])
		file, line := f.FileLine(pc[0])
		fileName := filepath.Base(file)

		msg = append([]interface{}{f.Name(), fileName + ":" + strconv.Itoa(line)}, msg...)

		logger.Output(CallDepth, color(RED, "[ERROR]", fmt.Sprintln(msg...)))
	}
}

func Errorf(format string, msg ...interface{}) {
	if level >= LevelError {
		pc := make([]uintptr, 10)
		runtime.Callers(2, pc)
		f := runtime.FuncForPC(pc[0])
		file, line := f.FileLine(pc[0])
		fileName := filepath.Base(file)

		msg = append([]interface{}{f.Name(), fileName + ":" + strconv.Itoa(line)}, fmt.Sprintf(format, msg...))

		logger.Output(CallDepth, color(RED, "[ERROR]", fmt.Sprintln(msg...)))
	}
}

func Debug(msg ...interface{}) {
	if level >= LevelDebug {
		pc := make([]uintptr, 10)
		runtime.Callers(2, pc)
		f := runtime.FuncForPC(pc[0])
		file, line := f.FileLine(pc[0])
		fileName := filepath.Base(file)

		msg = append([]interface{}{f.Name(), fileName + ":" + strconv.Itoa(line)}, msg...)

		logger.Output(CallDepth, color(GREEN, "[DEBUG]", fmt.Sprintln(msg...)))
	}
}

func Debugf(format string, msg ...interface{}) {
	if level >= LevelDebug {
		pc := make([]uintptr, 10)
		runtime.Callers(2, pc)
		f := runtime.FuncForPC(pc[0])
		file, line := f.FileLine(pc[0])
		fileName := filepath.Base(file)

		msg = append([]interface{}{f.Name(), fileName + ":" + strconv.Itoa(line)}, fmt.Sprintf(format, msg...))

		logger.Output(CallDepth, color(GREEN, "[DEBUG]", fmt.Sprintln(msg...)))
	}
}

func color(color, level, msg string) string {
	return fmt.Sprintf("\033[%sm%-7s\033[m %s", color, level, msg)
}
