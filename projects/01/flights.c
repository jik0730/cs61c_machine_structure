/* 
 * CS61C Summer 2016
 * Name: Ingyo Chung
 * Login: bn
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "flights.h"
#include "timeHM.h"

typedef struct flight flight_t;

struct flight {

  airport_t* src;
  airport_t* dst;
  timeHM_t* departure;
  timeHM_t* arrival;
  int cost;
  flight_t* next;

};

struct flightSys {

  airport_t* airport;
  flightSys_t* next;

};

struct airport {

  char* name;
  flight_t* flights;

};

/*
   This should be called if memory allocation failed.
 */
static void allocation_failed() {
    fprintf(stderr, "Out of memory.\n");
    exit(EXIT_FAILURE);
}


/*
   Creates and initializes a flight system, which stores the flight schedules of several airports.
   Returns a pointer to the system created.
 */
flightSys_t* createSystem() {
  flightSys_t* flightSys = (flightSys_t*) malloc(sizeof(flightSys_t)); // Create single flight system.

  if(flightSys == NULL) { // In case of fail of memory allocation.
    allocation_failed();
  }

  return flightSys;
}


/*
   Frees all memory associated with this system; that's all memory you dynamically allocated in your code.
 */
void deleteSystem(flightSys_t* s) {
  // Recursively delete system.
  if(s == NULL) { // Base case of recursive function.
    return ;
  }

  // Do I have to free memory of all stuffs like airports or flights?
  deleteSystem(s->next); // Move into as deeply as possible to prevent memory leaks.
  free(s); // Free.

  return ;
}


/*
   Adds a airport with the given name to the system. You must copy the string and store it.
   Do not store "name" (the pointer) as the contents it point to may change.
 */
void addAirport(flightSys_t* s, char* name) {
  airport_t* airport = (airport_t*) malloc(sizeof(airport_t)); // Allocate memory to airport.
  char* a_name = (char*) malloc(sizeof(name)); // Allocate memory to a_name with the same size of a memory we received.
  airport->name = a_name;

  if(a_name == NULL) { // In case of fail of memory allocation.
    allocation_failed();
  }

  strcpy(a_name, name); // String copy function to a_name from name.

  flightSys_t* temp = s;
  while(temp->next) {
    temp = temp->next;
  } // Get to the last element in flight system.

  temp->next = (flightSys_t*) malloc(sizeof(flightSys_t));
  temp->next->airport = airport;
  return ;
}


/*
   Returns a pointer to the airport with the given name.
   If the airport doesn't exist, return NULL.
 */
airport_t* getAirport(flightSys_t* s, char* name) {
  flightSys_t* temp = s;

  while(strcmp(temp->airport->name, name)) { // Iterate through flight system until finding an airport with same name.
    temp = temp->next;
    if(temp == NULL) {
      return NULL;
    }
  }

  return temp->airport;
}


/*
   Print each airport name in the order they were added through addAirport, one on each line.
   Make sure to end with a new line. You should compare your output with the correct output
   in flights.out to make sure your formatting is correct.
 */
void printAirports(flightSys_t* s) {
  flightSys_t* temp = s;

  while(temp) {
    printf("%s\n", temp->airport->name);
    temp = temp->next;
  }
}


/*
   Adds a flight to src's schedule, stating a flight will leave to dst at departure time and arrive at arrival time.
 */
void addFlight(airport_t* src, airport_t* dst, timeHM_t* departure, timeHM_t* arrival, int cost) {
  flight_t* temp = src->flights;

  while(temp->next) {
    temp = temp->next;
  }

  temp->next = (flight_t*) malloc(sizeof(flight_t));
  temp->next->src = src;
  temp->next->dst = dst;
  temp->next->departure = departure;
  temp->next->arrival = arrival;
  temp->next->cost = cost;

  return ;
}


/*
   Prints the schedule of flights of the given airport.

   Prints the airport name on the first line, then prints a schedule entry on each 
   line that follows, with the format: "destination_name departure_time arrival_time $cost_of_flight".

   You should use printTime (look in timeHM.h) to print times, and the order should be the same as 
   the order they were added in through addFlight. Make sure to end with a new line.
   You should compare your output with the correct output in flights.out to make sure your formatting is correct.
 */
void printSchedule(airport_t* s) {
  printf("%s\n", s->name);

  flight_t* temp = s->flights;
  while(temp) {
    printf("%s ", temp->dst->name);
    printTime(temp->departure);
    printf(" ");
    printTime(temp->arrival);
    printf("%d", temp->cost);
    temp = temp->next;
  }

  return ;
}


/*
   Given a src and dst airport, and the time now, finds the next flight to take based on the following rules:
   1) Finds the cheapest flight from src to dst that departs after now.
   2) If there are multiple cheapest flights, take the one that arrives the earliest.

   If a flight is found, you should store the flight's departure time, arrival time, and cost in departure, arrival, 
   and cost params and return true. Otherwise, return false. 

   Please use the function isAfter() from time.h when comparing two timeHM_t objects.
 */
bool getNextFlight(airport_t* src, airport_t* dst, timeHM_t* now, timeHM_t* departure, timeHM_t* arrival, int* cost) {
  flight_t* temp = src->flights;
  if(temp == NULL) {
    return false;
  }

  airport_t* toCompare = (airport_t*) malloc(sizeof(airport_t));
  if(toCompare == NULL) {
    allocation_failed();
  }

  int costcomp = INT_MAX;
  while(temp) {
    if(temp->cost < costcomp && isAfter(temp->departure, now)) {
      costcomp = temp->cost;
    }
    temp = temp->next;
  }

  if(costcomp == INT_MAX) {
    return false;
  }

  temp = src->flights;
  while(temp) {
    if(temp->cost == costcomp) {
      addFlight(toCompare, temp->dst, temp->departure, temp->arrival, temp->cost);
    }
    temp = temp->next;
  }

  temp = toCompare->flights;
  flight_t* pointer = temp;
  while(temp) {
    if(isAfter(pointer->departure, temp->departure)) {
      pointer = temp;
    }
    temp = temp->next;
  }

  *departure = *pointer->departure;
  *arrival = *pointer->arrival;
  *cost = pointer->cost;

  return true;
}
