// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package events

import (
	"testing"
	"time"

	"github.com/elastos/Elastos.ELA/utils/test"
	"github.com/stretchr/testify/assert"
)

func TestNotify(t *testing.T) {
	test.SkipShort(t)
	notifyChan := make(chan struct{})
	Subscribe(func(event *Event) {
		notifyChan <- struct{}{}
	})

	for i := 0; i < 100; i++ {
		go func() {
			Notify(ETBlockAccepted, nil)
		}()
	}

	for i := 0; i < 100; i++ {
		select {
		case <-notifyChan:
		case <-time.After(time.Millisecond):
			t.Error("notify timeout")
		}
	}
}

func TestRecursiveNotify(t *testing.T) {
	Subscribe(func(event *Event) {
		Notify(ETBlockConnected, nil)
	})

	go func() {
		defer func() {
			if err := recover(); err != nil {
				if !assert.Equal(t, err, "recursive notifies detected") {
					t.FailNow()
				}
			}
		}()
		Notify(ETBlockAccepted, nil)
	}()

	<-time.After(time.Millisecond)

}
