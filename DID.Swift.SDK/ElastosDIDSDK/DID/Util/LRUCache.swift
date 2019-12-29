
import Foundation


public class LRUCache {
    
    public init(_ s: Int) {
        size = s
        cacheQueue.reserveCapacity(size)
    }
    
    private let dispatchQueue = DispatchQueue(label: "com.erik.LRUCache.queue", attributes: .concurrent)
    public var cacheItems: [AnyHashable: Any] = [:]
    public var cacheQueue: [AnyHashable] = []
    public let size: Int
    
    func getCount() -> Int {
        return self.cacheItems.count
    }

    public var cache: [AnyHashable: Any] {
        var selfItems: [AnyHashable: Any] = [:]
        dispatchQueue.sync {
            selfItems = self.cacheItems
        }
        return selfItems
    }
    
    public var queue: [AnyHashable] {
        var selfQueue: [AnyHashable] = []
        dispatchQueue.sync {
            selfQueue = self.cacheQueue
        }
        return selfQueue
    }
    
    private func update(_ key: AnyHashable, _ data: Any?) {
        dispatchQueue.async(flags: .barrier) { [weak self] in
            guard let self = self else {
                return
            }
            let index = self.cacheQueue.firstIndex(of: key)
            if index == nil {
                // Add new data to cache
                if let data = data {
                    // If the queue is full, remove oldest key/value
                    if self.cacheQueue.count == self.size {
                        let last = self.cacheQueue.remove(at: 0)
                        self.cacheItems.removeValue(forKey: last)
                    }
                    self.cacheQueue.append(key)
                    self.cacheItems.updateValue(data, forKey: key)
                }
            }
            else {
                // Move the accessed key to the front of the cache queue
                self.cacheQueue.remove(at: index!)
                self.cacheQueue.append(key)
            }
        }
    }
    
    public func put(_ key: AnyHashable, data: Any) {
        update(key, data)
    }
    
    public func get(_ key: AnyHashable, data: Any? = nil) -> Any? {
        update(key, data)
        // Verify if the cache was added successfully
        if cache.contains(where: { $0.key == key }) {
            return self.cache[key]
        }
        return nil
    }
    
    public func clear() {
        dispatchQueue.async(flags: .barrier) {
            self.cacheQueue.removeAll()
            self.cacheItems.removeAll()
        }
    }
}
