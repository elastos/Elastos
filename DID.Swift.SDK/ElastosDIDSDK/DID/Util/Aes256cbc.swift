

import Foundation
import CryptoSwift

class Aes256cbc {
    
    class func encrypt(_ passw: String, _ plain: [UInt8]) throws -> [UInt8] {
        let key: [UInt8] = [UInt8](repeating: 0, count: 32)
        let iv: [UInt8] = [UInt8](repeating: 0, count: 16)
        // TODO: 补全 key & iv  的生成规则
        let aes = try AES(key: key, blockMode: CBC(iv: iv))
        let encrypted = try aes.encrypt(plain)
        return encrypted
    }
    
    class func decrypt(_ passwd: String, _ secret: [UInt8]) -> [UInt8]{
        
        return [UInt8]()
    }
    
}




