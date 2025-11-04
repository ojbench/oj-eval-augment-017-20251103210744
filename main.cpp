#include <iostream>
#include <fstream>
#include <cstring>
#include <string>

using std::string;
using std::cin;
using std::cout;

const int MAX_STATIONS = 100;

struct User {
    char username[25];
    char password[35];
    char name[25];
    char mailAddr[35];
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

    Train() : stationNum(0), seatNum(0), startTime(0), type('G'), released(false) {
        memset(trainID, 0, sizeof(trainID));
        memset(prices, 0, sizeof(prices));
        memset(travelTimes, 0, sizeof(travelTimes));
        memset(stopoverTimes, 0, sizeof(stopoverTimes));
        saleDate[0] = saleDate[1] = 0;
    }
};

std::fstream user_file;
std::fstream train_file;

void init_files() {
    user_file.open("users.dat", std::ios::in | std::ios::out | std::ios::binary);
    if (!user_file.is_open()) {
        user_file.open("users.dat", std::ios::out | std::ios::binary);
        user_file.close();
        user_file.open("users.dat", std::ios::in | std::ios::out | std::ios::binary);
    }

    train_file.open("trains.dat", std::ios::in | std::ios::out | std::ios::binary);
    if (!train_file.is_open()) {
        train_file.open("trains.dat", std::ios::out | std::ios::binary);
        train_file.close();
        train_file.open("trains.dat", std::ios::in | std::ios::out | std::ios::binary);
    }
}

void close_files() {
    if (user_file.is_open()) user_file.close();
    if (train_file.is_open()) train_file.close();
}

int find_user(const char* username) {
    user_file.clear();
    user_file.seekg(0, std::ios::end);
    int count = user_file.tellg() / sizeof(User);
    user_file.seekg(0);

    User u;
    for (int i = 0; i < count; i++) {
        user_file.read((char*)&u, sizeof(User));
        if (strcmp(u.username, username) == 0) {
            return i;
        }
    }
    return -1;
}

User read_user(int idx) {
    User u;
    user_file.clear();
    user_file.seekg(idx * sizeof(User));
    user_file.read((char*)&u, sizeof(User));
    return u;
}

void write_user(int idx, const User& u) {
    user_file.clear();
    user_file.seekp(idx * sizeof(User));
    user_file.write((char*)&u, sizeof(User));
    user_file.flush();
}

void append_user(const User& u) {
    user_file.clear();
    user_file.seekp(0, std::ios::end);
    user_file.write((char*)&u, sizeof(User));
    user_file.flush();
}

int find_train(const char* trainID) {
    train_file.clear();
    train_file.seekg(0, std::ios::end);
    int count = train_file.tellg() / sizeof(Train);
    train_file.seekg(0);

    Train t;
    for (int i = 0; i < count; i++) {
        train_file.read((char*)&t, sizeof(Train));
        if (strcmp(t.trainID, trainID) == 0) {
            return i;
        }
    }
    return -1;
}

Train read_train(int idx) {
    Train t;
    train_file.clear();
    train_file.seekg(idx * sizeof(Train));
    train_file.read((char*)&t, sizeof(Train));
    return t;
}

void write_train(int idx, const Train& t) {
    train_file.clear();
    train_file.seekp(idx * sizeof(Train));
    train_file.write((char*)&t, sizeof(Train));
    train_file.flush();
}

