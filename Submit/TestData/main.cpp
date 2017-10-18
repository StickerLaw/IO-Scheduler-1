#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdint>
#include <queue>
#include <cmath>

#define ADD 0
#define ISSUE 1
#define FINISH 3

#define FORWARD 0
#define BACKWARD 1

#define A_ 0
#define B_ 1
using namespace std;

struct IORequest {
    int reqNum;
    int start_timestamp;
    int trackNum;
};


void readInstructions(string filename, vector<IORequest> & IORequests){
    static int incrReqNum = 0;
    ifstream infile;
    infile.open(filename);
    string line;
    while (getline(infile, line)){
        char newline[1024];
        strcpy(newline, line.c_str());
        char * line_ptr = strtok(newline, " \n");
        IORequest newIOReq;
        while (line_ptr != NULL){
            bool skip = false;
            if (line_ptr[0] == '#' || line_ptr[0] == '\t' || line_ptr[0] == ' ') {
                skip = true;
                break;
            }
            if (!skip) {
                newIOReq.start_timestamp = atoi(line_ptr);
                line_ptr = strtok(NULL, " \n");
                newIOReq.trackNum = atoi(line_ptr);
            }
            line_ptr = strtok(NULL, " \n");
            newIOReq.reqNum = incrReqNum;
            incrReqNum++;
            IORequests.push_back(newIOReq);
        }
    }
    
    infile.close();
}

void printIOReqs(vector<IORequest> & IORequests) {
    for (int i = 0; i < IORequests.size(); i++)
        cout << IORequests[i].reqNum << ": " << IORequests[i].start_timestamp << " " << IORequests[i].trackNum << endl;
}

struct Event {
    int timestamp;
    IORequest * IOReq;
    int instruction;
};

struct greaterThanByTimestamp {
    bool operator()(const Event lhs, Event rhs) const {
        return lhs.timestamp > rhs.timestamp;
    }
};

class EventQueue {
    priority_queue<Event, vector<Event>, greaterThanByTimestamp> EQ;
public:
    void fillEventQueueWithAdds(vector<IORequest> & IORequests) {
        for (int i = 0; i < IORequests.size(); i++) {
            Event newEvent;
            newEvent.instruction = ADD;
            newEvent.IOReq = &IORequests[i];
            newEvent.timestamp = IORequests[i].start_timestamp;
            EQ.push(newEvent);
        }
    }
    
    long int isEmpty() {
        return EQ.empty();
    }
    
    Event getEvent() {
        Event nextEvent = EQ.top();
        EQ.pop();
        return nextEvent;
    }
    
    void addEvent(Event newEvent) {
        EQ.push(newEvent);
    }
    
    int nextEventTime() {
        return EQ.top().timestamp;
    }

};

class Scheduler {
    IORequest * nextIO(int currentTrack);
    void addIssueandFinishEvent(int time, int currentTrack, EventQueue & events);
    void addReq(IORequest * request);
};

class FIFO : Scheduler {
    vector<IORequest *> toBeIssued;
public:
    void addReq(IORequest * request) {
        toBeIssued.push_back(request);
        return;
    }
        IORequest * nextIO(int currentTrack) {
            IORequest * next = toBeIssued.front();
            toBeIssued.erase(toBeIssued.begin());
            return next;
            
    }
    
    void addIssueandFinishEvent(int time, int currentTrack, EventQueue & events) {
        if (events.isEmpty() && toBeIssued.size() == 0)
            return;
        Event newIssueEvent;
        Event newFinishEvent;
        newIssueEvent.timestamp = time;
        newIssueEvent.instruction = ISSUE;
        newIssueEvent.IOReq = nextIO(currentTrack);
        if (newIssueEvent.IOReq->trackNum - currentTrack > 0)
            newFinishEvent.timestamp = time + newIssueEvent.IOReq->trackNum - currentTrack;
        else // if direction == BACKWARD
            newFinishEvent.timestamp = time + currentTrack - newIssueEvent.IOReq->trackNum;
        newFinishEvent.instruction = FINISH;
        newFinishEvent.IOReq = newIssueEvent.IOReq;
        events.addEvent(newIssueEvent);
        events.addEvent(newFinishEvent);
        return;
    }
    

    
};

class SSTF : FIFO {
    vector<IORequest *> toBeIssued;
public:
    void addReq(IORequest * request) {
        toBeIssued.push_back(request);
        return;
    }
    
