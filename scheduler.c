#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_PROCESS_COUNT 100
#define MAX_INTERVALS 20

typedef struct ProcessItem {
    int pid;
    int arrival;
    int burst;
    int priority;
    int intervals;
    int firstRun;
    int waitTime[MAX_INTERVALS][2];
} ProcessItem;

void printProcesses(ProcessItem **processesArr, const int *processesCount) {
    printf("Processes sorted by priority:\n");
    for (int i = 0; i < *processesCount; i++) {
        printf("PID: %d\tArrival: %d\tBurst: %d\tPriority: %d\n",
               processesArr[i]->pid,
               processesArr[i]->arrival,
               processesArr[i]->burst,
               processesArr[i]->priority);
    }
    printf("\n");
}

int comparePriority(const void *a, const void *b) {
    const ProcessItem *processA = (const ProcessItem *) a;
    const ProcessItem *processB = (const ProcessItem *) b;

    if (processA->priority < processB->priority)
        return -1;
    else if (processA->priority > processB->priority)
        return 1;
    else {
        if (processA->arrival < processB->arrival)
            return -1;
        else if (processA->arrival > processB->arrival)
            return 1;
        else
            return 0;
    }
}

void freeProcessesArr(ProcessItem **processesArr, const int *processesCount) {
    for (int i = 0; i < *processesCount; i++) {
        free(processesArr[i]);
    }
    free(processesArr);
}

int noProcessesLeft(ProcessItem **processesArr, int processesCount) {
    for (int i = 0; i < processesCount; i++) {
        if (processesArr[i]->burst > 0)
            return 0;
    }
    return 1;
}

int noSpecificProcessesLeft(ProcessItem **processesArr, int processesCount, int currentPriority) {
    for (int i = 0; i < processesCount; i++) {
        if (processesArr[i]->priority == currentPriority && processesArr[i]->burst > 0)
            return 0;
    }
    return 1;
}

ProcessItem *
nextProcess(ProcessItem **processesArr, int lastProcessIndex, int processesCount, const int *currentPriority) {
    // Searches for the next item in the array with the same priority that still has a burst > 0
    for (int i = lastProcessIndex; i < processesCount; i++) {
        if (processesArr[i]->priority == *currentPriority && i != lastProcessIndex && processesArr[i]->burst > 0)
            return processesArr[i];
    }

    // Starts from the beginning of the array to search for an item that still has a burst > 0
    for (int i = 0; i < processesCount; i++) {
        if (processesArr[i]->priority == *currentPriority && i != lastProcessIndex && processesArr[i]->burst > 0)
            return processesArr[i];
    }

    // Returns the last processed item if its burst still is > 0
    if (processesArr[lastProcessIndex]->burst > 0)
        return processesArr[lastProcessIndex];
    else
        return NULL;
}

ProcessItem *
checkHigherPriorityProcess(ProcessItem **processesArr, const int lastProcessIndex, const int processesCount,
                           const int currentQuantum) {
    ProcessItem *process = processesArr[lastProcessIndex];
    for (int i = 0; i < processesCount; i++) {
        if (processesArr[i]->arrival <= currentQuantum && processesArr[i]->priority < process->priority &&
            processesArr[i]->burst > 0) {
            return processesArr[i];
        }
    }
    return NULL;
}

int getProcessIndex(ProcessItem **processesArr, ProcessItem *process, int processesCount) {
    for (int i = 0; i < processesCount; i++) {
        if (processesArr[i]->pid == process->pid)
            return i;
    }
    // If by any chance -1 gets returned, something really unexpected happened and I think the world is about to end
    return -1;
}

void setProcessWaitTime(ProcessItem *process) {
    process->waitTime[0][0] = -1;
    process->waitTime[0][1] = process->arrival;
    for (int i = 1; i < MAX_INTERVALS; i++) {
        for (int j = 0; j < 2; j++)
            process->waitTime[i][j] = -1;
    }
}

void printAllWaitTimes(ProcessItem **processArr, const int processesCount) {
    for (int i = 0; i < processesCount; i++) {
        printf("PID: %d | Burst: %d | Priority: %d\n",
               processArr[i]->pid, processArr[i]->burst, processArr[i]->priority);
        for (int j = 0; j < MAX_INTERVALS / 2; j++) {
//            if (processArr[i]->waitTime[j][0] == -1 && processArr[i]->waitTime[j][1] == -1) {
//                printf("{%d, %d} ",
//                       processArr[i]->waitTime[j][0],
//                       processArr[i]->waitTime[j][1]);
//                break;
//            }
            printf("{%d, %d} ",
                   processArr[i]->waitTime[j][0],
                   processArr[i]->waitTime[j][1]);
        }
        printf("\n");
    }
}

