#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <string>
#include <algorithm>
#include <chrono>

using namespace std;
using namespace std::chrono;

class Task {
public:
    string name;
    string description;
    string deadline;
    string status;
    int priority;

    Task(const string& na, const string& des, const string& dl, int pri, const string& stat = "Pending")
        : name(na), description(des), deadline(dl), status(stat), priority(pri) {}

    bool operator<(const Task& other) const {
        return priority < other.priority;
    }
};

class TaskManager {
private:
    priority_queue<Task> pq;
    int maxTasks;
    string filename;
    bool headerWritten;

public:
    TaskManager(int max, const string& file) : maxTasks(max), filename(file), headerWritten(false) {
        loadFromCSV(); // Load tasks from the CSV file
    }

    void addTask(const string& name, const string& description, const string& deadline, int priority) {
        if (pq.size() >= maxTasks) {
            cerr << "Maximum number of tasks reached. Cannot add more tasks." << endl;
            return;
        }
        Task task(name, description, deadline, priority);
        pq.push(task);
        saveToCSV(task); // Append the newly added task to the CSV file
    }

    void processTask(int index) {
        if (index < 1 || index > pq.size()) {
            cerr << "Invalid task index." << endl;
            return;
        }

        priority_queue<Task> tempQueue;
        int i = 1;
        auto currentTime = system_clock::to_time_t(system_clock::now());
        while (!pq.empty()) {
            Task task = pq.top();
            pq.pop();
            if (i == index) {
                if (currentTime > parseDeadline(task.deadline)) {
                    task.status = "Late Done";
                } else {
                    task.status = "Done";
                }
            }
            tempQueue.push(task);
            ++i;
        }

        pq = tempQueue;
        saveToCSV(); // Save the updated tasks to the CSV file
    }

    void showMissedTasks() const {
        auto currentTime = system_clock::to_time_t(system_clock::now());
        priority_queue<Task> missedTasks;
        priority_queue<Task> pqCopy = pq;
        while (!pqCopy.empty()) {
            Task task = pqCopy.top();
            pqCopy.pop();
            if (task.status != "Done" && task.status != "Late Done") {
                time_t deadlineTime = parseDeadline(task.deadline);
                if (currentTime > deadlineTime) {
                    task.status = "Missed";
                    missedTasks.push(task);
                }
            }
        }

        if (missedTasks.empty()) {
            cout << "No missed tasks." << endl;
            return;
        }

        cout << "Missed Tasks:" << endl;
        int index = 1;
        while (!missedTasks.empty()) {
            Task task = missedTasks.top();
            cout << index << ". Name: " << task.name << ", Description: " << task.description
                 << ", Deadline: " << task.deadline << ", Priority: " << task.priority
                 << ", Status: " << task.status << endl;
            missedTasks.pop();
            ++index;
        }
    }

    void suggestNextTask() const {
    vector<Task> tasks;
    priority_queue<Task> pqCopy = pq;
    
    while (!pqCopy.empty()) {
        tasks.push_back(pqCopy.top());
        pqCopy.pop();
    }

    sort(tasks.begin(), tasks.end(), [this](const Task& a, const Task& b) {
        time_t aDeadlineTime = parseDeadline(a.deadline);
        time_t bDeadlineTime = parseDeadline(b.deadline);
        return aDeadlineTime < bDeadlineTime;
    });

    if (tasks.empty()) {
        cout << "No tasks to suggest." << endl;
        return;
    }

    cout << "You should do the following task first:" << endl;
    Task nextTask = tasks.front();
    cout << "Name: " << nextTask.name << ", Description: " << nextTask.description
         << ", Deadline: " << nextTask.deadline << ", Priority: " << nextTask.priority << endl;
}
private:
    void loadFromCSV() {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Error: Unable to open file for reading." << endl;
            return;
        }

