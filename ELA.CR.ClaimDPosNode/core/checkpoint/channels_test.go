// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package checkpoint

import (
	"testing"

	"github.com/elastos/Elastos.ELA/utils"
	"github.com/stretchr/testify/assert"
)

func TestFileChannels_LifeCycle(t *testing.T) {
	channels := NewFileChannels(&Config{
		EnableHistory: true,
	})
	data := uint64(1)
	currentHeight := uint32(10)
	pt := &checkpoint{
		data:   &data,
		height: currentHeight,
	}

	// save
	reply := make(chan bool)
	channels.Save(pt, reply)
	<-reply
	assert.FileExists(t, getFilePath("", pt))

	// replace
	reply = make(chan bool)
	channels.Replace(pt, reply, pt.height)
	<-reply
	assert.FileExists(t, getFilePath("", pt))
	assert.FileExists(t, getDefaultPath("", pt))

	// clean without changing height
	reply = make(chan bool)
	channels.Clean(pt, reply)
	<-reply
	assert.FileExists(t, getFilePath("", pt))
	assert.FileExists(t, getDefaultPath("", pt))

	// clean after change height
	currentHeight += 1
	pt.SetHeight(currentHeight)
	reply = make(chan bool)
	channels.Clean(pt, reply)
	<-reply
	assert.False(t, utils.FileExisted(getFilePath("", pt)))
	assert.FileExists(t, getDefaultPath("", pt))

	// reset should clean all reserved files
	channels.Reset(pt, reply)
	<-reply
	assert.False(t, utils.FileExisted(getDefaultPath("", pt)))

	// exit
	channels.Exit()

	cleanCheckpoints()
}
