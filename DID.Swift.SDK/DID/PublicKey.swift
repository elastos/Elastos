

import Foundation

public class PublicKey: DIDObject {

    var controller: DID?
    var keyBase58: String?

    init(_ id: DIDURL, _ type: String, _ controller: DID, _ keyBase58: String) {
        super.init(id, type)
    }

   class public func fromJson(_ dic: Dictionary<String, Any>, _ ref: DID) throws -> PublicKey{
        let id: DIDURL = try JsonHelper.getDidUrl(dic, Constants.id, ref, "publicKey' id")
        let type = try JsonHelper.getString(dic, Constants.type, true, Constants.defaultPublicKeyType, "publicKey' type")
        let controller: DID = try JsonHelper.getDid(dic, Constants.controller, true, ref, "publicKey' controller")
        let keyBase58: String = try JsonHelper.getString(dic, Constants.publicKeyBase58, false, nil, "publicKeyBase58")
        return PublicKey(id, type, controller, keyBase58)
    }
}