void append_train(const Train& t) {
    train_file.clear();
    train_file.seekp(0, std::ios::end);
    train_file.write((char*)&t, sizeof(Train));
    train_file.flush();
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

    user_file.clear();
    user_file.seekg(0, std::ios::end);
    int user_count = user_file.tellg() / sizeof(User);

    if (user_count == 0) {
        User new_user;
        strcpy(new_user.username, u.c_str());
        strcpy(new_user.password, p.c_str());
        strcpy(new_user.name, n.c_str());
        strcpy(new_user.mailAddr, m.c_str());
        new_user.privilege = 10;
        new_user.logged_in = false;
        append_user(new_user);
        cout << "0\n";
        return;
    }

    int cur_idx = find_user(c.c_str());
    if (cur_idx == -1) {
        cout << "-1\n";
        return;
    }

    User cur_user = read_user(cur_idx);
    if (!cur_user.logged_in) {
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
    append_user(new_user);
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

    User user = read_user(idx);
    if (strcmp(user.password, p.c_str()) != 0) {
        cout << "-1\n";
        return;
    }

    if (user.logged_in) {
        cout << "-1\n";
        return;
    }

    user.logged_in = true;
    write_user(idx, user);
    cout << "0\n";
}

void cmd_logout(const string& cmd) {
    string u = read_arg(cmd, "u");

    int idx = find_user(u.c_str());
    if (idx == -1) {
        cout << "-1\n";
        return;
    }

    User user = read_user(idx);
    if (!user.logged_in) {
        cout << "-1\n";
        return;
    }

    user.logged_in = false;
    write_user(idx, user);
    cout << "0\n";
}

void cmd_query_profile(const string& cmd) {
    string c = read_arg(cmd, "c");
    string u = read_arg(cmd, "u");

    int cur_idx = find_user(c.c_str());
    if (cur_idx == -1) {
        cout << "-1\n";
        return;
    }

    User cur_user = read_user(cur_idx);
    if (!cur_user.logged_in) {
        cout << "-1\n";
        return;
    }

    int idx = find_user(u.c_str());
    if (idx == -1) {
        cout << "-1\n";
        return;
    }

    User user = read_user(idx);
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

    int cur_idx = find_user(c.c_str());
    if (cur_idx == -1) {
        cout << "-1\n";
        return;
    }

    User cur_user = read_user(cur_idx);
    if (!cur_user.logged_in) {
        cout << "-1\n";
        return;
    }

    int idx = find_user(u.c_str());
    if (idx == -1) {
        cout << "-1\n";
        return;
    }

    User user = read_user(idx);
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

    write_user(idx, user);
    cout << user.username << " " << user.name << " " << user.mailAddr << " " << user.privilege << "\n";
}

void cmd_add_train(const string& cmd) {
    string i = read_arg(cmd, "i");

    if (find_train(i.c_str()) != -1) {
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

    append_train(train);
    cout << "0\n";
}

void cmd_release_train(const string& cmd) {
    string i = read_arg(cmd, "i");

    int idx = find_train(i.c_str());
    if (idx == -1) {
        cout << "-1\n";
        return;
    }

    Train train = read_train(idx);
    if (train.released) {
        cout << "-1\n";
        return;
    }

    train.released = true;
    write_train(idx, train);
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

    Train train = read_train(idx);

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

    Train train = read_train(idx);
    if (train.released) {
        cout << "-1\n";
        return;
    }

    cout << "0\n";
}

void cmd_query_ticket(const string& cmd) {
    string s = read_arg(cmd, "s");
    string t = read_arg(cmd, "t");
    string d = read_arg(cmd, "d");
    string p = read_arg(cmd, "p");

    // For now, return 0 trains found
    cout << "0\n";
}

void cmd_query_transfer(const string& cmd) {
    // Return 0 for no transfer found
    cout << "0\n";
}

void cmd_buy_ticket(const string& cmd) {
    string u = read_arg(cmd, "u");

    int idx = find_user(u.c_str());
    if (idx == -1) {
        cout << "-1\n";
        return;
    }

    User user = read_user(idx);
    if (!user.logged_in) {
        cout << "-1\n";
        return;
    }

    // For now, fail all ticket purchases
    cout << "-1\n";
}

void cmd_query_order(const string& cmd) {
    string u = read_arg(cmd, "u");

    int idx = find_user(u.c_str());
    if (idx == -1) {
        cout << "-1\n";
        return;
    }

    User user = read_user(idx);
    if (!user.logged_in) {
        cout << "-1\n";
        return;
    }

    // Return 0 orders
    cout << "0\n";
}

void cmd_refund_ticket(const string& cmd) {
    string u = read_arg(cmd, "u");

    int idx = find_user(u.c_str());
    if (idx == -1) {
        cout << "-1\n";
        return;
    }

    User user = read_user(idx);
    if (!user.logged_in) {
        cout << "-1\n";
        return;
    }

    // For now, fail all refunds
    cout << "-1\n";
}

void cmd_clean() {
    close_files();
    std::remove("users.dat");
    std::remove("trains.dat");
    init_files();
    cout << "0\n";
}

int main() {
    std::ios::sync_with_stdio(false);
    cin.tie(0);

    init_files();

    string line;
    while (getline(cin, line)) {
        if (line.empty()) continue;

        size_t cmd_end = line.find(' ');
        string cmd_name = (cmd_end == string::npos) ? line : line.substr(0, cmd_end);

        if (cmd_name == "exit") {
            close_files();
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

