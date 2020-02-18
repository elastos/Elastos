import Foundation

public class DIDDocumentProof {
    private var _type: String
    private var _createdDate: Date
    private var _creator: DIDURL
    private var _signature: String
    
    init(_ type: String, _ createdDate: Date, _ creator: DIDURL, _ signature: String) {
        self._type = type
        self._createdDate = createdDate
        self._creator = creator
        self._signature = signature
    }
    
    convenience init(_ creator: DIDURL, _ signature: String) {
        self.init(Constants.DEFAULT_PUBLICKEY_TYPE, DateHelper.currentDate(), creator, signature)
    }

    public var type: String {
        return self._type
    }

    public var createdDate: Date {
        return self._createdDate
    }

    public var creator: DIDURL {
        return self._creator
    }

    public var signature: String {
        return self._signature
    }

    class func fromJson(_ node: JsonNode, _ refSginKey: DIDURL) throws -> DIDDocumentProof {
        let serializer = JsonSerializer(node)
        var options: JsonSerializer.Options

        options = JsonSerializer.Options()
                                .withOptional()
                                .withRef(Constants.DEFAULT_PUBLICKEY_TYPE)
                                .withHint("document proof type")
        let type = try serializer.getString(Constants.TYPE, options)

        options = JsonSerializer.Options()
                                .withOptional()
                                .withHint("document proof type")
        let created = try serializer.getDate(Constants.CREATED, options)

        options = JsonSerializer.Options()
                                .withOptional()
                                .withRef(refSginKey.did)
                                .withHint("document proof creator")
        var creator = try serializer.getDIDURL(Constants.CREATOR, options)
        if  creator == nil {
            creator = refSginKey
        }

        options = JsonSerializer.Options()
                                .withHint("document proof signature")
        let signature = try serializer.getString(Constants.SIGNATURE_VALUE, options)

        return DIDDocumentProof(type, created, creator!, signature)
    }

    func toJson(_ generator: JsonGenerator, _ normalized: Bool) {
        generator.writeStartObject()

        // type
        if normalized || self.type != Constants.DEFAULT_PUBLICKEY_TYPE {
            generator.writeFieldName(Constants.TYPE)
            generator.writeString(self._type)
        }

        // createdDate
        generator.writeFieldName(Constants.CREATED)
        generator.writeString(DateHelper.formateDate(self.createdDate))

        // creator
        if normalized {
            generator.writeFieldName(Constants.CREATOR)
            generator.writeString(self.creator.toString())
        }

        // signature
        generator.writeFieldName(Constants.SIGNATURE_VALUE)
        generator.writeString(self.signature)

        generator.writeEndObject()
    }
}
