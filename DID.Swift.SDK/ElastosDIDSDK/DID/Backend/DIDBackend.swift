import Foundation

class DIDBackend: NSObject {

    public class func create(_ doc: DIDDocument, _ signKey: DIDURL, _ passphrase: String) throws -> Bool {
        // TODO:
        return false
    }

    public class func update(_ doc: DIDDocument, _ signKey: DIDURL, _ passphrase: String) throws -> Bool {
        // TODO:
        return false
    }

    public class func deactivate(_ did: DID, _ signKey: DIDURL, _ passphrase: String) throws -> Bool {
        // TODO:
        return false
    }

    public class func resolve(_ did: DID) throws -> DIDDocument? {
        
        let request: IDChainRequest = try IDChainRequest(IDChainRequest.Operation.CREATE, did)
//        let json: String = request.sign
        
        // TODO:
        return nil
    }
}
