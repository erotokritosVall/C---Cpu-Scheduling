#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

/**
Program that simulates a CPU Scheduler. Uses Shortest Job First and Priority First scheduling methods, on both preemptive and non-preemptive mode
*/

typedef enum
{
	sjf,
	priority
}AlgorithmToCall;

typedef struct Process
{
	int procID;
	int arrivalTime;
	int serviceTime;
	int priority;
	int waitTime;
	int finishTime;
	int turnAroundTime;
	int startTime;
	int remainingTime;
	struct Process* nextProc;
}Process;

//Process methods :
Process* CreateProc();
void SwapContents(Process* proc1, Process* proc2);

typedef struct Queue
{
	Process* ptrHead;
	Process* ptrTail;
}Queue;

//Queue methods :
Queue* CreateQueue();
void SortQueueByArrival(Queue* const queueToSort, AlgorithmToCall currentState);
void AddToQueue(Queue* const queueToAdd, Process* const item);
void FreeQueue(Queue* const queueToFree);
bool QueueIsEmpty(const Queue* const queueToCheck);


//Input / Output methods :
int ReadFileData(Queue* const dataHolder, FILE* const fileToRead);
void PrintValues(Queue* const outputQueue, int total);

//Simulation methods :
void UpdateEntryQueue(Queue* const inputQueue, Queue* const simulationQueue, int currentTime);
void Tick(Queue* const simulationQueue, Queue* const outputQueue, int currentTime, bool bIsPreemptive, void(*functionToCall)(Queue* const));
void FindNextSJF(Queue* const simulationQueue);
void FindNextPriority(Queue* const simulationQueue);

int main(int argc, char *argv[])
{
	AlgorithmToCall whichOne;
	bool bisPreemptive;
	FILE* ptrProcsFile;

	if (argc != 4)
	{
		printf("Error! Invalid number of arguments!");
		exit(0);
	}

	if (strcmp(argv[1], "sjf") == 0 || strcmp(argv[1], "SJF") == 0)
		whichOne = sjf;
	else if (strcmp(argv[1], "priority") == 0 || strcmp(argv[1], "PRIORITY") == 0)
		whichOne = priority;
	else
	{
		printf("Error! Invalid algorithm call!");
		exit(1);
	}

	if (strcmp(argv[2], "p") == 0 || strcmp(argv[2], "P") == 0)
		bisPreemptive = true;
	else if (strcmp(argv[2], "np") == 0 || strcmp(argv[2], "NP") == 0)
		bisPreemptive = false;
	else
	{
		printf("Error! Invalid pre-emptiveness");
		exit(2);
	}

	ptrProcsFile = fopen(argv[3], "r");
	if (ptrProcsFile == NULL)
	{
		printf("Error! File %s could not be opened!", argv[3]);
		exit(3);
	}

	Queue* entryQueue = CreateQueue();

	int totalProcs = ReadFileData(entryQueue, ptrProcsFile);
	fclose(ptrProcsFile);

	SortQueueByArrival(entryQueue, whichOne);

	if (whichOne == sjf)
	{
		Queue* simulationQueue = CreateQueue();
		Queue* outputQueue = CreateQueue();
		int currentTime = 0;
		UpdateEntryQueue(entryQueue, simulationQueue, currentTime);
		printf("\n\n \t\t\t\t ---/ Gant Diagram \\---\n\n");
		while (!QueueIsEmpty(entryQueue) || !QueueIsEmpty(simulationQueue))
		{
			currentTime++;
			UpdateEntryQueue(entryQueue, simulationQueue, currentTime);
			Tick(simulationQueue, outputQueue, currentTime, bisPreemptive, &FindNextSJF);
		}
		printf("\n\n");
		free(entryQueue);
		free(simulationQueue);
		printf("\n\t\t\t\t---/ Shortest Job First Output \\---\n\n");
		PrintValues(outputQueue, totalProcs);
		FreeQueue(outputQueue);
		free(outputQueue);
	}
	else
	{
		Queue* simulationQueue = CreateQueue();
		Queue* outputQueue = CreateQueue();
		int currentTime = 0;
		UpdateEntryQueue(entryQueue, simulationQueue, currentTime);
		printf("\n\n \t\t\t\t ---/ Gant Diagram \\---\n\n");
		while (!QueueIsEmpty(entryQueue) || !QueueIsEmpty(simulationQueue))
		{
			currentTime++;
			UpdateEntryQueue(entryQueue, simulationQueue, currentTime);
			Tick(simulationQueue, outputQueue, currentTime, bisPreemptive, &FindNextPriority);
		}
		printf("\n\n");
		free(entryQueue);
		free(simulationQueue);
		printf("\n\t\t\t\t---/ Priority Scheduling Output \\--- \n\n");
		PrintValues(outputQueue, totalProcs);
		FreeQueue(outputQueue);
		free(outputQueue);
	}
	return 0;
}

