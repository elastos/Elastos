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

    class func fromJson(_ node: Dictionary<String, Any>, _ refSginKey: DIDURL) throws -> DIDDocumentProof {
        let serializer = JsonSerializer(node)

        let type = try serializer.getString(Constants.TYPE,
                            JsonSerializer.Options<String>()
                            .withOptional()
                            .withDefValue(Constants.DEFAULT_PUBLICKEY_TYPE)
                            .withHint("document proof type"))
        let created = try serializer.getDate(Constants.CREATED,
                            JsonSerializer.Options<Date>()
                            .withOptional()
                            .withHint("document proof created date"))
        let creator = try serializer.getDIDURL(Constants.CREATOR,
                            JsonSerializer.Options<DIDURL>()
                            .withOptional()
                            .withHint("document proof creator"))
        let signature = try serializer.getString(Constants.SIGNATURE_VALUE,
                            JsonSerializer.Options<String>()
                            .withHint("document signature"))

        return DIDDocumentProof(type!, created, creator!, signature!)
    }

    func toJson(_ generator: JsonGenerator, _ normalized: Bool) throws {
        try generator.writeStartObject()

        // type
        if normalized || self.type != Constants.DEFAULT_PUBLICKEY_TYPE {
            try generator.writeFieldName(Constants.TYPE)
            try generator.writeString(self._type)
        }

        // createdDate
        try generator.writeFieldName(Constants.CREATED)
        try generator.writeString(JsonHelper.fromDate(self.createdDate)!) // TODO:

        // creator
        if normalized {
            try generator.writeFieldName(Constants.CREATOR)
            try generator.writeString(self.creator.toString())
        }

        // signature
        try generator.writeFieldName(Constants.SIGNATURE_VALUE)
        try generator.writeString(self.signature)

        try generator.writeEndObject()
    }
}
