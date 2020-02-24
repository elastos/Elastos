import Foundation

public class VerifiableCredentialSubject {
    private var _id: DID
    private var _properties: JsonNode?
    
    init(_ id: DID) {
        self._id = id
    }

    public var did: DID {
        return self._id
    }

    public var propertyCount: Int {
        return self._properties?.count ?? 0
    }

    public func getPropertiesAsString() -> String {
        return self._properties?.toString() ?? ""
    }

    public func getProperties() -> JsonNode? {
        return self._properties?.deepCopy()
    }

    public func getPropertyAsString(ofName: String) -> String? {
        return self._properties?.get(forKey: ofName)?.toString()
    }

    public func getProperty(ofName: String) -> JsonNode? {
        return self._properties?.get(forKey: ofName)
    }

    func setProperties(_ properties: JsonNode) {
        self._properties = properties.deepCopy()
    }

    class func fromJson(_ node: JsonNode, _ ref: DID?) throws -> VerifiableCredentialSubject {
        let serializer = JsonSerializer(node)
        let options    = JsonSerializer.Options()
                                    .withOptional(ref != nil)
                                    .withRef(ref)
                                    .withHint("credentialSubject id")
                                    .withError() { (des) -> DIDError in
                                        return DIDError.malformedCredential(des)
                                    }
        let did = try serializer.getDID(Constants.ID, options)

        let credential = VerifiableCredentialSubject(did)
        credential.setProperties(node)
        return credential
    }

    func toJson(_ generator: JsonGenerator, _ ref: DID?, _ normalized: Bool) {
        generator.writeStartObject()

        if normalized || ref == nil || self.did != ref {
            generator.writeStringField(Constants.ID, self.did.toString())
        }

        if self.propertyCount > 0 {
            //TODO: JsonHelper.toJson(generator, self._properties, true)
        }

        generator.writeEndObject()
    }
}
