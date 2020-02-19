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

    class func formateDate(_ date: Date) -> String {
        // TODO:
        return "TODO"
    }

    class func isExipired(_ date: Date) -> Bool {
        return isExpired(date, DateHelper.currentDate())
    }

    class func isExpired(_ date: Date, _ expirateDate: Date) -> Bool {
        // TODO
        return false
    }

    class func maxExpirationDate() -> Date {
        // TODO:
        return Date()
    }
}