    IORequest * nextIO(int currentTrack) {
        int min_dist = 1000000000;
        IORequest * next = NULL;
        int req_ind = -1;
        for (int i = 0; i < toBeIssued.size(); i++) {
            if (abs(currentTrack - toBeIssued[i]->trackNum) < min_dist) {
                min_dist = abs(currentTrack - toBeIssued[i]->trackNum);
                next = toBeIssued[i];
                req_ind = i;
            }
        }
        
        toBeIssued.erase(toBeIssued.begin() + req_ind);
        return next;
    }
    
    void addIssueandFinishEvent(int time, int currentTrack, EventQueue & events) {
        if (events.isEmpty() && toBeIssued.size() == 0)
            return;
        Event newIssueEvent;
        Event newFinishEvent;
        newIssueEvent.timestamp = time;
        newIssueEvent.instruction = ISSUE;
        newIssueEvent.IOReq = nextIO(currentTrack);
        if (newIssueEvent.IOReq->trackNum - currentTrack > 0)
            newFinishEvent.timestamp = time + newIssueEvent.IOReq->trackNum - currentTrack;
        else // if direction == BACKWARD
            newFinishEvent.timestamp = time + currentTrack - newIssueEvent.IOReq->trackNum;
        newFinishEvent.instruction = FINISH;
        newFinishEvent.IOReq = newIssueEvent.IOReq;
        events.addEvent(newIssueEvent);
        events.addEvent(newFinishEvent);
        return;
    }
};

class SCAN : Scheduler {
    vector<IORequest *> toBeIssued;
    int direction = FORWARD;
public:
    void addReq(IORequest * request) {
        toBeIssued.push_back(request);
        return;
    }
    
    IORequest * FORWARDnextIO(int currentTrack) {
        int min_dst = 1000000000;
        IORequest * next = NULL;
        int min_ind = - 1;
        for (int i = 0; i < toBeIssued.size(); i++) {
            if (toBeIssued[i]->trackNum - currentTrack < min_dst && toBeIssued[i]->trackNum > currentTrack) {
                min_dst = toBeIssued[i]->trackNum - currentTrack;
                next = toBeIssued[i];
                min_ind = i;
            }
        }
        
        if (min_ind == -1)
            return BACKWARDnextIO(currentTrack);
        toBeIssued.erase(toBeIssued.begin() + min_ind);
        return next;
        
    }
    
    IORequest * BACKWARDnextIO(int currentTrack) {
        int min_dst = 1000000000;
        IORequest * next = NULL;
        int min_ind = - 1;
        for (int i = 0; i < toBeIssued.size(); i++) {
            if (currentTrack - toBeIssued[i]->trackNum < min_dst && currentTrack > toBeIssued[i]->trackNum) {
                min_dst = currentTrack - toBeIssued[i]->trackNum;
                next = toBeIssued[i];
                min_ind = i;
            }
        }
        
        if (min_ind == -1)
            return FORWARDnextIO(currentTrack);
        toBeIssued.erase(toBeIssued.begin() + min_ind);
        return next;
    }

    IORequest * nextIO(int currentTrack) {
        if (direction == FORWARD) {
            return FORWARDnextIO(currentTrack);
        }
        else { // direction == BACKWARD
            return BACKWARDnextIO(currentTrack);
        }
    }
    
    void addIssueandFinishEvent(int time, int currentTrack, EventQueue & events) {
        if (events.isEmpty() && toBeIssued.size() == 0)
            return;
        Event newIssueEvent;
        Event newFinishEvent;
        newIssueEvent.timestamp = time;
        newIssueEvent.instruction = ISSUE;
        newIssueEvent.IOReq = nextIO(currentTrack);
        if (newIssueEvent.IOReq->trackNum - currentTrack > 0)
            newFinishEvent.timestamp = time + newIssueEvent.IOReq->trackNum - currentTrack;
        else // if direction == BACKWARD
            newFinishEvent.timestamp = time + currentTrack - newIssueEvent.IOReq->trackNum;
        newFinishEvent.instruction = FINISH;
        newFinishEvent.IOReq = newIssueEvent.IOReq;
        events.addEvent(newIssueEvent);
        events.addEvent(newFinishEvent);
        return;
    }
};

class CSCAN : Scheduler {
    vector<IORequest *> toBeIssued;
    int direction = FORWARD;
public:
    void addReq(IORequest * request) {
        toBeIssued.push_back(request);
        return;
    }
    
