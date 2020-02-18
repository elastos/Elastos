import Foundation

public class VerifiableCredentialSubject {
    private var _id: DID
    private var _properties: Dictionary<String, Any>?
    
    init(_ id: DID) {
        self._id = id
    }

    public var did: DID {
        return self._id
    }

    public var propertyCount: Int {
        return self._properties?.count ?? 0
    }

    public func getPropertiesInString() -> String {
        // TODO:
        return "TODO"
    }

    public func getProperties() -> Dictionary<String, Any>? {
        // TODO:
        return nil
    }

    public func getPropertyInString(ofName: String) -> String {
        // TODO:
        return "TODO"
    }

    public func getProperity(ofName: String) ->  Dictionary<String, Any>? {
        // TODO:
        return nil
    }

    func setProperties(_ props: JsonNode) {
        // TODO:
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
