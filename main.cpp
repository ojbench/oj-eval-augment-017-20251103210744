#include <iostream>
#include <fstream>
#include <cstring>
#include <string>

using std::string;
using std::cin;
using std::cout;

const int MAX_USERS = 100000;
const int MAX_TRAINS = 10000;
const int MAX_STATIONS = 100;
const int MAX_ORDERS = 1000000;

struct User {
    char username[25];
    char password[35];
    char name[25];
    char mailAddr[35];
    int privilege;
    bool logged_in;
    bool exists;
    
    User() : privilege(0), logged_in(false), exists(false) {
        memset(username, 0, sizeof(username));
        memset(password, 0, sizeof(password));
        memset(name, 0, sizeof(name));
        memset(mailAddr, 0, sizeof(mailAddr));
    }
};

struct Train {
    char trainID[25];
    int stationNum;
    char stations[MAX_STATIONS][40];
    int seatNum;
    int prices[MAX_STATIONS];
    int startTime;
    int travelTimes[MAX_STATIONS];
    int stopoverTimes[MAX_STATIONS];
    int saleDate[2];
    char type;
    bool released;
    bool exists;
    
    Train() : stationNum(0), seatNum(0), startTime(0), type('G'), released(false), exists(false) {
        memset(trainID, 0, sizeof(trainID));
        memset(prices, 0, sizeof(prices));
        memset(travelTimes, 0, sizeof(travelTimes));
        memset(stopoverTimes, 0, sizeof(stopoverTimes));
        saleDate[0] = saleDate[1] = 0;
    }
};

User users[MAX_USERS];
Train trains[MAX_TRAINS];
int user_count = 0;
int train_count = 0;

void load_data() {
    std::ifstream uf("users.dat", std::ios::binary);
    if (uf.is_open()) {
        uf.read((char*)&user_count, sizeof(int));
        uf.read((char*)users, sizeof(User) * user_count);
        uf.close();
    }
    
    std::ifstream tf("trains.dat", std::ios::binary);
    if (tf.is_open()) {
        tf.read((char*)&train_count, sizeof(int));
        tf.read((char*)trains, sizeof(Train) * train_count);
        tf.close();
    }
}

void save_data() {
    std::ofstream uf("users.dat", std::ios::binary);
    uf.write((char*)&user_count, sizeof(int));
    uf.write((char*)users, sizeof(User) * user_count);
    uf.close();
    
    std::ofstream tf("trains.dat", std::ios::binary);
    tf.write((char*)&train_count, sizeof(int));
    tf.write((char*)trains, sizeof(Train) * train_count);
    tf.close();
}

int find_user(const char* username) {
    for (int i = 0; i < user_count; i++) {
        if (users[i].exists && strcmp(users[i].username, username) == 0) {
            return i;
        }
    }
    return -1;
}

int find_train(const char* trainID) {
    for (int i = 0; i < train_count; i++) {
        if (trains[i].exists && strcmp(trains[i].trainID, trainID) == 0) {
            return i;
        }
    }
    return -1;
}

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
    
    if (find_user(u.c_str()) != -1) {
        cout << "-1\n";
        return;
    }
    
    int active_users = 0;
    for (int i = 0; i < user_count; i++) {
        if (users[i].exists) active_users++;
    }
    
    if (active_users == 0) {
        User& new_user = users[user_count++];
        strcpy(new_user.username, u.c_str());
        strcpy(new_user.password, p.c_str());
        strcpy(new_user.name, n.c_str());
        strcpy(new_user.mailAddr, m.c_str());
        new_user.privilege = 10;
        new_user.logged_in = false;
        new_user.exists = true;
        cout << "0\n";
        return;
    }
    
    int cur_idx = find_user(c.c_str());
    if (cur_idx == -1 || !users[cur_idx].logged_in) {
        cout << "-1\n";
        return;
    }
    
    int privilege = std::stoi(g);
    if (privilege >= users[cur_idx].privilege) {
        cout << "-1\n";
        return;
    }
    
    User& new_user = users[user_count++];
    strcpy(new_user.username, u.c_str());
    strcpy(new_user.password, p.c_str());
    strcpy(new_user.name, n.c_str());
    strcpy(new_user.mailAddr, m.c_str());
    new_user.privilege = privilege;
    new_user.logged_in = false;
    new_user.exists = true;
    cout << "0\n";
}

