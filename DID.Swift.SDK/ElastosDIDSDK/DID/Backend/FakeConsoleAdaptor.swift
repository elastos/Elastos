

import Foundation

class FakeConsoleAdaptor: DIDAdaptor {
    
    func createIdTransaction(_ payload: String, _ memo: String?) throws -> Bool {
        // MAKE: TODO
        return false
    }
    
    func resolve(_ did: String) throws -> String? {
        // MAKE: TODO
        return nil
    }
}
