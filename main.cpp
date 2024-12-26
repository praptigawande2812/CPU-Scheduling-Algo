#include <bits/stdc++.h>
#include "parser.h"

#define all(v) v.begin(), v.end()

using namespace std;

const string TRACE = "trace";
const string SHOW_STATISTICS = "stats";
const string ALGORITHMS[9] = {"", "FCFS", "RR-", "SPN", "SRT", "HRRN", "FB-1", "FB-2i", "AGING"};

bool sortByServiceTime(const tuple<string, int, int> &a, const tuple<string, int, int> &b)
{
    return (get<2>(a) < get<2>(b));
}
bool sortByArrivalTime(const tuple<string, int, int> &a, const tuple<string, int, int> &b)
{
    return (get<1>(a) < get<1>(b));
}
bool descendingly_by_response_ratio(tuple<string, double, int> a, tuple<string, double, int> b)
{
    return get<1>(a) > get<1>(b);
}
bool byPriorityLevel (const tuple<int,int,int>&a,const tuple<int,int,int>&b){
    if(get<0>(a)==get<0>(b))
        return get<2>(a)> get<2>(b);
    return get<0>(a) > get<0>(b);
}//if priority level are same sort by service time

void clear_timeline()
{
    for(int i=0; i<last_instant; i++)
        for(int j=0; j<process_count; j++)
            timeline[i][j] = ' ';
}

string getProcessName(tuple<string, int, int> &a)
{
    return get<0>(a);
}

int getArrivalTime(tuple<string, int, int> &a)
{
    return get<1>(a);
}

int getServiceTime(tuple<string, int, int> &a)
{
    return get<2>(a);
}

int getPriorityLevel(tuple<string, int, int> &a)
{
    return get<2>(a);
}

double calculate_response_ratio(int wait_time, int service_time)
{
    return (wait_time + service_time)*1.0 / service_time;
}

void fillInWaitTime(){
    for (int i = 0; i < process_count; i++)
    {
        int arrivalTime = getArrivalTime(processes[i]);
        for (int k = arrivalTime; k < finishTime[i]; k++)
        {
            if (timeline[k][i] != '*')
                timeline[k][i] = '.';
        }
    }
}

// First-Come, First-Serve Scheduling (FCFS)
void firstComeFirstServe()
{
    int time = getArrivalTime(processes[0]); // Start at the arrival time of the first process
    for (int i = 0; i < process_count; i++) {
        int processIndex = i;
        int arrivalTime = getArrivalTime(processes[i]);
        int serviceTime = getServiceTime(processes[i]);

        // Calculate finish time, turnaround time, and normalized turnaround time
        finishTime[processIndex] = time + serviceTime;
        turnAroundTime[processIndex] = finishTime[processIndex] - arrivalTime;
        normTurn[processIndex] = turnAroundTime[processIndex] * 1.0 / serviceTime;

        for (int j = time; j < finishTime[processIndex]; j++)
            timeline[j][processIndex] = '*';
        for (int j = arrivalTime; j < time; j++)
            timeline[j][processIndex] = '.';

        // Update current time after completing the process
        time += serviceTime;
    }
}

// Shortest Job First Scheduling (SJF)
void shortestJobFirst()
{
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq; // Min-heap based on service time
    int j = 0;
    for (int i = 0; i < last_instant; i++) {
        // Add all processes arriving at or before the current time
        while (j < process_count && getArrivalTime(processes[j]) <= i) {
            pq.push(make_pair(getServiceTime(processes[j]), j));
            j++;
        }

        if (!pq.empty()) {
            int processIndex = pq.top().second; // Process with shortest service time
            int arrivalTime = getArrivalTime(processes[processIndex]);
            int serviceTime = getServiceTime(processes[processIndex]);
            pq.pop();

            for (int temp = arrivalTime; temp < i; temp++)
                timeline[temp][processIndex] = '.';

            for (int temp = i; temp < i + serviceTime; temp++)
                timeline[temp][processIndex] = '*';

            // Calculate finish time, turnaround time, and normalized turnaround time
            finishTime[processIndex] = i + serviceTime;
            turnAroundTime[processIndex] = finishTime[processIndex] - arrivalTime;
            normTurn[processIndex] = turnAroundTime[processIndex] * 1.0 / serviceTime;

            i += serviceTime - 1; // Skip to the end of the process execution
        }
    }
}

