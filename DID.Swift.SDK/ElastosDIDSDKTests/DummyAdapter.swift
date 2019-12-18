
import XCTest
import ElastosDIDSDK
import SPVAdapter

class DummyAdapter: DIDAdapter {
 
    func createIdTransaction(_ payload: String, _ memo: String?) throws -> Bool {
        return false
    }
    
    func resolve(_ did: String) throws -> String? {
        return nil
    }
}
