
import Foundation

extension Int {
    
    static prefix  func ++(num:inout Int) -> Int  {
        num += 1
        return num
    }
    
    static postfix  func ++(num:inout Int) -> Int  {
        let temp = num
        num += 1
        return temp
    }
    
    static prefix  func --(num:inout Int) -> Int  {
        num -= 1
        return num
    }
    
    static postfix  func --(num:inout Int) -> Int  {
        let temp = num
        num -= 1
        return temp
    }
}

extension Dictionary {
    mutating func merge(dict: [Key: Value]){
        for (k, v) in dict {
            updateValue(v, forKey: k)
        }
    }
}
