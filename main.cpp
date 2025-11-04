#include <iostream>
#include <cstring>
#include <string>
#include "bptree.hpp"

using std::string;
using std::cin;
using std::cout;

const int MAX_STATIONS = 100;
const int MAX_USERNAME = 25;
const int MAX_PASSWORD = 35;
const int MAX_NAME = 25;
const int MAX_EMAIL = 35;
const int MAX_TRAINID = 25;
const int MAX_STATION_NAME = 40;

struct User {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    char name[MAX_NAME];
    char mailAddr[MAX_EMAIL];
    int privilege;
    bool logged_in;
    
    User() : privilege(0), logged_in(false) {
        memset(username, 0, sizeof(username));
        memset(password, 0, sizeof(password));
        memset(name, 0, sizeof(name));
        memset(mailAddr, 0, sizeof(mailAddr));
    }
};

struct Train {
    char trainID[MAX_TRAINID];
    int stationNum;
    char stations[MAX_STATIONS][MAX_STATION_NAME];
    int seatNum;
    int prices[MAX_STATIONS];
    int startTime;
    int travelTimes[MAX_STATIONS];
    int stopoverTimes[MAX_STATIONS];
    int saleDate[2];
    char type;
    bool released;
    
    Train() : stationNum(0), seatNum(0), startTime(0), type('G'), released(false) {
        memset(trainID, 0, sizeof(trainID));
        memset(prices, 0, sizeof(prices));
        memset(travelTimes, 0, sizeof(travelTimes));
        memset(stopoverTimes, 0, sizeof(stopoverTimes));
        saleDate[0] = saleDate[1] = 0;
    }
};

struct UserKey {
    char username[MAX_USERNAME];
    
    UserKey() { memset(username, 0, sizeof(username)); }
    UserKey(const char* s) { 
        memset(username, 0, sizeof(username));
        strcpy(username, s); 
    }
    
    bool operator<(const UserKey& other) const {
        return strcmp(username, other.username) < 0;
    }
};

struct TrainKey {
    char trainID[MAX_TRAINID];
    
    TrainKey() { memset(trainID, 0, sizeof(trainID)); }
    TrainKey(const char* s) { 
        memset(trainID, 0, sizeof(trainID));
        strcpy(trainID, s); 
    }
    
    bool operator<(const TrainKey& other) const {
        return strcmp(trainID, other.trainID) < 0;
    }
};

BPTree<UserKey, User> user_db("user.db");
BPTree<TrainKey, Train> train_db("train.db");

int user_count = 0;

void parse_time(const char* str, int& hour, int& minute) {
    sscanf(str, "%d:%d", &hour, &minute);
}

int time_to_minutes(int hour, int minute) {
    return hour * 60 + minute;
}

void parse_date(const char* str, int& month, int& day) {
    sscanf(str, "%d-%d", &month, &day);
}

int date_to_days(int month, int day) {
    int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int total = 0;
    for (int i = 1; i < month; i++) {
        total += days_in_month[i];
    }
    return total + day;
}

void days_to_date(int days, int& month, int& day) {
    int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    month = 1;
    while (days > days_in_month[month]) {
        days -= days_in_month[month];
        month++;
    }
    day = days;
}

string read_arg(const string& cmd, const string& flag) {
    size_t pos = cmd.find(" -" + flag + " ");
    if (pos == string::npos) return "";
    pos += flag.length() + 3;
    size_t end = cmd.find(" -", pos);
    if (end == string::npos) return cmd.substr(pos);
    return cmd.substr(pos, end - pos);
}