        string line;
        getline(file, line); // Skip header
        while (getline(file, line)) {
            stringstream ss(line);
            string name, description, deadline, status;
            int priority;
            getline(ss, name, ',');
            getline(ss, description, ',');
            getline(ss, deadline, ',');
            ss >> priority;
            getline(ss, status, ',');

            if (status.empty()) {
                status = "Pending";
            }

            pq.push(Task(name, description, deadline, priority, status));
        }

        file.close();
    }

    void saveToCSV(const Task& task) {
        ofstream file(filename, ios::app);
        if (!file.is_open()) {
            cerr << "Error: Unable to open file for writing." << endl;
            return;
        }

        file << task.name << "," << task.description << "," << task.deadline << ","
             << task.priority << "," << task.status << "\n";

        file.close();
    }

    void saveToCSV() {
        ofstream file(filename, ios::trunc);
        if (!file.is_open()) {
            cerr << "Error: Unable to open file for writing." << endl;
            return;
        }

        file << "Task Name,Description,Deadline,Priority,Status\n";

        priority_queue<Task> pqCopy = pq;
        while (!pqCopy.empty()) {
            Task task = pqCopy.top();
            file << task.name << "," << task.description << "," << task.deadline << ","
                 << task.priority << "," << task.status << "\n";
            pqCopy.pop();
        }

        file.close();
    }

    time_t parseDeadline(const string& dl) const {
        stringstream ss(dl);
        int hour, min;
        char colon;
        ss >> hour >> colon >> min;

        time_t now = time(nullptr);
        tm* deadline_tm = localtime(&now);
        deadline_tm->tm_hour = hour;
        deadline_tm->tm_min = min;
        deadline_tm->tm_sec = 0;

        return mktime(deadline_tm);
    }

public:
    void displayTasks() const {
        cout << "Tasks:" << endl;
        priority_queue<Task> pqCopy = pq;
        int index = 1;
        while (!pqCopy.empty()) {
            Task task = pqCopy.top();
            cout << index << ". Name: " << task.name << ", Description: " << task.description
                 << ", Deadline: " << task.deadline << ", Priority: " << task.priority
                 << ", Status: " << task.status << endl;
            pqCopy.pop();
            ++index;
        }
    }
};

int main() {
    cout << "Enter maximum number of tasks: ";
    int maxTasks;
    cin >> maxTasks;
    cin.ignore();

    cout << "Enter filename to save tasks: ";
    string filename;
    getline(cin, filename);

    TaskManager taskManager(maxTasks, filename);

    while (true) {
        cout << "--------------------------------------------------" << endl;
        cout << "Press 1 to add New Task" << endl;
        cout << "Press 2 to complete a Task" << endl;
        cout << "Press 3 to show Missed Tasks" << endl;
        cout << "Press 4 to Get A Summary of Your To-Do List" << endl;
        cout << "Press 5 to Display All Tasks" << endl;
        cout << "Press 6 to Exit from program" << endl;

        cout << "Enter your choice: ";

        int choice;
        cin >> choice;
        cin.ignore();

        switch (choice) {
            case 1: {
                cout << "Enter Task Name: ";
                string name;
                getline(cin, name);
                cout << "Enter Task Description: ";
                string description;
                getline(cin, description);
                cout << "Enter Task Deadline (hh:mm): ";
                string deadline;
                getline(cin, deadline);
                cout << "Enter Task Priority: ";
                int priority;
                cin >> priority;
                taskManager.addTask(name, description, deadline, priority);
                break;
            }

            case 2: {
                taskManager.displayTasks();
                cout << "Enter the index of the task to process: ";
                int index;
                cin >> index;
                taskManager.processTask(index);
                break;
            }

            case 3:
                taskManager.showMissedTasks();
                break;

            case 4:
                taskManager.suggestNextTask();
                break;

            case 5:
                taskManager.displayTasks();
                break;

            case 6:
                cout << "Exiting..." << endl;
                return 0;

            default:
                cout << "Invalid choice, please try again." << endl;
                break;
        }
    }
}
