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
        // TODO:
        return nil
    }
}
