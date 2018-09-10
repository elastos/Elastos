package log

import (
	"bytes"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"
	"runtime"
	"sort"
	"strconv"
	"strings"
	"sync"
	"time"

	"github.com/AlexpanXX/fsnotify"
)

const (
	White  = "1;00"
	Blue   = "1;34"
	Red    = "1;31"
	Green  = "1;32"
	Yellow = "1;33"
	Cyan   = "1;36"
	Pink   = "1;35"
)

func Color(code, msg string) string {
	return fmt.Sprintf("\033[%sm%s\033[m", code, msg)
}

const (
	infoLog = iota
	warnLog
	errorLog
	fatalLog
	traceLog
	debugLog
	maxLevelLog
)

var (
	levels = map[int]string{
		infoLog:  Color(White, "[INFO]"),
		warnLog:  Color(Yellow, "[WARN]"),
		errorLog: Color(Red, "[ERROR]"),
		fatalLog: Color(Pink, "[FATAL]"),
		traceLog: Color(Cyan, "[TRACE]"),
		debugLog: Color(Green, "[DEBUG]"),
	}
)

const (
	namePrefix           = "LEVEL"
	callDepth            = 2
	KB_SIZE              = int64(1024)
	MB_SIZE              = KB_SIZE * 1024
	GB_SIZE              = MB_SIZE * 1024
	defaultMaxPerLogSize = 20 * MB_SIZE
	defaultMaxLogsSize   = 5 * GB_SIZE
)

func GetGID() uint64 {
	var buf [64]byte
	b := buf[:runtime.Stack(buf[:], false)]
	b = bytes.TrimPrefix(b, []byte("goroutine "))
	b = b[:bytes.IndexByte(b, ' ')]
	n, _ := strconv.ParseUint(string(b), 10, 64)
	return n
}

func LevelName(level int) string {
	if name, ok := levels[level]; ok {
		return name
	}
	return namePrefix + strconv.Itoa(level)
}

type Logger struct {
	path        string
	level       int   // The log print level
	maxLogsSize int64 // The max logs total size

	// Current log file and printer
	mutex         sync.Mutex
	maxPerLogSize int64
	file          *os.File
	logger        *log.Logger
	watcher       *fsnotify.Watcher
}

func (l *Logger) Level() string {
	return LevelName(l.level)
}

func (l *Logger) init() {
	// setup file watcher for the printing log file,
	// watch the file size change and trigger new log
	// file create when the MaxPerLogSize limit reached.
	var err error
	l.watcher, err = fsnotify.NewWatcher()
	if err != nil {
		fmt.Println("create log file watcher failed,", err)
		os.Exit(-1)
	}
	go func() {
		for {
			select {
			case event := <-l.watcher.Events:
				l.handleFileEvents(event)
			case err := <-l.watcher.Errors:
				fmt.Println("error:", err.Error())
			}
		}
	}()

	// create new log file
	l.newLogFile()
}

func (l *Logger) prune() {
	// load the file list under logs output path
	// so we can delete the oldest log file when
	// the MaxLogsSize limit reached.
	fileList, err := ioutil.ReadDir(l.path)
	if err != nil {
		fmt.Println("read logs path failed,", err)
	}
	SortLogFiles(fileList)

	// calculate total size
	var totalSize int64
	for _, f := range fileList {
		totalSize += f.Size()
	}
	for totalSize >= l.maxLogsSize {
		// Get the oldest log file
		file := fileList[0]
		// Remove it
		os.Remove(l.path + file.Name())
		fileList = fileList[1:]
		// Update logs size
		totalSize -= file.Size()
	}
}

func (l *Logger) newLogFile() {
	// prune before create a new log file
	l.prune()

	// create new log file
	var err error
	l.file, err = newLogFile(l.path)
	if err != nil {
		fmt.Print("create log file failed,", err.Error())
		os.Exit(-1)
	}

	// get file stat
	info, err := l.file.Stat()
	if err != nil {
		fmt.Print("get log file stat failed,", err.Error())
		os.Exit(-1)
	}

	// setup new printer
	l.logger = log.New(io.MultiWriter(os.Stdout, l.file), "", log.Ldate|log.Lmicroseconds)

	// watch log file change
	l.watcher.Add(l.path + info.Name())
}

func (l *Logger) handleFileEvents(event fsnotify.Event) {
	switch event.Op {
	case fsnotify.Write:
		info, _ := l.file.Stat()
		if info.Size() >= l.maxPerLogSize {
			l.mutex.Lock()
			// close previous log file
			l.file.Close()
			// unwatch it
			l.watcher.Remove(l.path + info.Name())
			// create a new log file
			l.newLogFile()
			l.mutex.Unlock()
		}
	}
}

func NewLogger(path string, level int, maxPerLogSizeMb, maxLogsSizeMb int64) *Logger {
	logger := new(Logger)
	logger.path = path
	logger.level = level

	if maxPerLogSizeMb != 0 {
		logger.maxPerLogSize = maxPerLogSizeMb * MB_SIZE
	} else {
		logger.maxPerLogSize = defaultMaxPerLogSize
	}

	if maxLogsSizeMb != 0 {
		logger.maxLogsSize = maxLogsSizeMb * MB_SIZE
	} else {
		logger.maxLogsSize = defaultMaxLogsSize
	}

	logger.init()
	return logger
}

