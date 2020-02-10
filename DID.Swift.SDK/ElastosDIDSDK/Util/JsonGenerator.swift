import Foundation

protocol JsonGenerator {
    func writeStartObject() throws
    func writeEndObject() throws

    func writeStartArray() throws
    func writeEndArray() throws

    func writeFieldName(_ name: String) throws
    func writeString(_ value: String) throws

    func writeStringField(_ name: String, _ value: String) throws

    func writeNumberField(_ name: String, _ value: Int) throws

    func writeNumber(_ value: Int) throws
}