    IORequest * nextIO(int currentTrack) {
        int min_dst = 1000000000;
        IORequest * next = NULL;
        int min_ind = - 1;
        for (int i = 0; i < toBeIssued.size(); i++) {
            if (toBeIssued[i]->trackNum - currentTrack < min_dst && toBeIssued[i]->trackNum > currentTrack) {
                min_dst = toBeIssued[i]->trackNum - currentTrack;
                next = toBeIssued[i];
                min_ind = i;
            }
        }
        
        if (min_ind == -1)
            return nextIO(0);
        toBeIssued.erase(toBeIssued.begin() + min_ind);
        return next;
    }
    
    void addIssueandFinishEvent(int time, int currentTrack, EventQueue & events) {
        if (events.isEmpty() && toBeIssued.size() == 0)
            return;
        Event newIssueEvent;
        Event newFinishEvent;
        newIssueEvent.timestamp = time;
        newIssueEvent.instruction = ISSUE;
        newIssueEvent.IOReq = nextIO(currentTrack);
        if (newIssueEvent.IOReq->trackNum - currentTrack > 0)
            newFinishEvent.timestamp = time + newIssueEvent.IOReq->trackNum - currentTrack;
        else // if direction == BACKWARD
            newFinishEvent.timestamp = time + currentTrack - newIssueEvent.IOReq->trackNum;
        newFinishEvent.instruction = FINISH;
        newFinishEvent.IOReq = newIssueEvent.IOReq;
        events.addEvent(newIssueEvent);
        events.addEvent(newFinishEvent);
        return;
    }
};


class FSCAN {
    vector<IORequest *> A;
    vector<IORequest *> B;
    bool currentRunningVector = B_; // define A_ 0 & define B_ 1
    bool direction = FORWARD;
public:
    void addReq(IORequest * request) {
        if (currentRunningVector == B_) {
            A.push_back(request);
        }
        else {
            B.push_back(request);

        }
    }


    IORequest * FORWARDnextIO(int currentTrack) {
        
        if (currentRunningVector == A_ && A.size() == 0) {
            currentRunningVector = B_;
        }
        else if (currentRunningVector == B_ && B.size() == 0) {
            currentRunningVector = A_;
        }
        
        int min_dst = 1000000000;
        IORequest * next = NULL;
        int min_ind = - 1;
        
        if (currentRunningVector == A_) {
            for (int i = 0; i < A.size(); i++) {
                if (A[i]->trackNum - currentTrack < min_dst && A[i]->trackNum > currentTrack) {
                    min_dst = A[i]->trackNum - currentTrack;
                    next = A[i];
                    min_ind = i;
                }
            }
        }
        
        else if (currentRunningVector == B_) {
            for (int i = 0; i < B.size(); i++) {
                if (B[i]->trackNum - currentTrack < min_dst && B[i]->trackNum > currentTrack) {
                    min_dst = B[i]->trackNum - currentTrack;
                    next = B[i];
                    min_ind = i;
                }
            }
        }
        
    
        if (min_ind == -1) {
            return BACKWARDnextIO(currentTrack);
        }
        if (currentRunningVector == A_)
            A.erase(A.begin() + min_ind);
        else if (currentRunningVector == B_)
            B.erase(B.begin() + min_ind);

        return next;
    
}

    IORequest * BACKWARDnextIO(int currentTrack) {
        if (currentRunningVector == A_ && A.size() == 0) {
            currentRunningVector = B_;
        }
        else if (currentRunningVector == B_ && B.size() == 0) {
            currentRunningVector = A_;
        }
        
        int min_dst = 1000000000;
        IORequest * next = NULL;
        int min_ind = - 1;
        
        
        if (currentRunningVector == A_) {
            for (int i = 0; i < A.size(); i++) {
                if (currentTrack - A[i]->trackNum  < min_dst && A[i]->trackNum < currentTrack) {
                    min_dst = currentTrack - A[i]->trackNum;
                    next = A[i];
                    min_ind = i;
                }
            }
        }
        
        if (currentRunningVector == B_) {
            for (int i = 0; i < B.size(); i++) {
                if (currentTrack - B[i]->trackNum  < min_dst && B[i]->trackNum < currentTrack) {
                    min_dst = currentTrack - B[i]->trackNum;
                    next = B[i];
                    min_ind = i;
                }
            }
        }
        
        
    
        if (min_ind == -1)
            return FORWARDnextIO(currentTrack);
        
        if (currentRunningVector == A_)
            A.erase(A.begin() + min_ind);
        else if (currentRunningVector == B_)
            B.erase(B.begin() + min_ind);
        
        return next;
    }

