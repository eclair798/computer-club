#pragma once

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <optional>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "event.h"

namespace club {

struct Table {
    Table(Cost hourCost) : hourCost(hourCost) {
    }

    void StartSession(Time start) {
        sessionStart = start;
        isBusy = true;
    }

    void StopSession(Time sessionEnd) {
        if (!isBusy) {
            return;
        }
        Time sessionTime = sessionEnd - sessionStart;
        timeInUse = timeInUse + sessionTime;
        Hour workTime = sessionTime.Hours();
        income += hourCost * workTime;
        isBusy = false;
        sessionStart = Time();
    }

    Cost hourCost;
    Cost income = 0;
    Time timeInUse;
    bool isBusy = false;
    Time sessionStart;
};

enum class Status { InQueue, InClub, AtTable };

struct UserState {
    Status status;
    std::optional<TableIndex> table;
    std::optional<std::list<Username>::iterator> placeInQueue;
};

class Club {
    using Path = std::filesystem::path;
    using File = std::ifstream;
    using OStream = std::ostream;
    using Line = std::string;
    using ILineStream = std::istringstream;
    using OLineStream = std::ostringstream;

public:
    Club() = default;

    void Run(File& file, OStream& output = std::cout);

    void HandleEvents(File& file, OLineStream& output);
    void HandleEvent(const Event& ev, OLineStream& output);

    Event NewClientArrived(const Event& ev);
    Event NewSitAtTable(const Event& ev);
    Event NewClientWaiting(const Event& ev);
    Event NewClientLeaving(const Event& ev);

    void HandleLastUsers(OLineStream& oss);
    void PrintIncome(OLineStream& oss);

    TableIndex FindFreeTable() const;

    void LeaveQueue(Username name);

private:
    TableIndex tablesNum_;
    Time workDayStart_;
    Time workDayEnd_;
    Cost hourCost_;

    std::unordered_map<Username, UserState> users_;
    std::list<Username> queue_;
    std::vector<Table> tables_;
    Counter busyTables_ = 0;
};

}  // namespace club
