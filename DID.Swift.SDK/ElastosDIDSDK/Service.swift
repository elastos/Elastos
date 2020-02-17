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

    class func fromJson(_ node: Dictionary<String, Any>, _ ref: DID?) throws -> Service {
        let serializer = JsonSerializer(node)

        let id = try serializer.getDIDURL(Constants.ID,
                        JsonSerializer.Options<DIDURL>()
                            .withHint("serviceId"))
        let type = try serializer.getString(Constants.TYPE,
                        JsonSerializer.Options<String>()
                            .withHint("Service Type"))
        let endpoint = try serializer.getString(Constants.SERVICE_ENDPOINT,
                        JsonSerializer.Options<String>()
                            .withHint("Service Endpoint"))

        return  Service(id!, type!, endpoint!)
    }

    func toJson(_ generator: JsonGenerator, _ ref: DID?, _ normalized: Bool) throws {
        try generator.writeStartObject()
        try generator.writeStringField(Constants.ID, IDGetter(getId(), ref).value(normalized))
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
