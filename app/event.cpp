#include "event.h"

namespace club {

Time::Time(Hour h, Minute m) {
    minutes = h * 60 + m;
}

Time::Time(Minute m) {
    minutes = m;
}

Time Time::operator-(const Time& other) const {
    Minute diff;
    if (minutes >= other.minutes) {
        diff = minutes - other.minutes;
    } else {
        diff = 24 * 60 + minutes - other.minutes;
    }

    return Time(diff);
}

Time Time::operator+(const Time& other) const {
    Minute sum;
    sum = minutes + other.minutes;
    return Time(sum);
}

Hour Time::Hours() const {
    return static_cast<Hour>(std::ceil(minutes / 60.0));
}

bool Time::In(const Time& left, const Time& right) const {
    if (left.minutes <= right.minutes) {
        return minutes >= left.minutes && minutes <= right.minutes;
    } else {
        return minutes >= left.minutes || minutes <= right.minutes;
    }
}

Event::Event(TableIndex count) : tablesNum(count) {
}

Event Event::Error(Time eventTime, ErrorMsg msg) {
    Event ev;
    ev.time = eventTime;
    ev.id = 13;
    ev.error = msg;
    return ev;
}

Event Event::ClientLeaving(Time eventTime, Username name) {
    Event ev;
    ev.time = eventTime;
    ev.id = 11;
    ev.name = name;
    return ev;
}

Event Event::SitAtTable(Time eventTime, Username name, TableIndex tableNum) {
    Event ev;
    ev.time = eventTime;
    ev.id = 12;
    ev.name = name;
    ev.tableNum = tableNum;
    return ev;
}

void Event::Reset() {
    time = Time();
    id = 0;
    name = "";
    error = "";
    tableNum = -1;
}

// === parsing ===

inline bool IsValidName(const std::string& name) {
    for (char c : name) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-')) {
            return false;
        }
    }
    return true;
}

std::ostream& operator<<(std::ostream& os, const Time& t) {
    Hour h = t.minutes / 60;
    Minute m = t.minutes - h * 60;
    os << (h < 10 ? "0" : "") << h << ":" << (m < 10 ? "0" : "") << m;
    return os;
}

std::istream& operator>>(std::istream& in, Time& t) {
    std::string timeString;
    if (!(in >> timeString)) {
        in.setstate(std::ios::failbit);
        return in;
    }
    if (timeString.size() != 5 || timeString[2] != ':' ||
        !std::isdigit(static_cast<unsigned char>(timeString[0])) ||
        !std::isdigit(static_cast<unsigned char>(timeString[1])) ||
        !std::isdigit(static_cast<unsigned char>(timeString[3])) ||
        !std::isdigit(static_cast<unsigned char>(timeString[4]))) {
        in.setstate(std::ios::failbit);
        return in;
    }

    int h = (timeString[0] - '0') * 10 + (timeString[1] - '0');
    int m = (timeString[3] - '0') * 10 + (timeString[4] - '0');

    if (h < 0 || h > 23 || m < 0 || m > 59) {
        in.setstate(std::ios::failbit);
        return in;
    }

    t = Time(h, m);
    return in;
}

std::ostream& operator<<(std::ostream& os, const Event& ev) {
    if (ev.id == 0) {
        return os;
    }

    os << ev.time << " ";
    os << ev.id << " ";

    if (!ev.name.empty())
        os << ev.name << " ";

    if (ev.tableNum != -1)
        os << ev.tableNum + 1 << " ";

    if (!ev.error.empty())
        os << ev.error;

    os << "\n";
    return os;
}

std::istream& operator>>(std::istream& in, Event& ev) {
    ev.Reset();
    if (!(in >> ev.time >> ev.id)) {
        in.setstate(std::ios::failbit);
        return in;
    }

    if (ev.id < 1 || ev.id > 4) {
        in.setstate(std::ios::failbit);
        return in;
    }
    if (ev.id == 1 || ev.id == 3 || ev.id == 4) {
        if (!(in >> ev.name)) {
            in.setstate(std::ios::failbit);
            return in;
        }
        if (!IsValidName(ev.name)) {
            in.setstate(std::ios::failbit);
            return in;
        }

    } else if (ev.id == 2) {
        if (!(in >> ev.name >> ev.tableNum)) {
            in.setstate(std::ios::failbit);
            return in;
        }
        --ev.tableNum;
        if (!IsValidName(ev.name)) {
            in.setstate(std::ios::failbit);
            return in;
        }
        if (ev.tablesNum != -1 && (ev.tableNum < 0 || ev.tableNum > ev.tablesNum - 1)) {
            in.setstate(std::ios::failbit);
            return in;
        }
    }

    return in;
}

}  // namespace club