func newLogFile(path string) (*os.File, error) {
	if fi, err := os.Stat(path); err == nil {
		if !fi.IsDir() {
			return nil, fmt.Errorf("open %s: not a directory", path)
		}
	} else if os.IsNotExist(err) {
		if err := os.MkdirAll(path, 0766); err != nil {
			return nil, err
		}
	} else {
		return nil, err
	}

	var timestamp = time.Now().Format("2006-01-02_15.04.05")

	file, err := os.OpenFile(path+timestamp+"_LOG.log", os.O_RDWR|os.O_CREATE, 0666)
	if err != nil {
		return nil, err
	}
	return file, nil
}

func SortLogFiles(files []os.FileInfo) {
	sort.Sort(byTime(files))
}

type byTime []os.FileInfo

func (f byTime) Len() int           { return len(f) }
func (f byTime) Less(i, j int) bool { return f[i].Name() < f[j].Name() }
func (f byTime) Swap(i, j int)      { f[i], f[j] = f[j], f[i] }

func (l *Logger) SetPrintLevel(level int) error {
	if level > maxLevelLog || level < 0 {
		return errors.New("Invalid Debug Level")
	}

	l.level = level
	return nil
}

func (l *Logger) Output(level int, a ...interface{}) error {
	l.mutex.Lock()
	defer l.mutex.Unlock()
	if l.level >= level {
		gidStr := strconv.FormatUint(GetGID(), 10)
		a = append([]interface{}{LevelName(level), "GID", gidStr + ","}, a...)
		return l.logger.Output(callDepth, fmt.Sprintln(a...))
	}
	return nil
}

func (l *Logger) Outputf(level int, format string, v ...interface{}) error {
	l.mutex.Lock()
	defer l.mutex.Unlock()
	if l.level >= level {
		v = append([]interface{}{LevelName(level), "GID", GetGID()}, v...)
		return l.logger.Output(callDepth, fmt.Sprintf("%s %s %d, "+format+"\n", v...))
	}
	return nil
}

func (l *Logger) Trace(a ...interface{}) {
	if l.level < traceLog {
		return
	}

	pc := make([]uintptr, 10)
	runtime.Callers(2, pc)
	f := runtime.FuncForPC(pc[0])
	file, line := f.FileLine(pc[0])
	fileName := filepath.Base(file)

	nameFull := f.Name()
	nameEnd := filepath.Ext(nameFull)
	funcName := strings.TrimPrefix(nameEnd, ".")

	a = append([]interface{}{funcName + "()", fileName + ":" + strconv.Itoa(line)}, a...)

	l.Output(traceLog, a...)
}

func (l *Logger) Tracef(format string, a ...interface{}) {
	if l.level < traceLog {
		return
	}

	pc := make([]uintptr, 10)
	runtime.Callers(2, pc)
	f := runtime.FuncForPC(pc[0])
	file, line := f.FileLine(pc[0])
	fileName := filepath.Base(file)

	nameFull := f.Name()
	nameEnd := filepath.Ext(nameFull)
	funcName := strings.TrimPrefix(nameEnd, ".")

	a = append([]interface{}{funcName, fileName, line}, a...)

	l.Outputf(traceLog, "%s() %s:%d "+format, a...)
}

func (l *Logger) Debug(a ...interface{}) {
	if l.level < debugLog {
		return
	}

	pc := make([]uintptr, 10)
	runtime.Callers(2, pc)
	f := runtime.FuncForPC(pc[0])
	file, line := f.FileLine(pc[0])
	fileName := filepath.Base(file)

	a = append([]interface{}{f.Name(), fileName + ":" + strconv.Itoa(line)}, a...)

	l.Output(debugLog, a...)
}

func (l *Logger) Debugf(format string, a ...interface{}) {
	if l.level < debugLog {
		return
	}

	pc := make([]uintptr, 10)
	runtime.Callers(2, pc)
	f := runtime.FuncForPC(pc[0])
	file, line := f.FileLine(pc[0])
	fileName := filepath.Base(file)

	a = append([]interface{}{f.Name(), fileName, line}, a...)

	l.Outputf(debugLog, "%s %s:%d "+format, a...)
}

func (l *Logger) Info(a ...interface{}) {
	l.Output(infoLog, a...)
}

func (l *Logger) Infof(format string, a ...interface{}) {
	l.Outputf(infoLog, format, a...)
}

func (l *Logger) Warn(a ...interface{}) {
	l.Output(warnLog, a...)
}

func (l *Logger) Warnf(format string, a ...interface{}) {
	l.Outputf(warnLog, format, a...)
}

func (l *Logger) Error(a ...interface{}) {
	l.Output(errorLog, a...)
}

func (l *Logger) Errorf(format string, a ...interface{}) {
	l.Outputf(errorLog, format, a...)
}

func (l *Logger) Fatal(a ...interface{}) {
	l.Output(fatalLog, a...)
}

func (l *Logger) Fatalf(format string, a ...interface{}) {
	l.Outputf(fatalLog, format, a...)
}
