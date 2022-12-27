// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package gpath

import (
	"reflect"
	"testing"

	"github.com/stretchr/testify/assert"
)

type A struct {
	N int
	S []string
}

type B struct {
	A A
}

func TestAt(t *testing.T) {
	b := B{
		A: A{
			N: 10,
			S: []string{"A", "B", "C"},
		},
	}

	n, err := At(b, "A.N")
	assert.NoError(t, err)
	assert.Equal(t, n, 10)

	v, err := At(b, "A.S")
	assert.NoError(t, err)
	assert.Equal(t, reflect.TypeOf([]string{}), reflect.TypeOf(v))
	s := v.([]string)
	assert.Equal(t, b.A.S[0], s[0])
	assert.Equal(t, b.A.S[1], s[1])
	assert.Equal(t, b.A.S[2], s[2])
}

func TestSet(t *testing.T) {
	b := B{
		A: A{
			N: 0,
			S: []string{"A", "B", "C"},
		},
	}

	assert.NoError(t, Set(&b, 10, "A.N"))
	assert.Equal(t, b.A.N, 10)

	assert.Error(t, Set(&b, uint32(10), "A.N"))

	assert.NoError(t, Set(&b, []string{"D", "E", "F"}, "A.S"))
	assert.Equal(t, "D", b.A.S[0])
	assert.Equal(t, "E", b.A.S[1])
	assert.Equal(t, "F", b.A.S[2])
}