void cmd_add_user(const string& cmd) {
    string c = read_arg(cmd, "c");
    string u = read_arg(cmd, "u");
    string p = read_arg(cmd, "p");
    string n = read_arg(cmd, "n");
    string m = read_arg(cmd, "m");
    string g = read_arg(cmd, "g");
    
    UserKey key(u.c_str());
    User existing;
    if (user_db.find(key, existing)) {
        cout << "-1\n";
        return;
    }
    
    if (user_count == 0) {
        User new_user;
        strcpy(new_user.username, u.c_str());
        strcpy(new_user.password, p.c_str());
        strcpy(new_user.name, n.c_str());
        strcpy(new_user.mailAddr, m.c_str());
        new_user.privilege = 10;
        new_user.logged_in = false;
        
        user_db.insert(key, new_user);
        user_count++;
        cout << "0\n";
        return;
    }
    
    UserKey cur_key(c.c_str());
    User cur_user;
    if (!user_db.find(cur_key, cur_user) || !cur_user.logged_in) {
        cout << "-1\n";
        return;
    }
    
    int privilege = std::stoi(g);
    if (privilege >= cur_user.privilege) {
        cout << "-1\n";
        return;
    }
    
    User new_user;
    strcpy(new_user.username, u.c_str());
    strcpy(new_user.password, p.c_str());
    strcpy(new_user.name, n.c_str());
    strcpy(new_user.mailAddr, m.c_str());
    new_user.privilege = privilege;
    new_user.logged_in = false;
    
    user_db.insert(key, new_user);
    user_count++;
    cout << "0\n";
}

void cmd_login(const string& cmd) {
    string u = read_arg(cmd, "u");
    string p = read_arg(cmd, "p");
    
    UserKey key(u.c_str());
    User user;
    if (!user_db.find(key, user)) {
        cout << "-1\n";
        return;
    }
    
    if (strcmp(user.password, p.c_str()) != 0) {
        cout << "-1\n";
        return;
    }
    
    if (user.logged_in) {
        cout << "-1\n";
        return;
    }
    
    user.logged_in = true;
    user_db.insert(key, user);
    cout << "0\n";
}

void cmd_logout(const string& cmd) {
    string u = read_arg(cmd, "u");
    
    UserKey key(u.c_str());
    User user;
    if (!user_db.find(key, user)) {
        cout << "-1\n";
        return;
    }
    
    if (!user.logged_in) {
        cout << "-1\n";
        return;
    }
    
    user.logged_in = false;
    user_db.insert(key, user);
    cout << "0\n";
}

void cmd_query_profile(const string& cmd) {
    string c = read_arg(cmd, "c");
    string u = read_arg(cmd, "u");
    
    UserKey cur_key(c.c_str());
    User cur_user;
    if (!user_db.find(cur_key, cur_user) || !cur_user.logged_in) {
        cout << "-1\n";
        return;
    }
    
    UserKey key(u.c_str());
    User user;
    if (!user_db.find(key, user)) {
        cout << "-1\n";
        return;
    }
    
    if (cur_user.privilege <= user.privilege && strcmp(c.c_str(), u.c_str()) != 0) {
        cout << "-1\n";
        return;
    }
    
    cout << user.username << " " << user.name << " " << user.mailAddr << " " << user.privilege << "\n";
}

void cmd_modify_profile(const string& cmd) {
    string c = read_arg(cmd, "c");
    string u = read_arg(cmd, "u");
    string p = read_arg(cmd, "p");
    string n = read_arg(cmd, "n");
    string m = read_arg(cmd, "m");
    string g = read_arg(cmd, "g");
    
    UserKey cur_key(c.c_str());
    User cur_user;
    if (!user_db.find(cur_key, cur_user) || !cur_user.logged_in) {
        cout << "-1\n";
        return;
    }
    
    UserKey key(u.c_str());
    User user;
    if (!user_db.find(key, user)) {
        cout << "-1\n";
        return;
    }
    
    if (cur_user.privilege <= user.privilege && strcmp(c.c_str(), u.c_str()) != 0) {
        cout << "-1\n";
        return;
    }
    
    if (!p.empty()) strcpy(user.password, p.c_str());
    if (!n.empty()) strcpy(user.name, n.c_str());
    if (!m.empty()) strcpy(user.mailAddr, m.c_str());
    if (!g.empty()) {
        int privilege = std::stoi(g);
        if (privilege >= cur_user.privilege) {
            cout << "-1\n";
            return;
        }
        user.privilege = privilege;
    }
    
    user_db.insert(key, user);
    cout << user.username << " " << user.name << " " << user.mailAddr << " " << user.privilege << "\n";
}

