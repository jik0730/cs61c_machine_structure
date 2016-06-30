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
  timeHM_t departure;
  timeHM_t arrival;
  int cost;
  flight_t* next;
};

struct flightSys {
  airport_t* airport;
  size_t size;
};

struct airport {
  char* name;
  flight_t* flights;
  airport_t* next;
  size_t size;
};

/*
   This should be called if memory allocation failed.
 */
static void allocation_failed() {
  fprintf(stderr, "Out of memory.\n");
  exit(EXIT_FAILURE);
}

void freeAirport(airport_t* airport);
void freeFlights(flight_t* flights);


/*
   Creates and initializes a flight system, which stores the flight schedules of several airports.
   Returns a pointer to the system created.
 */
flightSys_t* createSystem() {
  flightSys_t* flightSys;
  flightSys = (flightSys_t*) malloc(1 * sizeof(flightSys_t)); // Create single flight system.

  if(flightSys == NULL) { // In case of fail of memory allocation.
    allocation_failed();
  }

  flightSys->airport = NULL;
  flightSys->size = 0;
  return flightSys;
}


/*
   Frees all memory associated with this system; that's all memory you dynamically allocated in your code.
 */
void deleteSystem(flightSys_t* s) {
  // Recursively delete system.
  if(s == NULL || s->size == 0) { // Base case of recursive function.
    return ;
  }

  airport_t* temp = s->airport;
  for(int i = 0; i < s->size; i++) {
    freeFlights(temp->flights);
    temp = temp->next;
  }
  freeAirport(s->airport);
  free(s); // Free.

  return ;
}


/*
   Adds a airport with the given name to the system. You must copy the string and store it.
   Do not store "name" (the pointer) as the contents it point to may change.
 */
void addAirport(flightSys_t* s, char* name) {
  airport_t* airport = (airport_t*) malloc(1 * sizeof(airport_t)); // Allocate memory to airport.
  if(airport == NULL) {
    allocation_failed();
  }

  char* a_name = (char*) malloc(sizeof(name)); // Allocate memory to a_name with the same size of a memory we received.
  if(a_name == NULL) { // In case of fail of memory allocation.
    allocation_failed();
  }
  strcpy(a_name, name); // String copy function to a_name from name.
  airport->name = a_name;
  airport->size = 0;
  airport->flights = NULL;
  airport->next = NULL;

  if(s->size == 0) {
    s->airport = airport;
    s->size++;
  } else {
    airport_t* temp = s->airport;
    for(int i = 0; i < s->size - 1; i++) {
      temp = temp->next;
    }
    temp->next = airport;
    s->size++;
  }
  return ;
}


/*
   Returns a pointer to the airport with the given name.
   If the airport doesn't exist, return NULL.
 */
airport_t* getAirport(flightSys_t* s, char* name) {
  if(s->size == 0) {
    return NULL;
  }

  airport_t* temp = s->airport;
  for(int i = 0; i < s->size; i++) {
    if(strcmp(temp->name, name) == 0) {
      return temp;
    }
    temp = temp->next;
  }

  return NULL;
}


/*
   Print each airport name in the order they were added through addAirport, one on each line.
   Make sure to end with a new line. You should compare your output with the correct output
   in flights.out to make sure your formatting is correct.
 */
void printAirports(flightSys_t* s) {
  if(s->size == 0) {
    return ;
  }
  airport_t* temp = s->airport;

  for(int i = 0; i < s->size; i++) {
    printf("%s\n", temp->name);
    temp = temp->next;
  }
}


/*
   Adds a flight to src's schedule, stating a flight will leave to dst at departure time and arrive at arrival time.
 */
void addFlight(airport_t* src, airport_t* dst, timeHM_t* departure, timeHM_t* arrival, int cost) {
  if(src->size == 0) {
    src->flights = (flight_t*) malloc(1 * sizeof(flight_t));
    src->flights->src = src;
    src->flights->dst = dst;
    src->flights->departure = *departure;
    src->flights->arrival = *arrival;
    src->flights->cost = cost;
    src->flights->next = NULL;
    src->size++;
    return ;
  }

  flight_t* temp = src->flights;
  for(int i = 0; i < src->size - 1; i++) {
    temp = temp->next;
  }
  temp->next = (flight_t*) malloc(sizeof(flight_t));
  temp->next->src = src;
  temp->next->dst = dst;
  temp->next->departure = *departure;
  temp->next->arrival = *arrival;
  temp->next->cost = cost;
  temp->next->next = NULL;
  src->size++;
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
  for(int i = 0; i < s->size; i++) {
    printf("%s ", temp->dst->name);
    printTime(&temp->departure);
    printf(" ");
    printTime(&temp->arrival);
    printf(" %d\n", temp->cost);
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
  if(src->size == 0) {
    return false;
  }

  flight_t* temp = src->flights;
  airport_t* toCompare = (airport_t*) malloc(1 * sizeof(airport_t));
  if(toCompare == NULL) {
    allocation_failed();
  }
  toCompare->name = NULL;
  toCompare->next = NULL;
  toCompare->size = 0;
  toCompare->flights = NULL;

  int costcomp = INT_MAX;
  for(int i = 0; i < src->size; i++) {
    if(temp->cost < costcomp && isAfter(&temp->departure, now)) {
      costcomp = temp->cost;
    }
    temp = temp->next;
  }

  if(costcomp == INT_MAX) {
    return false;
  }

  temp = src->flights;
  for(int i = 0; i < src->size; i++) {
    if(temp->cost == costcomp) {
      addFlight(toCompare, temp->dst, &temp->departure, &temp->arrival, temp->cost);
    }
    temp = temp->next;
  }

  temp = toCompare->flights;
  flight_t* pointer = temp;
  for(int i = 0; i < toCompare->size; i++) {
    if(isAfter(&pointer->departure, &temp->departure)) {
      pointer = temp;
    }
    temp = temp->next;
  }

  *departure = pointer->departure;
  *arrival = pointer->arrival;
  *cost = pointer->cost;

  freeFlights(toCompare->flights);
  freeAirport(toCompare);
  return true;
}

void freeAirport(airport_t* airport) {
  if(airport) {
    free(airport->name);
    freeAirport(airport->next);
    free(airport);
  }
}

void freeFlights(flight_t* flights) {
  if(flights) {
    freeFlights(flights->next);
    free(flights);
  }
}
