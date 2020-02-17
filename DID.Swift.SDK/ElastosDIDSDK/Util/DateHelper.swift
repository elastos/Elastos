import Foundation

class DateHelper {
    class func currentDate()-> Date {
        let current  = Date()
        var calendar = Calendar(identifier: .gregorian)
        calendar.timeZone = TimeZone(abbreviation: "UTC")!

        var comps:DateComponents?
        comps = calendar.dateComponents([.year, .month, .day, .hour, .minute, .second], from: current)
        comps?.year = 0
        comps?.month = 0
        comps?.day = 0
        comps?.hour = 0
        comps?.minute = 0
        comps?.second = 0
        comps?.nanosecond = 0

        return calendar.date(byAdding: comps!, to: current) ?? Date()
    }
}
