// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package types

import (
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types/payload"
)

// DPOSHeader represents a POW header + DPOS confirm
type DPOSHeader struct {
	Header
	HaveConfirm bool
	payload.Confirm
}

func (h *DPOSHeader) Serialize(w io.Writer) error {
	if err := h.Header.Serialize(w); err != nil {
		return err
	}

	if err := common.WriteElement(w, h.HaveConfirm); err != nil {
		return err
	}

	if h.HaveConfirm {
		return h.Confirm.Serialize(w)
	}
	return nil
}

func (h *DPOSHeader) Deserialize(r io.Reader) error {
	if err := h.Header.Deserialize(r); err != nil {
		return err
	}

	if err := common.ReadElement(r, &h.HaveConfirm); err != nil {
		return err
	}

	if h.HaveConfirm {
		return h.Confirm.Deserialize(r)
	}
	return nil
}
