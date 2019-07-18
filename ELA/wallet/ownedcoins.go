// Copyright (c) 2017-2019 Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package wallet

import "github.com/elastos/Elastos.ELA/core/types"

// CoinOwnership as a key for OwnedCoins map to represents the ownership
// between OutPoint and owner.
type CoinOwnership struct {
	owner string
	op    types.OutPoint
}

// CoinLinkedItem as a value for OwnedCoins map to be a linked list item.
type CoinLinkedItem struct {
	prev *types.OutPoint
	next *types.OutPoint
}

// OwnedCoins store the ownership between OutPoint and owner, and can traverse the
// OutPoint of an owner through a linked list.
type OwnedCoins map[CoinOwnership]CoinLinkedItem

func (oc OwnedCoins) read(owner string, op *types.OutPoint) CoinLinkedItem {
	if op == nil {
		op = &types.OutPoint{}
	}
	return oc[CoinOwnership{owner, *op}]
}

func (oc OwnedCoins) write(owner string, op *types.OutPoint, item CoinLinkedItem) {
	if op == nil {
		op = &types.OutPoint{}
	}
	oc[CoinOwnership{owner, *op}] = item
}

func (oc OwnedCoins) readHead(owner string) CoinLinkedItem {
	return oc.read(owner, nil)
}

func (oc OwnedCoins) writeHead(owner string, item CoinLinkedItem) {
	oc.write(owner, nil, item)
}

func (oc OwnedCoins) append(owner string, op *types.OutPoint) {
	if op == nil {
		return
	}
	_, exist := oc[CoinOwnership{owner, *op}]
	if exist {
		return
	}
	headItem := oc.readHead(owner)
	newHead := CoinLinkedItem{
		prev: op,
		next: headItem.next,
	}
	oc.writeHead(owner, newHead)

	prevItem := oc.read(owner, headItem.prev)
	newPrevItem := CoinLinkedItem{
		prev: prevItem.prev,
		next: op,
	}
	oc.write(owner, headItem.prev, newPrevItem)

	item := CoinLinkedItem{
		prev: headItem.prev,
		next: nil,
	}
	oc.write(owner, op, item)
}

func (oc OwnedCoins) remove(owner string, op *types.OutPoint) {
	if op == nil {
		return
	}
	item, exist := oc[CoinOwnership{owner, *op}]
	if !exist {
		return
	}
	prevItem := oc.read(owner, item.prev)
	newPrevItem := CoinLinkedItem{
		prev: prevItem.prev,
		next: item.next,
	}
	oc.write(owner, item.prev, newPrevItem)

	nextItem := oc.read(owner, item.next)
	newNextItem := CoinLinkedItem{
		prev: item.prev,
		next: nextItem.next,
	}
	oc.write(owner, item.next, newNextItem)

	delete(oc, CoinOwnership{owner, *op})
}

func (oc OwnedCoins) list(owner string) []*types.OutPoint {
	resultOp := make([]*types.OutPoint, 0)
	itemHead := oc.readHead(owner)
	op := itemHead.next
	for {
		if op == nil {
			break
		}
		resultOp = append(resultOp, op)
		item := oc.read(owner, op)
		op = item.next
	}

	return resultOp
}

func NewOwnedCoins() OwnedCoins {
	return make(map[CoinOwnership]CoinLinkedItem, 0)
}