double calculateAverageWaitTime(ProcessItem **processesArr, const int processesCount) {
    double total = 0;
    for (int i = 0; i < processesCount; i++) {
        double tempResult = 0;
        for (int j = 0; j < MAX_INTERVALS; j++) {
            // Breaks when no stop marks were signed, meaning the processes finished during that run
            if (processesArr[i]->waitTime[j][1] == - 1)
                break;
            tempResult += processesArr[i]->waitTime[j][0] - processesArr[i]->waitTime[j][1];
        }
        total += tempResult;
    }
    return (double) (total / processesCount);
}

void printAverageWaitTime(ProcessItem **processesArray, const int processesCount) {
    double awt = calculateAverageWaitTime(processesArray, processesCount);
    printf("The average wait time was: %.2lf quantum", awt);
}

// Interpreter
ProcessItem **getProcesses(int *processesCount) {
    ProcessItem **processesArr = (ProcessItem **) malloc(sizeof(ProcessItem *) * MAX_PROCESS_COUNT);
    if (processesArr == NULL) {
        printf("Error while allocating memory for the array of processes.\n");
        return NULL;
    }

    char line[100];
    FILE *file = fopen("processes.txt", "r");
    if (file == NULL) {
        printf("Error while opening the file\n!");
        free(processesArr);
        return NULL;
    }

    // Reads the first line and discards it, since it only contains info about the data sorting
    fgets(line, sizeof(line), file);

    while (fgets(line, sizeof(line), file) != NULL) {
        int pid, arrival, burst, priority;
        ProcessItem *newProcess = (ProcessItem *) malloc(sizeof(ProcessItem));
        if (newProcess == NULL) {
            printf("Not enough memory to allocate a new process!\nAmount of processes already allocated: %d\n",
                   *processesCount);
            fclose(file);
            freeProcessesArr(processesArr, processesCount);
            return NULL;
        }
        if (sscanf(line, "%d,%d,%d,%d", &pid, &arrival, &burst, &priority) != 4) {
            printf("Error found while converting the values.\nPID: %d\nArrival: %d\nBurst: %d\nPriority: %d\n",
                   pid, arrival, burst, priority);
            fclose(file);
            freeProcessesArr(processesArr, processesCount);
            return NULL;
        }
        newProcess->pid = pid;
        newProcess->arrival = arrival;
        newProcess->burst = burst;
        newProcess->priority = priority;
        newProcess->intervals = 0;
        newProcess->firstRun = 1;
        setProcessWaitTime(newProcess);

        processesArr[*processesCount] = newProcess;
        (*processesCount)++;
    }
    qsort(processesArr, *processesCount, sizeof(ProcessItem *), comparePriority);
    fclose(file);

    return processesArr;
}

// Interpreter command line
ProcessItem **getProcessesCommandLine(int *processesCount, char *fileLocation) {
    ProcessItem **processesArr = (ProcessItem **) malloc(sizeof(ProcessItem *) * MAX_PROCESS_COUNT);
    if (processesArr == NULL) {
        printf("Error while allocating memory for the array of processes.\n");
        return NULL;
    }

    char line[100];
    FILE *file = fopen("processes.txt", "r");
    if (file == NULL) {
        printf("Error while opening the file\n!");
        free(processesArr);
        return NULL;
    }

    // Reads the first line and discards it, since it only contains info about the data sorting
    fgets(line, sizeof(line), file);

    while (fgets(line, sizeof(line), file) != NULL) {
        int pid, arrival, burst, priority;
        ProcessItem *newProcess = (ProcessItem *) malloc(sizeof(ProcessItem));
        if (newProcess == NULL) {
            printf("Not enough memory to allocate a new process!\nAmount of processes already allocated: %d\n",
                   *processesCount);
            fclose(file);
            freeProcessesArr(processesArr, processesCount);
            return NULL;
        }
        if (sscanf(line, "%d,%d,%d,%d", &pid, &arrival, &burst, &priority) != 4) {
            printf("Error found while converting the values.\nPID: %d\nArrival: %d\nBurst: %d\nPriority: %d\n",
                   pid, arrival, burst, priority);
            fclose(file);
            freeProcessesArr(processesArr, processesCount);
            return NULL;
        }
        newProcess->pid = pid;
        newProcess->arrival = arrival;
        newProcess->burst = burst;
        newProcess->priority = priority;
        newProcess->intervals = 0;
        newProcess->firstRun = 1;
        setProcessWaitTime(newProcess);

        processesArr[*processesCount] = newProcess;
        (*processesCount)++;
    }
    qsort(processesArr, *processesCount, sizeof(ProcessItem *), comparePriority);
    fclose(file);

    return processesArr;
}

