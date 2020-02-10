import Foundation

public class Service: DIDObject {
    private var _endpoint: String

    init(_ id: DIDURL, _ type: String, _ endpoint: String) {
        self._endpoint = endpoint
        super.init(id, type)
    }

    public var endpoint: String {
        return self._endpoint
    }

    class func fromJson(_ node: JsonNode, _ ref: DID?) throws -> Service {
        let id: DIDURL?
        let type: String?
        let endPoint: String?
        let errorGenerator = { (desc: String) -> DIDError in
            return DIDError.malformedDocument(desc)
        }

        id   = try JsonHelper.getDidUrl(node, Constants.ID, ref, "service id",
                                        errorGenerator)
        type = try JsonHelper.getString(node, Constants.TYPE, false, nil, "service type",
                                        errorGenerator)
        endPoint = try JsonHelper.getString(node, Constants.SERVICE_ENDPOINT,
                                        false, nil, "service endpoint",
                                        errorGenerator)

        return Service(id!, type!, endPoint!)
    }

    func toJson(_ generator: JsonGenerator, _ ref: DID?, _ normalized: Bool) throws {
        try generator.writeStartObject()
        try generator.writeFieldName(Constants.ID)

        let value: String
        if normalized || ref == nil || ref != getId().did {
            value = getId().toString()
        } else {
            value = "#" + getId().fragment
        }
        try generator.writeString(value)

        // type & endpoint
        try generator.writeStringField(Constants.TYPE, getType())
        try generator.writeStringField(Constants.SERVICE_ENDPOINT, self.endpoint)
        try generator.writeEndObject()
    }

    override func equalsTo(_ other: DIDObject) -> Bool {
        guard other is Service else {
            return false
        }

        let service = other as! Service
        return super.equalsTo(other) && self.endpoint == service.endpoint
    }
}

extension Service {
    public static func == (lhs: Service, rhs: Service) -> Bool {
        return lhs.equalsTo(rhs)
    }

    public static func != (lhs: Service, rhs: Service) -> Bool {
        return !lhs.equalsTo(rhs)
    }
}
