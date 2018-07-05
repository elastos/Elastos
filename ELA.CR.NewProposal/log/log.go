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
	Blue   = "0;34"
	Red    = "0;31"
	Green  = "0;32"
	Yellow = "0;33"
	Cyan   = "0;36"
	Pink   = "1;35"
)

func Color(code, msg string) string {
	return fmt.Sprintf("\033[%sm%s\033[m", code, msg)
}

const (
	debugLog    = iota
	infoLog
	warnLog
	errorLog
	fatalLog
	traceLog
	maxLevelLog
)

var (
	levels = map[int]string{
		debugLog: Color(Green, "[DEBUG]"),
		infoLog:  Color(Green, "[INFO ]"),
		warnLog:  Color(Yellow, "[WARN ]"),
		errorLog: Color(Red, "[ERROR]"),
		fatalLog: Color(Red, "[FATAL]"),
		traceLog: Color(Pink, "[TRACE]"),
	}
	Stdout = os.Stdout
)

const (
	OutputPath           = "./Logs/" // The log files output path
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

var Log *Logger

func LevelName(level int) string {
	if name, ok := levels[level]; ok {
		return name
	}
	return namePrefix + strconv.Itoa(level)
}

type Logger struct {
	level       int   // The log print level
	maxLogsSize int64 // The max logs total size

	// Current log file and printer
	printLock     *sync.Mutex
	maxPerLogSize int64
	file          *os.File
	logger        *log.Logger
	watcher       *fsnotify.Watcher
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
	fileList, err := ioutil.ReadDir(OutputPath)
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
		os.Remove(OutputPath + file.Name())
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
	l.file, err = newLogFile(OutputPath)
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
	l.watcher.Add(OutputPath + info.Name())
}

func (l *Logger) handleFileEvents(event fsnotify.Event) {
	switch event.Op {
	case fsnotify.Write:
		info, _ := l.file.Stat()
		if info.Size() >= l.maxPerLogSize {
			l.printLock.Lock()
			// close previous log file
			l.file.Close()
			// unwatch it
			l.watcher.Remove(OutputPath + info.Name())
			// create a new log file
			l.newLogFile()
			l.printLock.Unlock()
		}
	}
}

func NewLogger(level int, maxPerLogSizeMb, maxLogsSizeMb int64) *Logger {
	logger := new(Logger)
	logger.level = level
	logger.printLock = new(sync.Mutex)

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

func Init(level int, maxPerLogSizeMb, maxLogsSizeMb int64) {
	Log = NewLogger(level, maxPerLogSizeMb, maxLogsSizeMb)
}

func SortLogFiles(files []os.FileInfo) {
	sort.Sort(byTime(files))
}

type byTime []os.FileInfo

// Len is the number of elements in the collection.
func (f byTime) Len() int {
	return len(f)
}

// Less reports whether the element with
// index i should sort before the element with index j.
func (f byTime) Less(i, j int) bool {
	return f[i].Name() < f[j].Name()
}

// Swap swaps the elements with indexes i and j.
func (f byTime) Swap(i, j int) {
	f[i], f[j] = f[j], f[i]
}

func (l *Logger) SetPrintLevel(level int) error {
	if level > maxLevelLog || level < 0 {
		return errors.New("Invalid Debug Level")
	}

	l.level = level
	return nil
}

func (l *Logger) Output(level int, a ...interface{}) error {
	l.printLock.Lock()
	defer l.printLock.Unlock()
	if level >= l.level {
		gidStr := strconv.FormatUint(GetGID(), 10)
		a = append([]interface{}{LevelName(level), "GID", gidStr + ","}, a...)
		return l.logger.Output(callDepth, fmt.Sprintln(a...))
	}
	return nil
}

func (l *Logger) Outputf(level int, format string, v ...interface{}) error {
	l.printLock.Lock()
	defer l.printLock.Unlock()
	if level >= l.level {
		v = append([]interface{}{LevelName(level), "GID", GetGID()}, v...)
		return l.logger.Output(callDepth, fmt.Sprintf("%s %s %d, "+format+"\n", v...))
	}
	return nil
}

func (l *Logger) Trace(a ...interface{}) {
	l.Output(traceLog, a...)
}

func (l *Logger) Tracef(format string, a ...interface{}) {
	l.Outputf(traceLog, format, a...)
}

func (l *Logger) Debug(a ...interface{}) {
	l.Output(debugLog, a...)
}

func (l *Logger) Debugf(format string, a ...interface{}) {
	l.Outputf(debugLog, format, a...)
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

func Trace(a ...interface{}) {
	if traceLog < Log.level {
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

	Log.Trace(a...)
}

func Tracef(format string, a ...interface{}) {
	if traceLog < Log.level {
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

	Log.Tracef("%s() %s:%d "+format, a...)
}

func Debug(a ...interface{}) {
	if debugLog < Log.level {
		return
	}

	pc := make([]uintptr, 10)
	runtime.Callers(2, pc)
	f := runtime.FuncForPC(pc[0])
	file, line := f.FileLine(pc[0])
	fileName := filepath.Base(file)

	a = append([]interface{}{f.Name(), fileName + ":" + strconv.Itoa(line)}, a...)

	Log.Debug(a...)
}

func Debugf(format string, a ...interface{}) {
	if debugLog < Log.level {
		return
	}

	pc := make([]uintptr, 10)
	runtime.Callers(2, pc)
	f := runtime.FuncForPC(pc[0])
	file, line := f.FileLine(pc[0])
	fileName := filepath.Base(file)

	a = append([]interface{}{f.Name(), fileName, line}, a...)

	Log.Debugf("%s %s:%d "+format, a...)
}

func Info(a ...interface{}) {
	Log.Info(a...)
}

func Warn(a ...interface{}) {
	Log.Warn(a...)
}

func Error(a ...interface{}) {
	Log.Error(a...)
}

func Fatal(a ...interface{}) {
	Log.Fatal(a...)
}

func Infof(format string, a ...interface{}) {
	Log.Infof(format, a...)
}

func Warnf(format string, a ...interface{}) {
	Log.Warnf(format, a...)
}

func Errorf(format string, a ...interface{}) {
	Log.Errorf(format, a...)
}

func Fatalf(format string, a ...interface{}) {
	Log.Fatalf(format, a...)
}
