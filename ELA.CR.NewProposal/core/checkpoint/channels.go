// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package checkpoint

import (
	"bytes"
	"errors"
	"fmt"
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

type heightFileMsg struct {
	fileMsg
	height uint32
}

type fileChannels struct {
	cfg *Config

	save    chan fileMsg
	clean   chan fileMsg
	reset   chan fileMsg
	replace chan heightFileMsg
	exit    chan struct{}
}

func (c *fileChannels) Save(checkpoint ICheckPoint, reply chan bool) {
	c.save <- fileMsg{checkpoint, reply}
}

func (c *fileChannels) Clean(checkpoint ICheckPoint, reply chan bool) {
	c.clean <- fileMsg{checkpoint, reply}
}

func (c *fileChannels) Replace(checkpoint ICheckPoint, reply chan bool,
	height uint32) {
	c.replace <- heightFileMsg{fileMsg{checkpoint, reply},
		height}
}

func (c *fileChannels) Reset(checkpoint ICheckPoint, reply chan bool) {
	c.reset <- fileMsg{checkpoint, reply}
}

func (c *fileChannels) Exit() {
	c.exit <- struct{}{}
}

func (c *fileChannels) messageLoop() {
	for {
		var msg fileMsg
		var heightMsg heightFileMsg
		select {
		case msg = <-c.save:
			if err := c.saveCheckpoint(&msg); err != nil {
				msg.checkpoint.LogError(err)
			}
		case msg = <-c.clean:
			if err := c.cleanCheckpoints(&msg, true, false); err != nil {
				msg.checkpoint.LogError(err)
			}
		case msg = <-c.reset:
			if err := c.cleanCheckpoints(&msg, true, true); err != nil {
				msg.checkpoint.LogError(err)
			}
		case heightMsg = <-c.replace:
			if err := c.replaceCheckpoints(&heightMsg); err != nil {
				heightMsg.checkpoint.LogError(err)
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
		if err = os.MkdirAll(dir, 0700); err != nil {
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
		return c.cleanCheckpoints(msg, false, false)
	}
	return nil
}

func (c *fileChannels) cleanCheckpoints(msg *fileMsg,
	needReplay, cleanAll bool) (err error) {
	if needReplay {
		defer c.replyMsg(msg)
	}

	dir := getCheckpointDirectory(c.cfg.DataPath, msg.checkpoint)
	reserveCurrentName := getFileName(msg.checkpoint, msg.checkpoint.GetHeight())
	reservePrevName := getFileName(msg.checkpoint,
		msg.checkpoint.GetHeight()-msg.checkpoint.SavePeriod())
	defaultName := getDefaultFileName(msg.checkpoint)

	var files []os.FileInfo
	if files, err = ioutil.ReadDir(dir); err != nil {
		return
	}

	for _, f := range files {
		if !cleanAll {
			if f.Name() == reserveCurrentName || f.Name() == reservePrevName ||
				f.Name() == defaultName {
				continue
			}
		}
		if e := os.Remove(filepath.Join(dir, f.Name())); e != nil {
			msg.checkpoint.LogError(e)
		}
	}
	return
}

func (c *fileChannels) replaceCheckpoints(msg *heightFileMsg) (err error) {
	defer c.replyMsg(&msg.fileMsg)

	defaultFullName := getDefaultPath(c.cfg.DataPath, msg.checkpoint)
	// source file is the previous saved checkpoint
	sourceFullName := getFilePathByHeight(c.cfg.DataPath, msg.checkpoint,
		msg.height)
	if !utils.FileExisted(sourceFullName) {
		return errors.New(fmt.Sprintf("source file %s does not exist",
			sourceFullName))
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
		reset:   make(chan fileMsg),
		replace: make(chan heightFileMsg),
		exit:    make(chan struct{}),
	}
	go channels.messageLoop()
	return channels
}
