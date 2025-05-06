#pragma once

#include <cmath>
#include <iostream>
#include <optional>

namespace club {

using Minute = int;
using Hour = int;

using Username = std::string;
using ErrorMsg = std::string;
using EventID = int;

using TableIndex = int;
using Cost = int;
using Counter = int;

struct Time {
    Time() = default;
    Time(Hour h, Minute m);
    Time(Minute m);

    Time operator-(const Time& other) const;
    Time operator+(const Time& other) const;

    Hour Hours() const;
    bool In(const Time& left, const Time& right) const;

    friend std::ostream& operator<<(std::ostream& os, const Time& t);
    friend std::istream& operator>>(std::istream& in, Time& t);

    Minute minutes;
};

struct Event {
    Event() = default;
    Event(TableIndex tablesNum);

    static Event Error(Time eventTime, ErrorMsg msg);
    static Event ClientLeaving(Time eventTime, Username name);
    static Event SitAtTable(Time eventTime, Username name, TableIndex tableNum);

    void Reset();

    friend std::ostream& operator<<(std::ostream& os, const Event& ev);
    friend std::istream& operator>>(std::istream& in, Event& ev);

    Time time;
    EventID id = 0;
    Username name;
    ErrorMsg error;
    TableIndex tableNum = -1;
    TableIndex tablesNum = -1;
};

}  // namespace club
