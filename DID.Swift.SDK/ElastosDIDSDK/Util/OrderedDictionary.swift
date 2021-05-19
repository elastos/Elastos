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

public struct OrderedDictionary<KeyType: Hashable, ValueType> {
    private var _dictionary: Dictionary<KeyType, ValueType>
    private var _keys: Array<KeyType>
    
    public init() {
        _dictionary = [:]
        _keys = []
    }
    
    public init(minimumCapacity: Int) {
        _dictionary = Dictionary<KeyType, ValueType>(minimumCapacity: minimumCapacity)
        _keys = Array<KeyType>()
    }
    
    public init(_ dictionary: Dictionary<KeyType, ValueType>) {
        _dictionary = dictionary
        _keys = dictionary.keys.map { $0 }
    }
    
    public subscript(key: KeyType) -> ValueType? {
        get {
            _dictionary[key]
        }
        set {
            if newValue == nil {
                _ = self.removeValueForKey(key: key)
            } else {
                _ = self.updateValue(value: newValue!, forKey: key)
            }
        }
    }
    
    public mutating func updateValue(value: ValueType, forKey key: KeyType) -> ValueType? {
        let oldValue = _dictionary.updateValue(value, forKey: key)
        if oldValue == nil {
            _keys.append(key)
        }
        return oldValue
    }
    
    public mutating func removeValueForKey(key: KeyType) -> Bool {
        _keys = _keys.filter {
            $0 != key
        }
        return (_dictionary.removeValue(forKey: key) != nil)
    }
    
    public mutating func removeAll(keepCapacity: Int) {
        _keys = []
        _dictionary = Dictionary<KeyType, ValueType>(minimumCapacity: keepCapacity)
    }
    
    public var count: Int {
        get {
            _dictionary.count
        }
    }
    
    // keys isn't lazy evaluated because it's just an array anyway
    public var keys: [KeyType] {
        get {
            _keys
        }
    }
    
    public var values: Array<ValueType> {
        get {
            _keys.map { _dictionary[$0]! }
        }
    }
    
    public static func ==<Key: Equatable, Value: Equatable>(lhs: OrderedDictionary<Key, Value>, rhs: OrderedDictionary<Key, Value>) -> Bool {
        lhs._keys == rhs._keys && lhs._dictionary == rhs._dictionary
    }
    
    public static func !=<Key: Equatable, Value: Equatable>(lhs: OrderedDictionary<Key, Value>, rhs: OrderedDictionary<Key, Value>) -> Bool {
        lhs._keys != rhs._keys || lhs._dictionary != rhs._dictionary
    }
}

extension OrderedDictionary: Sequence {
    
    public func makeIterator() -> OrderedDictionaryIterator<KeyType, ValueType> {
        OrderedDictionaryIterator<KeyType, ValueType>(sequence: _dictionary, keys: _keys, current: 0)
    }
}

public struct OrderedDictionaryIterator<KeyType: Hashable, ValueType>: IteratorProtocol {
    let sequence: Dictionary<KeyType, ValueType>
    let keys: Array<KeyType>
    var current = 0
    
    mutating public func next() -> (KeyType, ValueType)? {
        defer { current += 1 }
        guard sequence.count > current else {
            return nil
        }
        
        let key = keys[current]
        guard let value = sequence[key] else {
            return nil
        }
        return (key, value)
    }
    
}

