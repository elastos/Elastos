

import Foundation

public protocol DIDAdaptor {
    
     func createIdTransaction(_ payload: String, _ memo: String?) throws -> Bool
    
     func resolve(_ did: String) throws -> String?
    
}
