package checkpoint

import (
	"bytes"
	"errors"
	"io"
	"io/ioutil"
	"os"
	"path/filepath"

	"github.com/elastos/Elastos.ELA/utils"
)

type fileMsg struct {
	checkpoint ICheckPoint
	reply      chan bool
}

type fileChannels struct {
	cfg *Config

	save    chan fileMsg
	clean   chan fileMsg
	replace chan fileMsg
	exit    chan struct{}
}

func (c *fileChannels) Save(checkpoint ICheckPoint, reply chan bool) {
	c.save <- fileMsg{checkpoint, reply}
}

func (c *fileChannels) Clean(checkpoint ICheckPoint, reply chan bool) {
	c.clean <- fileMsg{checkpoint, reply}
}

func (c *fileChannels) Replace(checkpoint ICheckPoint, reply chan bool) {
	c.replace <- fileMsg{checkpoint, reply}
}

func (c *fileChannels) Exit() {
	c.exit <- struct{}{}
}

func (c *fileChannels) messageLoop() {
	for {
		var msg fileMsg
		select {
		case msg = <-c.save:
			if err := c.saveCheckpoint(&msg); err != nil {
				msg.checkpoint.LogError(err)
			}
		case msg = <-c.clean:
			if err := c.cleanCheckpoints(&msg, true); err != nil {
				msg.checkpoint.LogError(err)
			}
		case msg = <-c.replace:
			if err := c.replaceCheckpoints(&msg); err != nil {
				msg.checkpoint.LogError(err)
			}
		case <-c.exit:
			return
		}
	}
}

func (c *fileChannels) saveCheckpoint(msg *fileMsg) (err error) {
	defer c.replyMsg(msg)

	dir := getCheckpointDirectory(c.cfg.DataPath, msg.checkpoint)
	if !utils.FileExisted(dir) {
		if err = os.Mkdir(dir, 0700); err != nil {
			return
		}
	}

	filename := getFilePath(c.cfg.DataPath, msg.checkpoint)
	var file *os.File
	file, err = os.OpenFile(filename,
		os.O_CREATE|os.O_WRONLY|os.O_TRUNC, 0600)
	if err != nil {
		return
	}
	defer file.Close()

	buf := new(bytes.Buffer)
	if err = msg.checkpoint.Serialize(buf); err != nil {
		return
	}

	if _, err = file.Write(buf.Bytes()); err != nil {
		return
	}

	if !c.cfg.EnableHistory {
		return c.cleanCheckpoints(msg, false)
	}
	return nil
}

func (c *fileChannels) cleanCheckpoints(msg *fileMsg,
	needReplay bool) (err error) {
	if needReplay {
		defer c.replyMsg(msg)
	}

	dir := getCheckpointDirectory(c.cfg.DataPath, msg.checkpoint)
	reserveName := getFileName(msg.checkpoint, msg.checkpoint.GetHeight())
	defaultName := getDefaultFileName(msg.checkpoint)

	var files []os.FileInfo
	if files, err = ioutil.ReadDir(dir); err != nil {
		return
	}

	for _, f := range files {
		if f.Name() == reserveName || f.Name() == defaultName {
			continue
		}
		if e := os.Remove(filepath.Join(dir, f.Name())); e != nil {
			msg.checkpoint.LogError(e)
		}
	}
	return
}

func (c *fileChannels) replaceCheckpoints(msg *fileMsg) (err error) {
	defer c.replyMsg(msg)

	defaultFullName := getDefaultPath(c.cfg.DataPath, msg.checkpoint)
	sourceFullName := getFilePath(c.cfg.DataPath, msg.checkpoint)
	if !utils.FileExisted(sourceFullName) {
		return errors.New("source file does not exist")
	}

	if utils.FileExisted(defaultFullName) {
		if err = os.Remove(defaultFullName); err != nil {
			return
		}
	}

	if !c.cfg.EnableHistory {
		err = os.Rename(sourceFullName, defaultFullName)
	} else {
		var srcFile, desFile *os.File
		if srcFile, err = os.OpenFile(sourceFullName, os.O_RDONLY, 0400);
			err != nil || srcFile == nil {
			return
		}
		defer srcFile.Close()

		if desFile, err = os.OpenFile(defaultFullName,
			os.O_CREATE|os.O_WRONLY|os.O_TRUNC, 0600); err != nil {
			return
		}
		defer desFile.Close()

		_, err = io.Copy(desFile, srcFile)
	}
	return
}

func (c *fileChannels) replyMsg(msg *fileMsg) {
	if msg.reply != nil {
		msg.reply <- true
	}
}

func NewFileChannels(cfg *Config) *fileChannels {
	channels := &fileChannels{
		cfg:     cfg,
		save:    make(chan fileMsg),
		clean:   make(chan fileMsg),
		replace: make(chan fileMsg),
		exit:    make(chan struct{}),
	}
	go channels.messageLoop()
	return channels
}