//Creates a process item dynamically , checks if the creation was successfull and returns the address of the item
Process* CreateProc()
{
	Process* temp = (Process*)malloc(sizeof(Process));
	if (temp == NULL)
	{
		printf("Error! Malloc failed at procedure creation!");
		exit(5);
	}
	temp->nextProc = NULL;
	return temp;
}

//Method to swap two processe's contents by value
void SwapContents(Process* proc1, Process* proc2)
{
	Process temp = *proc1;

	proc1->arrivalTime = proc2->arrivalTime;
	proc1->priority = proc2->priority;
	proc1->serviceTime = proc2->serviceTime;
	proc1->procID = proc2->procID;
	proc1->startTime = proc2->startTime;
	proc1->waitTime = proc2->waitTime;
	proc1->remainingTime = proc2->remainingTime;

	proc2->arrivalTime = temp.arrivalTime;
	proc2->priority = temp.priority;
	proc2->serviceTime = temp.serviceTime;
	proc2->procID = temp.procID;
	proc2->startTime = temp.startTime;
	proc2->waitTime = temp.waitTime;
	proc2->remainingTime = temp.remainingTime;
}

//Creates a Queue item dynamically , checks if the creation was successfull and returns the address of the item
Queue* CreateQueue()
{
	Queue* temp = (Queue*)malloc(sizeof(Queue));
	if (temp == NULL)
	{
		printf("Error! Malloc failed at queue creation");
		exit(4);
	}
	temp->ptrHead = temp->ptrTail = NULL;
	return temp;
}

//Method to sort a queue by arrival time , uses selection sort algorithm
void SortQueueByArrival(Queue* const queueToSort, AlgorithmToCall currentState)
{
		for (Process* firstIterator = queueToSort->ptrHead; firstIterator != queueToSort->ptrTail; firstIterator = firstIterator->nextProc)
		{
			for (Process* secondIterator = firstIterator->nextProc; secondIterator != NULL; secondIterator = secondIterator->nextProc)
			{
				if (firstIterator->arrivalTime > secondIterator->arrivalTime)
				{
					SwapContents(firstIterator, secondIterator);
				}
				else if (firstIterator->arrivalTime == secondIterator->arrivalTime)
				{
					if (currentState == sjf)
					{
						if (firstIterator->serviceTime > secondIterator->serviceTime)
							SwapContents(firstIterator, secondIterator);
					}
					else
					{
						if (firstIterator->priority < secondIterator->priority)
							SwapContents(firstIterator, secondIterator);
					}
				}
			}
		}
}

//Adds a Process item in a queue , gets the item and the queue that the item should be added as a parameter
void AddToQueue(Queue* const queueToAdd, Process* const item)
{
	if (QueueIsEmpty(queueToAdd))
	{
		queueToAdd->ptrHead = queueToAdd->ptrTail = item;
		item->nextProc = NULL;
	}
	else
	{
		queueToAdd->ptrTail->nextProc = item;
		queueToAdd->ptrTail = item;
		item->nextProc = NULL;
	}
}

//Method to free allocated memory after simulation is complete
void FreeQueue(Queue* const queueToFree)
{
	Process* iterator = queueToFree->ptrHead;
	Process* deleter;
	while (iterator != NULL)
	{
		deleter = iterator;
		iterator = iterator->nextProc;
		free(deleter);
	}
}

//Method to check if a queue is empty , takes as a parameter the queue we want to check and returns if the head is null
bool QueueIsEmpty(const Queue* const queueToCheck)
{
	return (queueToCheck->ptrHead == NULL);
}

//Method to read and store file's data into queue , takes the queue and the file to be read as parameters , returns how many processe's were added
int ReadFileData(Queue* const dataHolder, FILE* const fileToRead)
{
	int arrivalTime, serviceTime, priority;
	int numOfProcs = 0;
	while (fscanf(fileToRead, "%d %d %d", &arrivalTime, &serviceTime, &priority) == 3)
	{
		Process* temp = CreateProc();
		temp->arrivalTime = arrivalTime;
		temp->serviceTime = serviceTime;
		temp->priority = priority;
		temp->procID = numOfProcs;
		temp->waitTime = 0;
		temp->startTime = 0;
		temp->remainingTime = temp->serviceTime;
		AddToQueue(dataHolder, temp);
		numOfProcs++;
	}
	return numOfProcs;
}

