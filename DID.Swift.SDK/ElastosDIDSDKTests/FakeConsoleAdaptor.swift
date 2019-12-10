
import Foundation
import ElastosDIDSDK

class FakeConsoleAdaptor: DIDAdaptor {
    
    func createIdTransaction(_ payload: String, _ memo: String?) throws -> Bool {
        let json = JsonHelper.handleString(payload) as! OrderedDictionary<String, Any>
        let header = json["header"] as! OrderedDictionary<String, Any>
        print("Operation: \(header["operation"] ?? "")")
        let _payload = json["payload"] as! String
        let data = _payload.data(using: .utf8)
        _ = data?.base64urlEncodedString()
        return true
    }
    
    func resolve(_ did: String) throws -> String? {
        print("Operation: resolve")
        print("        \(did)")
        return nil
    }
}
