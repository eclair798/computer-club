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

public:
    Club(Path inputPath);

    void RunEvents(File& file);

    void HandleEvent(const Event& ev);
    Event HandleClientArrived(const Event& ev);
    Event HandleSitAtTable(const Event& ev);
    Event HandleClientWaiting(const Event& ev);
    Event HandleClientLeaving(const Event& ev);

    void HandleLastUsers();
    void PrintIncome();

    TableIndex FindFreeTable() const;

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
