/*
* Copyright (c) 2020 Elastos Foundation
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

import Foundation

final class LRUCache<Key: Hashable, Value> {
    private struct CachePayload {
        let key: Key
        let value: Value
    }

    private let list = DoubleLinkedList<CachePayload>()
    private var nodesDict = [Key: DoubleLinkedListNode<CachePayload>]()

    private let initCapacity: Int
    private let maxCapacity: Int

    init(_ capacity: Int) {
        self.initCapacity = 0
        self.maxCapacity = max(0, capacity)
    }

    init(_ initCapacity: Int, _ maxCapacity: Int) {
        self.initCapacity = initCapacity
        self.maxCapacity = max(initCapacity, maxCapacity)
    }

    func setValue(_ value: Value, for key: Key) {
        let payload = CachePayload(key: key, value: value)

        if let node = self.nodesDict[key] {
            node.payload = payload
            self.list.moveToHead(node)
        } else {
            let node = self.list.addHead(payload)
            self.nodesDict[key] = node
        }

        if self.list.count > self.maxCapacity {
            let nodeRemoved = self.list.removeLast()
            if let key = nodeRemoved?.payload.key {
                self.nodesDict[key] = nil
            }
        }
    }

    func getValue(for key: Key) -> Value? {
        guard let node = nodesDict[key] else {
            return nil
        }

        list.moveToHead(node)
        return node.payload.value
    }
    
    func containsKey(for key: Key) -> Bool {
        let value = nodesDict[key]
        return value != nil
    }
    
    func clear() {
        self.list.clear()
        self.nodesDict.removeAll()
    }

    func removeValue(for key: Key) {
        guard let node = nodesDict[key] else {
            return
        }
        list.removeNode(node)
        nodesDict.removeValue(forKey: key)
    }
}

typealias DoubleLinkedListNode<T> = DoubleLinkedList<T>.Node<T>

final class DoubleLinkedList<T> {
    final class Node<T> {
        var payload: T
        var previous: Node<T>?
        var next: Node<T>?

        init(payload: T) {
            self.payload = payload
        }
    }

    private(set) var count: Int = 0
    private var head: Node<T>?
    private var tail: Node<T>?

    func addHead(_ payload: T) -> Node<T> {
        let node = Node(payload: payload)
        defer {
            head = node
            count += 1
        }

        guard let head = head else {
            tail = node
            return node
        }

        head.previous = node

        node.previous = nil
        node.next = head
        return node
    }

    func removeNode(_ node: Node<T>) {
        var curNode = head
        while curNode != nil {
            if curNode === node {
                let preNode = curNode?.previous
                let nextNode = curNode?.next

                if preNode == nil {
                    head = nextNode
                } else {
                    preNode?.next = nextNode
                }
                break
            } else {
                curNode = curNode?.next
            }
        }
    }
    func moveToHead(_ node: Node<T>) {
        guard node !== head else {
            return
        }

        let previous = node.previous
        let next = node.next

        previous?.next = next
        next?.previous = previous

        node.next = head
        node.previous = nil

        if node === tail {
            tail = previous
        }
        self.head = node
    }

    func removeLast() -> Node<T>? {
        guard let tail = self.tail else {
            return nil
        }

        let previous = tail.previous
        previous?.next = nil
        self.tail = previous

        if count == 1 {
            head = nil
        }

        count -= 1
        return tail
    }
    
    func clear() {
        
        head = nil
        count = 0
    }
}
