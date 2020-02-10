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
        self.init(Constants.DEFAULT_PUBLICKEY_TYPE, DateFormater.currentDate(), creator, signature)
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

    class func fromJson(_ node: JsonNode, _ refSignkey: DIDURL) throws -> DIDDocumentProof {
        let type: String?
        let created: Date?
        let creator: DIDURL?
        let signature: String?
        let errorGenerator = { (desc: String) -> DIDError in
            return DIDError.malformedDocument(desc)
        }

        type = try JsonHelper.getString(node, Constants.TYPE, true,
                                        Constants.DEFAULT_PUBLICKEY_TYPE,
                                        "document proof type",
                                        errorGenerator)
        created = try JsonHelper.getDate(node, Constants.CREATED, true, nil,
                                        "document proof created date",
                                        errorGenerator)
        creator = try JsonHelper.getDidUrl(node, Constants.CREATOR, true,
                                        refSignkey.did, "document proof creator",
                                        errorGenerator)
        signature = try JsonHelper.getString(node, Constants.SIGNATURE_VALUE,
                                        false, nil, "document signature",
                                        errorGenerator)

        return DIDDocumentProof(type!, created!, creator!, signature!)
    }

    func toJson(_ generator: JsonGenerator, _ normalized: Bool) throws {
        try generator.writeStartObject()

        // type
        if normalized || self._type != Constants.DEFAULT_PUBLICKEY_TYPE {
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