// Shortest Remaining Time First Scheduling (SRTF)
void shortestRemainingTimeFirst()
{
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq; // Min-heap based on remaining time
    int j = 0;
    for (int i = 0; i < last_instant; i++) {
        // Add all processes arriving at the current time
        while (j < process_count && getArrivalTime(processes[j]) == i) {
            pq.push(make_pair(getServiceTime(processes[j]), j));
            j++;
        }

        if (!pq.empty()) {
            int processIndex = pq.top().second;
            int remainingTime = pq.top().first;
            pq.pop();
            int serviceTime = getServiceTime(processes[processIndex]);
            int arrivalTime = getArrivalTime(processes[processIndex]);

            timeline[i][processIndex] = '*';

            // If the process finishes, calculate times; otherwise, update remaining time
            if (remainingTime == 1) {
                finishTime[processIndex] = i + 1;
                turnAroundTime[processIndex] = finishTime[processIndex] - arrivalTime;
                normTurn[processIndex] = turnAroundTime[processIndex] * 1.0 / serviceTime;
            } else {
                pq.push(make_pair(remainingTime - 1, processIndex));
            }
        }
    }
    fillInWaitTime();
}

// Round Robin Scheduling (RR)
void roundRobin(int originalQuantum)
{
    queue<pair<int, int>> q; // Queue to store process index and remaining service time
    int j = 0;

    // Add the first process if it arrives at time 0
    if (getArrivalTime(processes[j]) == 0) {
        q.push(make_pair(j, getServiceTime(processes[j])));
        j++;
    }

    int currentQuantum = originalQuantum;

    for (int time = 0; time < last_instant; time++) {
        if (!q.empty()) {
            int processIndex = q.front().first;
            int &remainingServiceTime = q.front().second; 
            int arrivalTime = getArrivalTime(processes[processIndex]);
            int serviceTime = getServiceTime(processes[processIndex]);

            timeline[time][processIndex] = '*';
            remainingServiceTime--;
            currentQuantum--;

            // Check if the process has finished or quantum expired
            if (remainingServiceTime == 0) {
                finishTime[processIndex] = time + 1;
                turnAroundTime[processIndex] = finishTime[processIndex] - arrivalTime;
                normTurn[processIndex] = turnAroundTime[processIndex] * 1.0 / serviceTime;
                q.pop();
                currentQuantum = originalQuantum;
            } else if (currentQuantum == 0) {
                q.pop();
                q.push(make_pair(processIndex, remainingServiceTime));
                currentQuantum = originalQuantum;
            }
        }

        // Add new processes arriving at the current time
        while (j < process_count && getArrivalTime(processes[j]) == time + 1) {
            q.push(make_pair(j, getServiceTime(processes[j])));
            j++;
        }
    }

    fillInWaitTime();
}

// Highest Response Ratio Next Scheduling (HRRN)
void highestResponseRatioNext()
{
    vector<tuple<string, double, int>> present_processes; // Tuple: <process_name, response_ratio, time_in_service>
    int j = 0;
    for (int i = 0; i < last_instant; i++) {
        // Add processes arriving at or before the current time
        while (j < process_count && getArrivalTime(processes[j]) <= i) {
            present_processes.push_back(make_tuple(getProcessName(processes[j]), 1.0, 0));
            j++;
        }

        // Update response ratios for all ready processes
        for (auto &proc : present_processes) {
            string process_name = get<0>(proc);
            int process_index = processToIndex[process_name];
            int wait_time = i - getArrivalTime(processes[process_index]);
            int service_time = getServiceTime(processes[process_index]);
            get<1>(proc) = calculate_response_ratio(wait_time, service_time);
        }

        // Sort processes by descending response ratio
        sort(all(present_processes), descendingly_by_response_ratio);

        if (!present_processes.empty()) {
            int process_index = processToIndex[get<0>(present_processes[0])];
            while (i < last_instant && get<2>(present_processes[0]) != getServiceTime(processes[process_index])) {
                timeline[i][process_index] = '*';
                i++;
                get<2>(present_processes[0])++;
            }
            i--;
            present_processes.erase(present_processes.begin());

            // Calculate finish time, turnaround time, and normalized turnaround time
            finishTime[process_index] = i + 1;
            turnAroundTime[process_index] = finishTime[process_index] - getArrivalTime(processes[process_index]);
            normTurn[process_index] = turnAroundTime[process_index] * 1.0 / getServiceTime(processes[process_index]);
        }
    }
    fillInWaitTime(); 
}