// Scheduler
void scheduleProcesses(ProcessItem **processesArr, int processesCount) {
    int maxQuantum = 4;
    int totalQuantum = 0;
    int elapsedQuantum = 0;
    int currentPriority = 1;
    int currentIndex = 0;
    int finishedProcesses = 0;
    ProcessItem *currentProcess = processesArr[0];

    // Finds the process with the lowest arrival
    for (int i = 0; i < processesCount; i++) {
        if (processesArr[i]->arrival < currentProcess->arrival) {
            currentProcess = processesArr[i];
            currentIndex = i;
        }
    }

    printf("\n********************************\n");
    // Runs while there are processes with burst > 0 left in processesArr
    while (!noProcessesLeft(processesArr, processesCount)) {
        currentProcess->waitTime[currentProcess->intervals][0] = totalQuantum;

        // Runs while elapsedQuantum != maxQuantum and currentProcess->burst > 0
        while (elapsedQuantum < maxQuantum && currentProcess->burst > 0) {
            currentProcess->burst--;
            elapsedQuantum++;
            totalQuantum++;
            printf("Total Quantum: %d\tPID: %d\tBurst: %d\tPriority: %d\n",
                   totalQuantum, currentProcess->pid, currentProcess->burst, currentProcess->priority);
            ProcessItem *temp = checkHigherPriorityProcess(processesArr, currentIndex, processesCount, totalQuantum);
            if (temp != NULL)
                break;
        }

        // Runs if the process was finished
        if (currentProcess->burst <= 0) {
            printf("PID %d finished processing!\n\n", currentProcess->pid);
            finishedProcesses++;
            if (finishedProcesses == processesCount)
                return;
        }

        // **********************************************************************************************************
        // Runs if there are NO processes with the same priority left.
        if (noSpecificProcessesLeft(processesArr, processesCount, currentPriority)) {
            currentPriority++;
            ProcessItem *temp = currentProcess;
            currentProcess = nextProcess(processesArr, currentIndex, processesCount, &currentPriority);
            if (temp == currentProcess)
                break;
            printf("Process with lower priority being processed!\n");
        }

        // **********************************************************************************************************
        // Runs if there ARE processes with the same priority left
        else {
            ProcessItem *tempProcess = currentProcess;

            // Gets the next process
            currentProcess = nextProcess(processesArr, currentIndex, processesCount, &currentPriority);
            currentIndex = getProcessIndex(processesArr, currentProcess, processesCount);
            elapsedQuantum = 0; // Resets the elapsed quantum so the next process can run successfully

            // Runs if it finds a different process with the same priority and burst > 0
            if (currentProcess->pid != tempProcess->pid) {
                // Runs if it is the first time the process ran
                if (tempProcess->firstRun == 1) {
                    tempProcess->firstRun = 0;
                    tempProcess->intervals++;
                    tempProcess->waitTime[tempProcess->intervals][1] = totalQuantum;
                }
                // Runs if it wasn't the first time the process ran
                else {
                    tempProcess->waitTime[tempProcess->intervals][1] = totalQuantum;
                    tempProcess->intervals++;
                }
                printf("\nNew process being processed!\n");
            }
            // Runs if it does not find a different process with the same priority and burst > 0
            else {
                if (tempProcess->firstRun == 1) {
                    tempProcess->firstRun = 0;
                    tempProcess->intervals++;
                }
                else {
                    tempProcess->intervals++;
                }
            }
        }
//       sleep(1);
    }
}

int main() {
    // TODO Add a way to load the file path from the main() args
    int processesCount = 0;

    // Run the interpreter
    ProcessItem **processesArr = getProcesses(&processesCount);

    if (processesArr == NULL) {
        printf("Error while allocating memory for the array of processes.\n");
        return EXIT_FAILURE;
    }

    // Prints all the processes inside the array
    printProcesses(processesArr, &processesCount);
//    getchar();

    // Simulates a scheduler
    scheduleProcesses(processesArr, processesCount);

    // Prints all the processes inside the array again to show their bursts as 0
    printProcesses(processesArr, &processesCount);
//    getchar();

    printAllWaitTimes(processesArr, processesCount);
//    getchar();

    printAverageWaitTime(processesArr, processesCount);
//    getchar();

    freeProcessesArr(processesArr, &processesCount);

    return EXIT_SUCCESS;
}
