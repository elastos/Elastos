import Foundation

public class EmbeddedCredential: VerifiableCredential {
    
    
    override init(_ vc: VerifiableCredential) {
        super.init(vc)
    }

    override public class func fromJson(_ json: OrderedDictionary<String, Any>, _ ref: DID) throws -> VerifiableCredential {
        return try EmbeddedCredential(VerifiableCredential.fromJson(json, ref))
    }

    public override func toJson(_ ref: DID?, _ compact: Bool, _ forSign: Bool) -> OrderedDictionary<String, Any> {
        super.toJson(ref, compact, forSign)
    }
}
