import Foundation

class JsonGenerator {
    private static let COLON: Character = ":"
    private static let COMMA: Character = ","
    private static let OBJECT_STARTED: Character = "{"
    private static let OBJECT_END: Character = "}"
    private static let ARRAY_STARTED: Character = "["
    private static let ARRAY_END: Character = "]"
    private static let STRING_QUOTE_STARTED: Character = "\""
    private static let STRING_QUOTE_END: Character = "\""

    private var position: Int
    private var deep: Int
    private var state: [UInt8]
    private var buffer: String

    enum State: UInt8 {
        case Unkown = 0
        case Root = 1
        case Object = 2
        case Array = 3
        case Field = 4

        static func valueOf(_ value: UInt8) -> State {
            let state: State

            switch value {
            case 0:
                state = .Unkown
            case 1:
                state = .Root
            case 2:
                state = .Object
            case 3:
                state = .Array
            case 4:
                state = .Field
            default:
                state = .Unkown
            }
            return state
        }
    }

    init() {
        self.position = 0
        self.deep = 0
        self.buffer = ""
        self.state = []
    }

    private func pushState(_ state: State) {
        // TODO: CHECK
        if self.deep > self.state.count - 1 {
            self.state.append(state.rawValue)
        } else {
            self.state[self.deep] = state.rawValue
        }
        deep += 1
    }

    private func popState() -> State {
        if deep <= 0 {
            return .Unkown
        }

        deep -= 1
        return State.valueOf(state[deep] & 0x7F)
    }

    private func getState() -> State {
        if deep <= 0 {
            return .Unkown
        }

        return State.valueOf(state[deep-1] & 0x7F)
    }

    private func setSticky() {
        if deep - 1 < 0 {
            return
        }
        
        if deep - 1 > self.state.count {
            state.append(0)
        }
        state[deep - 1] |= 0x80
    }

    private func isSticky() -> Bool {
        if deep <= 0 {
            return false
        }

        return state[deep-1] & 0x80 == 0x80
    }

    func writeStartObject() {
        if isSticky() {
            buffer.append(JsonGenerator.COMMA)
        }

        buffer.append(JsonGenerator.OBJECT_STARTED)
        setSticky()
        pushState(.Object)
    }

    func writeEndObject() {
        buffer.append(JsonGenerator.OBJECT_END)
        _ = popState()
        if getState() == .Field {
            _ = popState() /* Pop field state*/
        }
    }

    func writeStartArray() {
        buffer.append(JsonGenerator.ARRAY_STARTED)
        pushState(.Array)
    }

    func writeEndArray() {
        buffer.append(JsonGenerator.ARRAY_END)
        _ = popState()
        if getState() == .Field {
            _ = popState() /* pop field state */
        }
    }

    func writeFieldName(_ name: String) {
        if isSticky() {
            buffer.append(JsonGenerator.COMMA)
        }

        buffer.append(JsonGenerator.STRING_QUOTE_STARTED)
        buffer.append(name)
        buffer.append(JsonGenerator.STRING_QUOTE_END)
        buffer.append(JsonGenerator.COLON)

        setSticky()
        pushState(.Field)
    }

    func writeRawValue(_ value: String) {
        if isSticky() {
            buffer.append(JsonGenerator.COMMA)
        }

        buffer.append(value)

        if getState() == .Field {
            _ = popState()
        } else {
            setSticky()
        }
    }
    func writeString(_ value: String) {
        if isSticky() {
            buffer.append(JsonGenerator.COMMA)
        }

        if !value.isEmpty {
            buffer.append(JsonGenerator.STRING_QUOTE_STARTED)
            buffer.append(value)
            buffer.append(JsonGenerator.STRING_QUOTE_END)
        } else {
            buffer.append(JsonGenerator.STRING_QUOTE_STARTED)
            buffer.append(JsonGenerator.STRING_QUOTE_END)
        }

        if getState() == .Field {
            _ = popState()
        } else {
            setSticky()
        }
    }

    func writeNumber(_ value: Any) {
        if isSticky() {
            buffer.append(JsonGenerator.COMMA)
        }

        buffer.append("\(value)")

        if getState() == .Field {
            _ = popState()
        } else {
            setSticky()
        }
    }

    func writeBool(_ value: Bool) {
        if isSticky() {
            buffer.append(JsonGenerator.COMMA)
        }

        if value {
            buffer.append("true")
        } else {
            buffer.append("false")
        }

        if getState() == .Field {
            _ = popState()
        } else {
            setSticky()
        }
    }

    func writeStringField(_ name: String, _ value: String) {
        writeFieldName(name)
        writeString(value)
    }

    func writeNumberField(_ name: String, _ value: Int) {
        writeFieldName(name)
        writeNumber(value)
    }

    func toString() -> String {
        let output = buffer
        buffer = ""
        return output
    }
}
