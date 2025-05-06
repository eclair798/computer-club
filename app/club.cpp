#include "club.h"

namespace club {

Club::Club(Path inputPath) {
    File file{inputPath};
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file");
    }
    if (!(file >> tablesNum_ >> workDayStart_ >> workDayEnd_ >> hourCost_)) {
        throw std::invalid_argument("Error. Incorrect settings");
    }
    tables_ = std::vector<Table>(tablesNum_, Table(hourCost_));
    RunEvents(file);
}

void Club::RunEvents(File& file) {
    using Line = std::string;
    using LineStream = std::istringstream;

    std::cout << workDayStart_ << "\n";

    Line line;
    std::getline(file, line);
    while (std::getline(file, line)) {
        LineStream lineStream(line);
        Event ev(tablesNum_);
        if (!(lineStream >> ev)) {
            throw std::invalid_argument(line);
        }
        HandleEvent(ev);
    }

    HandleLastUsers();
    std::cout << workDayEnd_ << "\n";
    PrintIncome();
}

void Club::HandleEvent(const Event& ev) {
    Event retEv;
    switch (ev.id) {
        case 1:
            retEv = HandleClientArrived(ev);
            break;
        case 2:
            retEv = HandleSitAtTable(ev);
            break;
        case 3:
            retEv = HandleClientWaiting(ev);
            break;
        case 4:
            retEv = HandleClientLeaving(ev);
            break;
        default:
            break;
    }
    std::cout << ev << retEv;
}

Event Club::HandleClientArrived(const Event& ev) {  // <время> 1 <имя клиента>
    Event retEv;
    if (!ev.time.In(workDayStart_, workDayEnd_)) {
        retEv = Event::Error(ev.time, "NotOpenYet");
        return retEv;
    }
    if (users_.find(ev.name) != users_.end()) {
        retEv = Event::Error(ev.time, "YouShallNotPass");
        return retEv;
    }
    users_[ev.name] = {Status::InClub, std::nullopt, std::nullopt};
    return retEv;
}

Event Club::HandleSitAtTable(const Event& ev) {  // <время> 2 <имя клиента> <номер стола>
    Event retEv;
    if (users_.find(ev.name) == users_.end()) {
        retEv = Event::Error(ev.time, "ClientUnknown");
        return retEv;
    }
    if (tables_[ev.tableNum].isBusy) {
        retEv = Event::Error(ev.time, "PlaceIsBusy");
        return retEv;
    }
    if (users_[ev.name].status == Status::AtTable) {
        --busyTables_;
        tables_[users_[ev.name].table.value()].StopSession(ev.time);
    }
    if (users_[ev.name].status == Status::InQueue) {
        queue_.erase(users_[ev.name].placeInQueue.value());
    }
    ++busyTables_;
    tables_[ev.tableNum].StartSession(ev.time);
    users_[ev.name] = {Status::AtTable, ev.tableNum, std::nullopt};
    return retEv;
}

Event Club::HandleClientWaiting(const Event& ev) {  // <время> 3 <имя клиента>
    Event retEv;
    if (busyTables_ < tablesNum_) {
        retEv = Event::Error(ev.time, "ICanWaitNoLonger!");
        return retEv;
    }
    if (queue_.size() > tablesNum_) {
        retEv = Event::ClientLeaving(ev.time, ev.name);
        users_.erase(ev.name);
        return retEv;
    }
    queue_.emplace_back(ev.name);
    users_[ev.name] = {Status::InQueue, std::nullopt, std::prev(queue_.end())};
    return retEv;
}

Event Club::HandleClientLeaving(const Event& ev) {  // <время> 4 <имя клиента>
    Event retEv;
    if (users_.find(ev.name) == users_.end()) {
        retEv = Event::Error(ev.time, "ClientUnknown");
        return retEv;
    }
    if (users_[ev.name].status == Status::AtTable) {
        --busyTables_;
        tables_[users_[ev.name].table.value()].StopSession(ev.time);
    }
    if (users_[ev.name].status == Status::InQueue) {
        queue_.erase(users_[ev.name].placeInQueue.value());
    }
    users_.erase(ev.name);
    if (busyTables_ < tablesNum_ && queue_.size() > 0) {
        TableIndex tableNum = FindFreeTable();
        Username user = queue_.front();

        ++busyTables_;
        tables_[tableNum].StartSession(ev.time);
        users_[user] = {Status::AtTable, tableNum, std::nullopt};

        queue_.pop_front();
        retEv = Event::SitAtTable(ev.time, user, tableNum);
    }
    return retEv;
}

void Club::HandleLastUsers() {
    for (size_t i = 0; i < tables_.size(); ++i) {
        tables_[i].StopSession(workDayEnd_);
    }

    std::vector<Username> usersInClub;
    for (const auto& pair : users_) {
        usersInClub.push_back(pair.first);
    }
    std::sort(usersInClub.begin(), usersInClub.end());

    for (size_t i = 0; i < usersInClub.size(); ++i) {
        Event retEv = Event::ClientLeaving(workDayEnd_, usersInClub[i]);
        std::cout << retEv;
    }

    users_.clear();
}

void Club::PrintIncome() {
    for (size_t i = 0; i < tables_.size(); ++i) {
        std::cout << i + 1 << " " << tables_[i].income << " " << tables_[i].timeInUse << "\n";
    }
}

TableIndex Club::FindFreeTable() const {
    if (busyTables_ == tablesNum_) {
        return -1;
    }
    TableIndex tableNum = 0;
    while (tables_[tableNum].isBusy) {
        ++tableNum;
    }
    return tableNum;
}

}  // namespace club