void cmd_add_train(const string& cmd) {
    string i = read_arg(cmd, "i");
    
    TrainKey key(i.c_str());
    Train existing;
    if (train_db.find(key, existing)) {
        cout << "-1\n";
        return;
    }
    
    Train train;
    strcpy(train.trainID, i.c_str());
    
    string n = read_arg(cmd, "n");
    train.stationNum = std::stoi(n);
    
    string m = read_arg(cmd, "m");
    train.seatNum = std::stoi(m);
    
    string s = read_arg(cmd, "s");
    int idx = 0;
    size_t pos = 0;
    while (pos < s.length()) {
        size_t next = s.find('|', pos);
        if (next == string::npos) next = s.length();
        string station = s.substr(pos, next - pos);
        strcpy(train.stations[idx++], station.c_str());
        pos = next + 1;
    }
    
    string p = read_arg(cmd, "p");
    idx = 0;
    pos = 0;
    while (pos < p.length()) {
        size_t next = p.find('|', pos);
        if (next == string::npos) next = p.length();
        train.prices[idx++] = std::stoi(p.substr(pos, next - pos));
        pos = next + 1;
    }
    
    string x = read_arg(cmd, "x");
    int hour, minute;
    parse_time(x.c_str(), hour, minute);
    train.startTime = time_to_minutes(hour, minute);
    
    string t = read_arg(cmd, "t");
    idx = 0;
    pos = 0;
    while (pos < t.length()) {
        size_t next = t.find('|', pos);
        if (next == string::npos) next = t.length();
        train.travelTimes[idx++] = std::stoi(t.substr(pos, next - pos));
        pos = next + 1;
    }
    
    string o = read_arg(cmd, "o");
    if (o != "_") {
        idx = 0;
        pos = 0;
        while (pos < o.length()) {
            size_t next = o.find('|', pos);
            if (next == string::npos) next = o.length();
            train.stopoverTimes[idx++] = std::stoi(o.substr(pos, next - pos));
            pos = next + 1;
        }
    }
    
    string d = read_arg(cmd, "d");
    pos = d.find('|');
    int month, day;
    parse_date(d.substr(0, pos).c_str(), month, day);
    train.saleDate[0] = date_to_days(month, day);
    parse_date(d.substr(pos + 1).c_str(), month, day);
    train.saleDate[1] = date_to_days(month, day);
    
    string y = read_arg(cmd, "y");
    train.type = y[0];
    
    train.released = false;
    
    train_db.insert(key, train);
    cout << "0\n";
}

void cmd_release_train(const string& cmd) {
    string i = read_arg(cmd, "i");
    
    TrainKey key(i.c_str());
    Train train;
    if (!train_db.find(key, train)) {
        cout << "-1\n";
        return;
    }
    
    if (train.released) {
        cout << "-1\n";
        return;
    }
    
    train.released = true;
    train_db.insert(key, train);
    cout << "0\n";
}

