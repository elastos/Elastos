import Foundation

public class VerifiableCredentialSubject {
    private var _id: DID
    private var _properties: ObjectNode
    
    init(_ id: DID) {
        self._id = id
        self._properties = ObjectNode()
    }

    public var did: DID {
        return self._id
    }

    public var propertyCount: Int {
        return self._properties.size
    }

    public func getPropertiesInString() -> String {
        return self._properties.toString()
    }

    public func getProperties() -> JsonNode {
        return self._properties.deepCopy()
    }

    public func getPropertyInString(ofName: String) -> String {
        // TODO:
        return "TODO"
    }

    public func getProperity(ofName: String) -> JsonNode {
        // TODO:
    }

    func setProperties(_ props: JsonNode) {
        // TODO:
    }

    class func fromJson(_ node: JsonNode, _ ref: DID?) throws -> VerifiableCredentialSubject {
        let errorGenerator = { (desc: String) -> DIDError in
            return DIDError.malformedDocument(desc)
        }

        let did = try JsonHelper.getDid(node, Constants.ID, ref != nil, ref, "credentialSubject id",
                                        errorGenerator)

        let subject = VerifiableCredentialSubject(did!)
        subject.setProperties(node)
        return subject
    }

    func toJson(_ generator: JsonGenerator, _ ref: DID?, _ normalized: Bool) throws {
        try generator.writeStartObject()

        if normalized || ref == nil || self.did != ref {
            try generator.writeStringField(Constants.ID, self.did.toString())
        }

        if self.propertyCount > 0 {
            //TODO: JsonHelper.toJson(generator, self._properties, true)
        }

        try generator.writeEndObject()
    }
}
