import CommonCrypto

class SHA256Helper {
    
    private var context = CC_SHA256_CTX()

    init() {
        CC_SHA256_Init(&context)
    }
    
    func update(_ data: inout [UInt8]) {

        CC_SHA256_Update(&context, &data, CC_LONG(data.count))
    }

    func finalize() -> [UInt8] {
        var hash = [UInt8](repeating: 0, count: Int(CC_SHA256_DIGEST_LENGTH))
        CC_SHA256_Final(&hash, &context)
        return hash
    }
}