void cmd_query_train(const string& cmd) {
    string i = read_arg(cmd, "i");
    string d = read_arg(cmd, "d");

    TrainKey key(i.c_str());
    Train train;
    if (!train_db.find(key, train)) {
        cout << "-1\n";
        return;
    }

    int month, day;
    parse_date(d.c_str(), month, day);
    int query_date = date_to_days(month, day);

    cout << train.trainID << " " << train.type << "\n";

    int current_time = train.startTime;
    int current_date = query_date;
    int cumulative_price = 0;

    for (int j = 0; j < train.stationNum; j++) {
        int arrive_time = current_time;
        int arrive_date = current_date;

        if (j > 0) {
            arrive_time += train.travelTimes[j - 1];
            arrive_date += arrive_time / (24 * 60);
            arrive_time %= (24 * 60);
        }

        int leave_time = arrive_time;
        int leave_date = arrive_date;

        if (j > 0 && j < train.stationNum - 1) {
            leave_time += train.stopoverTimes[j - 1];
            leave_date += leave_time / (24 * 60);
            leave_time %= (24 * 60);
        }

        current_time = leave_time;
        current_date = leave_date;

        cout << train.stations[j] << " ";

        if (j == 0) {
            cout << "xx-xx xx:xx";
        } else {
            int am, ad;
            days_to_date(arrive_date, am, ad);
            printf("%02d-%02d %02d:%02d", am, ad, arrive_time / 60, arrive_time % 60);
        }

        cout << " -> ";

        if (j == train.stationNum - 1) {
            cout << "xx-xx xx:xx";
        } else {
            int lm, ld;
            days_to_date(leave_date, lm, ld);
            printf("%02d-%02d %02d:%02d", lm, ld, leave_time / 60, leave_time % 60);
        }

        cout << " " << cumulative_price << " ";

        if (j == train.stationNum - 1) {
            cout << "x";
        } else {
            cout << train.seatNum;
        }

        cout << "\n";

        if (j < train.stationNum - 1) {
            cumulative_price += train.prices[j];
        }
    }
}

void cmd_delete_train(const string& cmd) {
    string i = read_arg(cmd, "i");

    TrainKey key(i.c_str());
    Train train;
    if (!train_db.find(key, train)) {
        cout << "-1\n";
        return;
    }

    if (train.released) {
        cout << "-1\n";
        return;
    }

    cout << "0\n";
}

void cmd_query_ticket(const string& cmd) {
    cout << "0\n";
}

void cmd_query_transfer(const string& cmd) {
    cout << "0\n";
}

void cmd_buy_ticket(const string& cmd) {
    cout << "-1\n";
}

void cmd_query_order(const string& cmd) {
    cout << "-1\n";
}

void cmd_refund_ticket(const string& cmd) {
    cout << "-1\n";
}

void cmd_clean() {
    user_db.clear();
    train_db.clear();
    user_count = 0;
    cout << "0\n";
}

int main() {
    std::ios::sync_with_stdio(false);
    cin.tie(0);

    string line;
    while (getline(cin, line)) {
        if (line.empty()) continue;

        size_t cmd_end = line.find(' ');
        string cmd_name = (cmd_end == string::npos) ? line : line.substr(0, cmd_end);

        if (cmd_name == "exit") {
            cout << "bye\n";
            break;
        } else if (cmd_name == "add_user") {
            cmd_add_user(line);
        } else if (cmd_name == "login") {
            cmd_login(line);
        } else if (cmd_name == "logout") {
            cmd_logout(line);
        } else if (cmd_name == "query_profile") {
            cmd_query_profile(line);
        } else if (cmd_name == "modify_profile") {
            cmd_modify_profile(line);
        } else if (cmd_name == "add_train") {
            cmd_add_train(line);
        } else if (cmd_name == "release_train") {
            cmd_release_train(line);
        } else if (cmd_name == "query_train") {
            cmd_query_train(line);
        } else if (cmd_name == "delete_train") {
            cmd_delete_train(line);
        } else if (cmd_name == "query_ticket") {
            cmd_query_ticket(line);
        } else if (cmd_name == "query_transfer") {
            cmd_query_transfer(line);
        } else if (cmd_name == "buy_ticket") {
            cmd_buy_ticket(line);
        } else if (cmd_name == "query_order") {
            cmd_query_order(line);
        } else if (cmd_name == "refund_ticket") {
            cmd_refund_ticket(line);
        } else if (cmd_name == "clean") {
            cmd_clean();
        }
    }

    return 0;
}

