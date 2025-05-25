#include "club.h"

namespace club {

void Club::Run(File& file, OStream& output) {
    Line line;
    Line remaining;
    ILineStream lineStream;

    std::getline(file, line);
    lineStream = ILineStream(line);
    if (!(lineStream >> tablesNum_) || tablesNum_ <= 0 || (lineStream >> remaining)) {
        throw std::invalid_argument(line);
    }

    std::getline(file, line);
    lineStream = ILineStream(line);
    if (!(lineStream >> workDayStart_ >> workDayEnd_) || (lineStream >> remaining)) {
        throw std::invalid_argument(line);
    }

    std::getline(file, line);
    lineStream = ILineStream(line);
    if (!(lineStream >> hourCost_) || hourCost_ <= 0 || (lineStream >> remaining)) {
        throw std::invalid_argument(line);
    }

    tables_ = std::vector<Table>(tablesNum_, Table(hourCost_));

    OLineStream buffer;
    HandleEvents(file, buffer);
    output << buffer.str();
}

void Club::HandleEvents(File& file, OLineStream& oss) {
    oss << workDayStart_ << "\n";

    Line line;
    Line remaining;
    ILineStream lineStream;
    while (std::getline(file, line)) {
        lineStream = ILineStream(line);
        Event ev(tablesNum_);
        if (!(lineStream >> ev) || (lineStream >> remaining)) {
            throw std::invalid_argument(line);
        }
        HandleEvent(ev, oss);
    }

    HandleLastUsers(oss);
    oss << workDayEnd_ << "\n";
    PrintIncome(oss);
}

void Club::HandleEvent(const Event& ev, OLineStream& oss) {
    Event newEv;
    switch (ev.id) {
        case 1:
            newEv = NewClientArrived(ev);
            break;
        case 2:
            newEv = NewSitAtTable(ev);
            break;
        case 3:
            newEv = NewClientWaiting(ev);
            break;
        case 4:
            newEv = NewClientLeaving(ev);
            break;
        default:
            break;
    }
    oss << ev << newEv;
}

Event Club::NewClientArrived(const Event& ev) {  // <время> 1 <имя клиента>
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

Event Club::NewSitAtTable(const Event& ev) {  // <время> 2 <имя клиента> <номер стола>
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
        TableIndex tableIndex = users_[ev.name].table.value();
        tables_[tableIndex].StopSession(ev.time);
    }
    if (users_[ev.name].status == Status::InQueue) {
        LeaveQueue(ev.name);
    }
    ++busyTables_;
    tables_[ev.tableNum].StartSession(ev.time);
    users_[ev.name] = {Status::AtTable, ev.tableNum, std::nullopt};
    return retEv;
}

Event Club::NewClientWaiting(const Event& ev) {  // <время> 3 <имя клиента>
    Event retEv;
    if (busyTables_ < tablesNum_) {
        retEv = Event::Error(ev.time, "ICanWaitNoLonger!");
        return retEv;
    }
    if (queue_.size() >= tablesNum_) {
        retEv = Event::ClientLeaving(ev.time, ev.name);
        users_.erase(ev.name);
        return retEv;
    }
    queue_.emplace_back(ev.name);
    users_[ev.name] = {Status::InQueue, std::nullopt, std::prev(queue_.end())};
    return retEv;
}

Event Club::NewClientLeaving(const Event& ev) {  // <время> 4 <имя клиента>
    Event retEv;
    if (users_.find(ev.name) == users_.end()) {
        retEv = Event::Error(ev.time, "ClientUnknown");
        return retEv;
    }
    if (users_[ev.name].status == Status::AtTable) {
        --busyTables_;
        TableIndex tableIndex = users_[ev.name].table.value();
        tables_[tableIndex].StopSession(ev.time);
    }
    if (users_[ev.name].status == Status::InQueue) {
        LeaveQueue(ev.name);
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

void Club::HandleLastUsers(OLineStream& oss) {
    for (size_t i = 0; i < tables_.size(); ++i) {
        tables_[i].StopSession(workDayEnd_);
    }

    std::vector<Username> usersInClub;
    for (const auto& pair : users_) {
        usersInClub.push_back(pair.first);
    }
    std::sort(usersInClub.begin(), usersInClub.end());

    for (size_t i = 0; i < usersInClub.size(); ++i) {
        Event newEv = Event::ClientLeaving(workDayEnd_, usersInClub[i]);
        oss << newEv;
    }

    users_.clear();
}

void Club::PrintIncome(OLineStream& oss) {
    for (size_t i = 0; i < tables_.size(); ++i) {
        oss << i + 1 << " " << tables_[i].income << " " << tables_[i].timeInUse << "\n";
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

void Club::LeaveQueue(Username name) {
    queue_.erase(users_[name].placeInQueue.value());
}

}  // namespace club
