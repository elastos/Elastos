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

    func getProperties() -> JsonNode? {
        return self._properties
    }

    func getPropertyAsString(ofName: String) -> String {
        return self._properties?.getValue(ofName) ?? ""
    }

    func getProperity(ofName: String) -> JsonNode? {
        return self._properties?.getNode(ofName)?.deepCopy()
    }

    func setProperties(_ props: JsonNode) {
        self._properties = props.deepCopy()
    }

    class func fromJson(_ node: JsonNode, _ ref: DID?) throws -> VerifiableCredentialSubject {
        let serializer = JsonSerializer(node)
        let options = JsonSerializer.Options()
                                    .withOptional(ref != nil)
                                    .withRef(ref)
                                    .withHint("credentialSubject id")
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
