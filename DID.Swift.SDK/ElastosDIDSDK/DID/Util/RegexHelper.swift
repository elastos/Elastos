
struct RegexHelper {
    let regex: NSRegularExpression

    init(_ pattern: String) throws {
        try regex = NSRegularExpression(pattern: pattern,
                                        options: .caseInsensitive)
    }

    func match(input: String) -> Bool {
        let matches = regex.matches(in: input,
                    options: [],
                    range: NSMakeRange(0, input.utf16.count))
        return matches.count > 0
    }
}
