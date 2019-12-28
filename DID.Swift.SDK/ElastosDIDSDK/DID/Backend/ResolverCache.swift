
import Foundation

class ResolverCache {
    private let CACHE_INITIAL_CAPACITY = 16
    private let CACHE_MAX_CAPACITY = 32
    private var rootDir: String = ""

    
    private class func getRootDir() -> String {
        // todo:
        
        return ""
    }
    
    private class func getFile(_ id: String) -> String {
        return ""
    }
    
    public class func reset() {
    
    }
    
    public class func store(_ rr: ResolveResult) throws {
        
    }

    public class func load(_ did: DID, _ ttl: Int) throws -> ResolveResult? {
    
        return nil
    }
}

