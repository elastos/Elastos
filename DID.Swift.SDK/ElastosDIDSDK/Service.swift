/*
* Copyright (c) 2020 Elastos Foundation
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

import Foundation

public class Service: DIDObject {
    private var _endpoint: String

    init(_ id: DIDURL, _ type: String, _ endpoint: String) {
        self._endpoint = endpoint
        super.init(id, type)
    }

    public var endpoint: String {
        return _endpoint
    }

    class func fromJson(_ node: JsonNode, _ ref: DID?) throws -> Service {
        let serializer = JsonSerializer(node)
        var options: JsonSerializer.Options

        options = JsonSerializer.Options()
                                .withRef(ref)
                                .withHint("service id")
        let id = try serializer.getDIDURL(Constants.ID, options)

        options = JsonSerializer.Options()
                                .withHint("service type")
        let type = try serializer.getString(Constants.TYPE, options)

        options = JsonSerializer.Options()
                                .withHint("service endpoint")
        let endpoint = try serializer.getString(Constants.SERVICE_ENDPOINT, options)

        return Service(id!, type, endpoint)
    }

    func toJson(_ generator: JsonGenerator, _ ref: DID?, _ normalized: Bool) {
        generator.writeStartObject()
        generator.writeStringField(Constants.ID, IDGetter(getId(), ref).value(normalized))
        generator.writeStringField(Constants.TYPE, getType())
        generator.writeStringField(Constants.SERVICE_ENDPOINT, endpoint)
        generator.writeEndObject()
    }

    override func equalsTo(_ other: DIDObject) -> Bool {
        guard other is Service else {
            return false
        }

        let service = other as! Service
        return super.equalsTo(other) && endpoint == service.endpoint
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
