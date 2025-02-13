#include <bits/stdc++.h>
#include "parser.h"

#define all(v) v.begin(), v.end()

using namespace std;

const string TRACE = "trace";
const string SHOW_STATISTICS = "stats";
const string ALGORITHMS[9] = {"", "FCFS", "RR-", "SPN", "SRT", "HRRN", "FB-1", "FB-2i"};

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
bool byPriorityLevel(const tuple<int, int, int> &a, const tuple<int, int, int> &b)
{
    if (get<0>(a) == get<0>(b))
        return get<2>(a) > get<2>(b);
    return get<0>(a) > get<0>(b);
} // if priority level are same sort by service time

void clear_timeline()
{
    for (int i = 0; i < last_instant; i++)
        for (int j = 0; j < process_count; j++)
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
    return (wait_time + service_time) * 1.0 / service_time;
}

void fillInWaitTime()
{
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
    for (int i = 0; i < process_count; i++)
    {
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
    for (int i = 0; i < last_instant; i++)
    {
        // Add all processes arriving at or before the current time
        while (j < process_count && getArrivalTime(processes[j]) <= i)
        {
            pq.push(make_pair(getServiceTime(processes[j]), j));
            j++;
        }

        if (!pq.empty())
        {
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
    for (int i = 0; i < last_instant; i++)
    {
        // Add all processes arriving at the current time
        while (j < process_count && getArrivalTime(processes[j]) == i)
        {
            pq.push(make_pair(getServiceTime(processes[j]), j));
            j++;
        }

        if (!pq.empty())
        {
            int processIndex = pq.top().second;
            int remainingTime = pq.top().first;
            pq.pop();
            int serviceTime = getServiceTime(processes[processIndex]);
            int arrivalTime = getArrivalTime(processes[processIndex]);

            timeline[i][processIndex] = '*';

            // If the process finishes, calculate times; otherwise, update remaining time
            if (remainingTime == 1)
            {
                finishTime[processIndex] = i + 1;
                turnAroundTime[processIndex] = finishTime[processIndex] - arrivalTime;
                normTurn[processIndex] = turnAroundTime[processIndex] * 1.0 / serviceTime;
            }
            else
            {
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
    if (getArrivalTime(processes[j]) == 0)
    {
        q.push(make_pair(j, getServiceTime(processes[j])));
        j++;
    }

    int currentQuantum = originalQuantum;

    for (int time = 0; time < last_instant; time++)
    {
        if (!q.empty())
        {
            int processIndex = q.front().first;
            int &remainingServiceTime = q.front().second;
            int arrivalTime = getArrivalTime(processes[processIndex]);
            int serviceTime = getServiceTime(processes[processIndex]);

            timeline[time][processIndex] = '*';
            remainingServiceTime--;
            currentQuantum--;

            // Check if the process has finished or quantum expired
            if (remainingServiceTime == 0)
            {
                finishTime[processIndex] = time + 1;
                turnAroundTime[processIndex] = finishTime[processIndex] - arrivalTime;
                normTurn[processIndex] = turnAroundTime[processIndex] * 1.0 / serviceTime;
                q.pop();
                currentQuantum = originalQuantum;
            }
            else if (currentQuantum == 0)
            {
                q.pop();
                q.push(make_pair(processIndex, remainingServiceTime));
                currentQuantum = originalQuantum;
            }
        }

        // Add new processes arriving at the current time
        while (j < process_count && getArrivalTime(processes[j]) == time + 1)
        {
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
    for (int i = 0; i < last_instant; i++)
    {
        // Add processes arriving at or before the current time
        while (j < process_count && getArrivalTime(processes[j]) <= i)
        {
            present_processes.push_back(make_tuple(getProcessName(processes[j]), 1.0, 0));
            j++;
        }

        // Update response ratios for all ready processes
        for (auto &proc : present_processes)
        {
            string process_name = get<0>(proc);
            int process_index = processToIndex[process_name];
            int wait_time = i - getArrivalTime(processes[process_index]);
            int service_time = getServiceTime(processes[process_index]);
            get<1>(proc) = calculate_response_ratio(wait_time, service_time);
        }

        // Sort processes by descending response ratio
        sort(all(present_processes), descendingly_by_response_ratio);

        if (!present_processes.empty())
        {
            int process_index = processToIndex[get<0>(present_processes[0])];
            while (i < last_instant && get<2>(present_processes[0]) != getServiceTime(processes[process_index]))
            {
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

// Feedback Queue Scheduling (FBQ1): Processes are executed for a fixed 1 time unit in every scheduling cycle.
void feedbackQ1()
{
    // Priority queue to schedule processes based on priority level (min-heap).
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    unordered_map<int, int> remainingServiceTime; // Tracks remaining service time for each process.
    int j = 0;

    // Add the first process if it arrives at time 0.
    if (getArrivalTime(processes[0]) == 0) {
        pq.push(make_pair(0, j));
        remainingServiceTime[j] = getServiceTime(processes[j]);
        j++;
    }

    // Iterate through each time unit.
    for (int time = 0; time < last_instant; time++) {
        if (!pq.empty()) {
            int priorityLevel = pq.top().first;
            int processIndex = pq.top().second;
            pq.pop();

            // Execute the current process for 1 unit of time.
            remainingServiceTime[processIndex]--;
            timeline[time][processIndex] = '*';

            // If the process is finished, calculate metrics.
            if (remainingServiceTime[processIndex] == 0) {
                finishTime[processIndex] = time + 1;
                turnAroundTime[processIndex] = finishTime[processIndex] - getArrivalTime(processes[processIndex]);
                normTurn[processIndex] = turnAroundTime[processIndex] * 1.0 / getServiceTime(processes[processIndex]);
            } else {
                // Re-add the process to the queue with adjusted priority.
                if (pq.size() >= 1)
                    pq.push(make_pair(priorityLevel + 1, processIndex));
                else
                    pq.push(make_pair(priorityLevel, processIndex));
            }
        }

        // Add newly arrived processes to the queue.
        while (j < process_count && getArrivalTime(processes[j]) == time + 1) {
            pq.push(make_pair(0, j));
            remainingServiceTime[j] = getServiceTime(processes[j]);
            j++;
        }
    }

    fillInWaitTime();
}

// Feedback Queue Scheduling (FBQ2i): Processes are executed for a fixed quantum time unit in every scheduling cycle.
void feedbackQ2i()
{
    // Priority queue to schedule processes based on priority level (min-heap).
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    unordered_map<int, int> remainingServiceTime; // Tracks remaining service time for each process.
    int j = 0;

    // Add the first process if it arrives at time 0.
    if (getArrivalTime(processes[0]) == 0) {
        pq.push(make_pair(0, j));
        remainingServiceTime[j] = getServiceTime(processes[j]);
        j++;
    }

    // Iterate through each time unit.
    for (int time = 0; time < last_instant; time++) {
        if (!pq.empty()) {
            int priorityLevel = pq.top().first;
            int processIndex = pq.top().second;
            pq.pop();

            // Execute the process for a quantum time based on priority.
            int currentQuantum = pow(2, priorityLevel);
            int temp = time;
            while (currentQuantum && remainingServiceTime[processIndex]) {
                currentQuantum--;
                remainingServiceTime[processIndex]--;
                timeline[temp++][processIndex] = '*';
            }

            // If the process is finished, calculate metrics.
            if (remainingServiceTime[processIndex] == 0) {
                finishTime[processIndex] = temp;
                turnAroundTime[processIndex] = finishTime[processIndex] - getArrivalTime(processes[processIndex]);
                normTurn[processIndex] = turnAroundTime[processIndex] * 1.0 / getServiceTime(processes[processIndex]);
            } else {
                // Re-add the process to the queue with adjusted priority.
                if (pq.size() >= 1)
                    pq.push(make_pair(priorityLevel + 1, processIndex));
                else
                    pq.push(make_pair(priorityLevel, processIndex));
            }

            time = temp - 1;
        }

        // Add newly arrived processes to the queue.
        while (j < process_count && getArrivalTime(processes[j]) <= time + 1) {
            pq.push(make_pair(0, j));
            remainingServiceTime[j] = getServiceTime(processes[j]);
            j++;
        }
    }

    fillInWaitTime();
}

void printAlgorithm(int algorithm_index)
{
    int algorithm_id = algorithms[algorithm_index].first - '0';
    if(algorithm_id==2)
        cout << ALGORITHMS[algorithm_id] <<algorithms[algorithm_index].second <<endl;
    else
        cout << ALGORITHMS[algorithm_id] << endl;
}

void printProcesses()
{
    cout << "Process    ";
    for (int i = 0; i < process_count; i++)
        cout << "|  " << getProcessName(processes[i]) << "  ";
    cout << "|\n";
}
void printArrivalTime()
{
    cout << "Arrival    ";
    for (int i = 0; i < process_count; i++)
        printf("|%3d  ",getArrivalTime(processes[i]));
    cout<<"|\n";
}
void printServiceTime()
{
    cout << "Service    |";
    for (int i = 0; i < process_count; i++)
        printf("%3d  |",getServiceTime(processes[i]));
    cout << " Mean|\n";
}
void printFinishTime()
{
    cout << "Finish     ";
    for (int i = 0; i < process_count; i++)
        printf("|%3d  ",finishTime[i]);
    cout << "|-----|\n";
}
void printTurnAroundTime()
{
    cout << "Turnaround |";
    int sum = 0;
    for (int i = 0; i < process_count; i++)
    {
        printf("%3d  |",turnAroundTime[i]);
        sum += turnAroundTime[i];
    }
    if((1.0 * sum / turnAroundTime.size())>=10)
		printf("%2.2f|\n",(1.0 * sum / turnAroundTime.size()));
    else
	 	printf(" %2.2f|\n",(1.0 * sum / turnAroundTime.size()));
}

void printNormTurn()
{
    cout << "NormTurn   |";
    float sum = 0;
    for (int i = 0; i < process_count; i++)
    {
        if( normTurn[i]>=10 )
            printf("%2.2f|",normTurn[i]);
        else
            printf(" %2.2f|",normTurn[i]);
        sum += normTurn[i];
    }

    if( (1.0 * sum / normTurn.size()) >=10 )
        printf("%2.2f|\n",(1.0 * sum / normTurn.size()));
	else
        printf(" %2.2f|\n",(1.0 * sum / normTurn.size()));
}
void printStats(int algorithm_index)
{
    printAlgorithm(algorithm_index);
    printProcesses();
    printArrivalTime();
    printServiceTime();
    printFinishTime();
    printTurnAroundTime();
    printNormTurn();
}

void printTimeline(int algorithm_index)
{
    for (int i = 0; i <= last_instant; i++)
        cout << i % 10<<" ";
    cout <<"\n";
    cout << "------------------------------------------------\n";
    for (int i = 0; i < process_count; i++)
    {
        cout << getProcessName(processes[i]) << "     |";
        for (int j = 0; j < last_instant; j++)
        {
            cout << timeline[j][i]<<"|";
        }
        cout << " \n";
    }
    cout << "------------------------------------------------\n";
}

void execute_algorithm(char algorithm_id, int quantum,string operation)
{
    switch (algorithm_id)
    {
    case '1':
        if(operation==TRACE)cout<<"FCFS  ";
        firstComeFirstServe();
        break;
    case '2':
        if(operation==TRACE)cout<<"RR-"<<quantum<<"  ";
        roundRobin(quantum);
        break;
    case '3':
        if(operation==TRACE)cout<<"SPN   ";
        shortestJobFirst();
        break;
    case '4':
        if(operation==TRACE)cout<<"SRT   ";
        shortestRemainingTimeFirst();
        break;
    case '5':
        if(operation==TRACE)cout<<"HRRN  ";
        highestResponseRatioNext();
        break;
    case '6':
        if(operation==TRACE)cout<<"FB-1  ";
        feedbackQ1();
        break;
    case '7':
        if(operation==TRACE)cout<<"FB-2i ";
        feedbackQ2i();
        break;
    default:
        break;
    }
}

int main()
{
    parse();
    for (int idx = 0; idx < (int)algorithms.size(); idx++)
    {
        clear_timeline();
        execute_algorithm(algorithms[idx].first, algorithms[idx].second,operation);
        if (operation == TRACE)
            printTimeline(idx);
        else if (operation == SHOW_STATISTICS)
            printStats(idx);
        cout << "\n";
    }
    return 0;
}