    IORequest * nextIO(int currentTrack) {
        static int track = 0;
        if (track == 0)
            currentRunningVector = A_;
        track = 1;
        if (direction == FORWARD) {
            return FORWARDnextIO(currentTrack);
        }
        else { // direction == BACKWARD
            return BACKWARDnextIO(currentTrack);
        }
    }

    void addIssueandFinishEvent(int time, int currentTrack, EventQueue & events) {
        if (events.isEmpty() && A.size() == 0 && B.size() == 0)
            return;
        Event newIssueEvent;
        Event newFinishEvent;
        newIssueEvent.timestamp = time;
        newIssueEvent.instruction = ISSUE;
        newIssueEvent.IOReq = nextIO(currentTrack);
        if (newIssueEvent.IOReq->trackNum - currentTrack > 0)
            newFinishEvent.timestamp = time + newIssueEvent.IOReq->trackNum - currentTrack;
        else // if direction == BACKWARD
            newFinishEvent.timestamp = time + currentTrack - newIssueEvent.IOReq->trackNum;
        newFinishEvent.instruction = FINISH;
        newFinishEvent.IOReq = newIssueEvent.IOReq;
        events.addEvent(newIssueEvent);
        events.addEvent(newFinishEvent);
        return;
    }
};



class Summary {
public:
    int total_time = 0;
    int total_movement = 0;
    double total_turnaround = 0;
    double avg_turnaround = 0;
    double total_waittime = 0;
    double avg_waittime = 0;
    int max_waittime = 0;
    double total_num_of_requests = 0;
    
    void printSummary() {
        total_num_of_requests++;
        avg_turnaround = total_turnaround / total_num_of_requests;
        avg_waittime = total_waittime / total_num_of_requests;
        printf("SUM: %d %d %.2lf %.2lf %d\n",
               total_time,
               total_movement,
               avg_turnaround,
               avg_waittime,
               max_waittime);
    }
    
};

template <class T>
void simulation(vector<IORequest> & IORequests, T scheduler) {
    EventQueue events;
    Summary summary;
    int max_wait_time = 0;
    events.fillEventQueueWithAdds(IORequests);
    int time = 0;
    int currentTrack = 0;
    bool reqCurrentlyIssued = false;
    while (!events.isEmpty()) {
        if (time == events.nextEventTime()) {
            Event currentEvent = events.getEvent();
            if (currentEvent.instruction == ADD) {
                cout << time << ":\t" << currentEvent.IOReq->reqNum << " add " << currentEvent.IOReq->trackNum << endl;
                scheduler.addReq(currentEvent.IOReq);
            }
            else if (currentEvent.instruction == ISSUE) {
                cout << time << ":\t" << currentEvent.IOReq->reqNum << " issue " << currentEvent.IOReq->trackNum << " " << currentTrack << endl;
                summary.total_waittime += time - currentEvent.IOReq->start_timestamp;
                if (time - currentEvent.IOReq->start_timestamp > max_wait_time) {
                    summary.max_waittime = time - currentEvent.IOReq->start_timestamp;
                    max_wait_time = summary.max_waittime;
                }
                if (currentTrack < currentEvent.IOReq->trackNum)
                    summary.total_movement += currentEvent.IOReq->trackNum -  currentTrack;
                else
                    summary.total_movement += currentTrack - currentEvent.IOReq->trackNum;
                reqCurrentlyIssued = true;
                currentTrack = currentEvent.IOReq->trackNum;
            }
            else if (currentEvent.instruction == FINISH) {
                cout << time << ":\t" << currentEvent.IOReq->reqNum << " finish " << time - currentEvent.IOReq->start_timestamp << endl;
                summary.total_turnaround += time - currentEvent.IOReq->start_timestamp;
                summary.total_num_of_requests = currentEvent.IOReq->reqNum;
                reqCurrentlyIssued = false;
            }
        
            if (reqCurrentlyIssued == false) {
                scheduler.addIssueandFinishEvent(time, currentTrack, events);
            }
        }
        if (events.nextEventTime() != time) {
            time++;
        }
    }
    
    summary.total_time = time;
    summary.printSummary();
}

int main() {
    string filename = "/Users/Lisa/Desktop/TestData/input0";
    vector<IORequest> IORequests;
    FSCAN scheduler;
    readInstructions(filename, IORequests);
    simulation(IORequests, scheduler);
}
