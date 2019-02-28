package config

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestFoundation(t *testing.T) {
	address, _ := mainNetFoundation.ToAddress()
	assert.Equal(t, "8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta", address)

	address, _ = testNetFoundation.ToAddress()
	assert.Equal(t, "8ZNizBf4KhhPjeJRGpox6rPcHE5Np6tFx3", address)
}