void cmd_login(const string& cmd) {
    string u = read_arg(cmd, "u");
    string p = read_arg(cmd, "p");
    
    int idx = find_user(u.c_str());
    if (idx == -1) {
        cout << "-1\n";
        return;
    }
    
    if (strcmp(users[idx].password, p.c_str()) != 0) {
        cout << "-1\n";
        return;
    }
    
    if (users[idx].logged_in) {
        cout << "-1\n";
        return;
    }
    
    users[idx].logged_in = true;
    cout << "0\n";
}

void cmd_logout(const string& cmd) {
    string u = read_arg(cmd, "u");
    
    int idx = find_user(u.c_str());
    if (idx == -1) {
        cout << "-1\n";
        return;
    }
    
    if (!users[idx].logged_in) {
        cout << "-1\n";
        return;
    }
    
    users[idx].logged_in = false;
    cout << "0\n";
}

void cmd_query_profile(const string& cmd) {
    string c = read_arg(cmd, "c");
    string u = read_arg(cmd, "u");
    
    int cur_idx = find_user(c.c_str());
    if (cur_idx == -1 || !users[cur_idx].logged_in) {
        cout << "-1\n";
        return;
    }
    
    int idx = find_user(u.c_str());
    if (idx == -1) {
        cout << "-1\n";
        return;
    }
    
    if (users[cur_idx].privilege <= users[idx].privilege && strcmp(c.c_str(), u.c_str()) != 0) {
        cout << "-1\n";
        return;
    }
    
    cout << users[idx].username << " " << users[idx].name << " " << users[idx].mailAddr << " " << users[idx].privilege << "\n";
}

void cmd_modify_profile(const string& cmd) {
    string c = read_arg(cmd, "c");
    string u = read_arg(cmd, "u");
    string p = read_arg(cmd, "p");
    string n = read_arg(cmd, "n");
    string m = read_arg(cmd, "m");
    string g = read_arg(cmd, "g");
    
    int cur_idx = find_user(c.c_str());
    if (cur_idx == -1 || !users[cur_idx].logged_in) {
        cout << "-1\n";
        return;
    }
    
    int idx = find_user(u.c_str());
    if (idx == -1) {
        cout << "-1\n";
        return;
    }
    
    if (users[cur_idx].privilege <= users[idx].privilege && strcmp(c.c_str(), u.c_str()) != 0) {
        cout << "-1\n";
        return;
    }
    
    if (!p.empty()) strcpy(users[idx].password, p.c_str());
    if (!n.empty()) strcpy(users[idx].name, n.c_str());
    if (!m.empty()) strcpy(users[idx].mailAddr, m.c_str());
    if (!g.empty()) {
        int privilege = std::stoi(g);
        if (privilege >= users[cur_idx].privilege) {
            cout << "-1\n";
            return;
        }
        users[idx].privilege = privilege;
    }
    
    cout << users[idx].username << " " << users[idx].name << " " << users[idx].mailAddr << " " << users[idx].privilege << "\n";
}

void cmd_add_train(const string& cmd) {
    string i = read_arg(cmd, "i");

    if (find_train(i.c_str()) != -1) {
        cout << "-1\n";
        return;
    }

    Train& train = trains[train_count++];
    strcpy(train.trainID, i.c_str());
    train.exists = true;

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

    cout << "0\n";
}

void cmd_release_train(const string& cmd) {
    string i = read_arg(cmd, "i");

    int idx = find_train(i.c_str());
    if (idx == -1) {
        cout << "-1\n";
        return;
    }

    if (trains[idx].released) {
        cout << "-1\n";
        return;
    }

    trains[idx].released = true;
    cout << "0\n";
}

void cmd_query_train(const string& cmd) {
    string i = read_arg(cmd, "i");
    string d = read_arg(cmd, "d");

    int idx = find_train(i.c_str());
    if (idx == -1) {
        cout << "-1\n";
        return;
    }

    Train& train = trains[idx];

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

    int idx = find_train(i.c_str());
    if (idx == -1) {
        cout << "-1\n";
        return;
    }

    if (trains[idx].released) {
        cout << "-1\n";
        return;
    }

    trains[idx].exists = false;
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
    user_count = 0;
    train_count = 0;
    for (int i = 0; i < MAX_USERS; i++) {
        users[i] = User();
    }
    for (int i = 0; i < MAX_TRAINS; i++) {
        trains[i] = Train();
    }
    cout << "0\n";
}

int main() {
    std::ios::sync_with_stdio(false);
    cin.tie(0);

    load_data();

    string line;
    while (getline(cin, line)) {
        if (line.empty()) continue;

        size_t cmd_end = line.find(' ');
        string cmd_name = (cmd_end == string::npos) ? line : line.substr(0, cmd_end);

        if (cmd_name == "exit") {
            save_data();
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