//Method to print the data after the simulation is complete , gets the outputQueue and the total number of processe's as parameters
void PrintValues(Queue* const outputQueue, int total)
{
	int serviceTimeSum = 0;
	int totalWaitTime = 0;
	int totalTurnaroundTime = 0;
	float averageServiceTime, averageWaitTime, averageTurnaroundTime;
	printf("ProcID\tArrivalTime\tServiceTime\tPriority\tWaitTime\tStartTime\tFinishTime\tTurnaroundTime\n");
	for (Process* iterator = outputQueue->ptrHead; iterator != NULL; iterator = iterator->nextProc)
	{
		serviceTimeSum += iterator->serviceTime;
		totalWaitTime += iterator->waitTime;
		totalTurnaroundTime += iterator->turnAroundTime;
		printf("%d    \t%d    \t        %d    \t        %d    \t        %d         \t%d    \t        %d    \t        %d\n", iterator->procID, iterator->arrivalTime, iterator->serviceTime,
			iterator->priority, iterator->waitTime, iterator->startTime, iterator->finishTime, iterator->turnAroundTime);
	}
	averageServiceTime = (float)serviceTimeSum / total;
	averageWaitTime = (float)totalWaitTime / total;
	averageTurnaroundTime = (float)totalTurnaroundTime / total;
	printf("\nAverage Service Time: %f \n", averageServiceTime);
	printf("Average Wait Time: %f \n", averageWaitTime);
	printf("Average Turnaround Time: %f \n", averageTurnaroundTime);
}

//Method to update the entry queue , takes the entry and simulations queue's as parameters as well as the current simulation time
void UpdateEntryQueue(Queue* const inputQueue, Queue* const simulationQueue, int currentTime)
{
    while (!QueueIsEmpty(inputQueue))
    {
	  if (inputQueue->ptrHead->arrivalTime == currentTime)
      {
          Process* toRemove = inputQueue->ptrHead;
			inputQueue->ptrHead = inputQueue->ptrHead->nextProc;
			AddToQueue(simulationQueue, toRemove);
      }
      else
        break;
    }
}

//Method to update the simulation on each clock tick , takes the sim queue , output queue , current simulation time , bool to check if the sim is preemptive and a pointer-to-function
//(for priority or sjf process swaping)
void Tick(Queue* const simulationQueue, Queue* const outputQueue, int currentTime, bool bIsPreemptive, void(*functionToCall)(Queue* const))
{
	if (!QueueIsEmpty(simulationQueue))
	{
	    printf("P%d-> ", simulationQueue->ptrHead->procID);
		Process* head = simulationQueue->ptrHead;
		head->remainingTime--;
		if (head->remainingTime == 0)
		{
			head->finishTime = currentTime;
			head->turnAroundTime = head->finishTime - head->arrivalTime;
			simulationQueue->ptrHead = simulationQueue->ptrHead->nextProc;
			AddToQueue(outputQueue, head);
			if (!QueueIsEmpty(simulationQueue))
			{
				(*functionToCall)(simulationQueue);
				if (simulationQueue->ptrHead->remainingTime == simulationQueue->ptrHead->serviceTime)
					simulationQueue->ptrHead->startTime = currentTime;
			}
			else
				{
			     printf("P(NULL) - ->");
			     return;
				}
		}
		else
			if (bIsPreemptive)
			{
				(*functionToCall)(simulationQueue);
				if (simulationQueue->ptrHead->remainingTime == simulationQueue->ptrHead->serviceTime)
					simulationQueue->ptrHead->startTime = currentTime;
			}
		for (Process* iterator = simulationQueue->ptrHead->nextProc; iterator != NULL; iterator = iterator->nextProc)
			iterator->waitTime++;
	}
}

//Method to find the next process that should be ran with Shortest Job First sorting , takes the simulation queue as a parameter
void FindNextSJF(Queue* const simulationQueue)
{
	Process* min = simulationQueue->ptrHead;
	for (Process* iterator = min->nextProc; iterator != NULL; iterator = iterator->nextProc)
		if (min->remainingTime > iterator->remainingTime)
			min = iterator;
	if (min == simulationQueue->ptrHead)
		return;
	else
		SwapContents(min, simulationQueue->ptrHead);
}

//Method to find the next process that should be ran with priority sorting , takes the simulation queue as a parameter
void FindNextPriority(Queue* const simulationQueue)
{
	Process* high = simulationQueue->ptrHead;
	for (Process* iterator = high->nextProc; iterator != NULL; iterator = iterator->nextProc)
		if (high->priority < iterator->priority)
			high = iterator;
	if (high == simulationQueue->ptrHead)
		return;
	else
		SwapContents(high, simulationQueue->ptrHead);
}
