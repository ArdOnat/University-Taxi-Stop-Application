#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// 2015510050 ARDA ONAT


// The maximum number of customer threads.
#define MAX_STUDENTS 100
#define MAX_DRIVERS 10

int availableSeats = 4; // Available seats at max for taxis.

int i = 0;
void *student(void *num);
void *driver(void *num);
void displayTaxis();

// Driver output counter.
int saying = 4;

// Total # of students.
sem_t waitingStudents;

// Total # of not busy drivers.
sem_t availableDrivers;

// Driver sleeping.
sem_t driverSleep;

// Student Mutex.
sem_t studentMutex;

// Driver critical section.
sem_t driverMutex;

// No drivers available at taxi stop.
sem_t waitForDrivers;

// Check if all students are carried.
int dayFinished = 0;

// Display # of students in taxis.
int numberOfStudents[MAX_DRIVERS];

int main(int argc, char *argv[]) {

  // Pthread arrays
  pthread_t taxiTids[MAX_DRIVERS];
  pthread_t tid[MAX_STUDENTS];


  int i = 0;

  // All taxis are empty at first
  for (i = 0; i < MAX_STUDENTS; i++) {
    numberOfStudents[i]=0;

  }


  for (i = 0; i < MAX_DRIVERS; i++) {
    printf("Taxi%d[%d]   ", i, numberOfStudents[i]);

    printf("\n \n");
  }

  int Number[MAX_STUDENTS];
  int Number2[MAX_DRIVERS];

  for (i = 0; i < MAX_STUDENTS; i++) {
    Number[i] = i;
  }
  for (i = 0; i < MAX_DRIVERS; i++) {
    Number2[i] = i;
  }

  // Initialize the semaphores.

  sem_init(&waitingStudents, 0, MAX_STUDENTS);
  sem_init(&availableDrivers, 0, MAX_DRIVERS);
  sem_init(&driverSleep, 0, 0);
  sem_init(&studentMutex, 0, 1);
  sem_init(&waitForDrivers, 0, 0);
  sem_init(&driverMutex, 0, 1);

  // Create the drivers.
  for (i = 0; i < MAX_DRIVERS; i++) {
    pthread_create(&taxiTids[i], NULL, driver, (void *)&Number2[i]);
  }

  // Display taxis.
  int counter = 0;
  for (counter = 0; counter < MAX_DRIVERS; counter++) {
    printf("Taxi%d[%d]   ", counter, numberOfStudents[counter]);
  }
  printf("\n");

  // Create the students
  for (i = 0; i < MAX_STUDENTS; i++) {
    pthread_create(&tid[i], NULL, student, (void *)&Number[i]);
    sleep(1);
  }

  // Join each of the threads to wait for them to finish.
  for (i = 0; i < MAX_STUDENTS; i++) {
    pthread_join(tid[i], NULL);
  }

  // Display day ending message.

  displayTaxis(); // TRY THİS
  printf("All students are transported, drivers are leaving for the day!");

  return 0;
}

// Student thread functions.
void *student(void *number) {

  // sem_wait(&waitingStudents);
  sem_wait(&studentMutex); // Critial section enter.

  int studentNum = *(int *)number; // Get student number from parameter.

  printf("\n");
  printf("-------------------------- \n \n");

  printf("Student %d is now at taxi stop. \n", studentNum);
  int drivers = 0;
  sem_getvalue(&availableDrivers, &drivers); // Get number of available drivers.

  if (drivers == 0) // Check for available taxis. If cant find any sleep
                    // the student function.
  {
    printf("Student%d waiting at taxi stop. (NO TAXI AVAILABLE) \n \n",
           studentNum);
    sem_wait(&waitForDrivers); // Sleep the students that are waiting for taxis
                               // at taxi stop.
  }

  if (availableSeats > 0) {
    printf("Student%d is going in a taxi \n \n", studentNum);
    sem_wait(&waitingStudents);
    if (availableSeats == 4) // If driver is sleeping.
    {
      printf("Student%d woke up driver \n", studentNum);
      sem_post(&driverSleep); // Wake up the driver if he is sleeping.
    }
    availableSeats--;
    printf("\n");
    sleep(1);
  }
  sleep(1);
  sem_post(&studentMutex); // Critical section finish.

  int studentCounter = 0;
  sem_getvalue(&waitingStudents, &studentCounter); // Get waiting students count.

  if (studentCounter == 0) // If this is the last student thread, sleep until
                           // all driver threads finish.
  {
    sleep(5);
  }
}

// Display taxis and students in them.
void displayTaxis() {
  for (i = 0; i < MAX_DRIVERS; i++) {
    printf("Taxi%d[%d]   ", i, numberOfStudents[i]);
  }
  printf("\n \n");
}

// Driver Thread Function.
void *driver(void *number) {
  while (!dayFinished) {
    int driverNum = *(int *)number;
    printf("Driver%d is sleeping.\n", driverNum);
    printf("\n");
    sem_wait(&driverSleep); // Sleep until a student arrives.

    sem_wait(&driverMutex); // Critical section (only 1 driver will
                            // call for students).

    printf("Driver%d woke up.\n \n", driverNum);
    sleep(1);

    numberOfStudents[driverNum] = 1; // One student alreay arrived.

    if (!dayFinished) {
      while (availableSeats > 0) // If taxi has available seats.
      {
        if (saying != availableSeats) // Display control.
        {
          displayTaxis();
          printf("\n");

          int seats;
          sem_getvalue(&availableSeats,
                       &seats); // Get number of available seats.

          // Call for students if taxi is not full.
          if (seats > 0) {
            printf("Driver%d : The last %d students, let's get up!!\n \n ",
                   driverNum, availableSeats);
          }

          // Start transportation if taxi is full.
          else {
            printf("Driver%d : My taxi is full, let's go!! \n ", driverNum);
            printf("ALERT: Taxi%d is transporting \n \n", driverNum);
          }
          numberOfStudents[driverNum]++;
          saying--;
        }
      }

      sem_post(&driverMutex); // Critical section end.

      // Reset values

      availableSeats = 4;
      saying = 4;

      // Transportation

      sem_wait(&availableDrivers);

      numberOfStudents[driverNum] = 4;
      displayTaxis();
      printf("\n");

      // Reset Values

      sleep(5);

      // Return From Transportation
      printf("ALERT: --Taxi%d returned from transporting.--\n", driverNum);
      numberOfStudents[driverNum] = 0;
      printf("\n");
      printf("-------------------------- \n \n");

      sem_post(&availableDrivers); // Driver is back at taxi port.

      int waitfordrivers;
      sem_getvalue(&availableDrivers, &waitfordrivers);

      if (waitfordrivers == 1) { // If this taxi is the only taxi at the
        // taxi port wake the students that are waiting for a taxi.
        int isLocked;
        sem_getvalue(&waitForDrivers, &isLocked); // Check if any student
                                                  // thread is sleeping.
        if (isLocked == 0) {
          sem_post(&waitForDrivers); // Wake the sleeping student threads.
        }
      }
    }
  }

}
