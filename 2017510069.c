#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
//Constant Numbers
#define PATIENTS 99
#define HC_STAFF 8
const int M = 8;
const int N = 3;

//Semaphores
sem_t Rooms[8];
sem_t Room_Locks[8];
sem_t is_Room_Used[8];
sem_t AllDone;

//Threads
void* hc_Staff(void *num);
void* patient(void*);

//Other Functions
void randwait();
int sem_Value(sem_t sem);
int get_Avaible_Room_Number(sem_t Semaphores[]);

int main(void) {

	int i, j;
	//Semaphore initializon
	for (i = 0; i < 8; i++)
		sem_init(&Rooms[i], 0, 3);
	for (i = 0; i < 8; i++)
		sem_init(&Room_Locks[i], 0, 1);
	for (i = 0; i < 8; i++)
		sem_init(&is_Room_Used[i], 0, 0);

	sem_init(&AllDone, 0, 0);

	//Healthcare Staff Threads
	pthread_t hcstid[HC_STAFF];
	int Number_Hcs[HC_STAFF];

	for (j = 0; j < HC_STAFF; j++) {
		Number_Hcs[j] = j;
	}
	for (i = 0; i < HC_STAFF; i++) {
		pthread_create(&hcstid[i], NULL, hc_Staff, (void*) &Number_Hcs[i]);
	}

	//patient Threads
	pthread_t pid[PATIENTS];
	int Number[PATIENTS + 1];

	for (i = 1; i < PATIENTS + 1; i++) {
		Number[i] = i;
	}
	for (i = 1; i < PATIENTS + 1; i++) {
		randwait();
		pthread_create(&pid[i], NULL, patient, (void*) &Number[i]);
	}

	//Joins
	for (i = 1; i < PATIENTS + 1; i++) {
		pthread_join(pid[i], NULL);
	}

	for (i = 0; i < HC_STAFF; i++) {
		pthread_join(hcstid[i], NULL);
	}

	printf("All patients done.\n");
	return 0;
}

void* hc_Staff(void *info) {
	int num = *(int*) info;
	int flag = 0;
	int flag2 = 0;
	int flag3 = 0;
	while (sem_Value(AllDone) != PATIENTS) {
		while (flag == 0) {
			if (sem_Value(AllDone) == PATIENTS) {
				break;
			}
			if (sem_Value(Rooms[num]) == 3) {
				flag = 1;
				printf("Healthcare staff %d opening the room %d\n", num, num);
			}
		}
		while (flag == 1) {
			if (sem_Value(AllDone) == PATIENTS) {
				break;
			}
			if (sem_Value(Rooms[num]) == 0) {
				sem_wait(&Room_Locks[num]);
				flag = 0;
				printf("Healthcare staff %d: Room is full!\n", num);
				sem_post(&is_Room_Used[num]);
			} else if (sem_Value(Rooms[num]) == 2 && flag2 == 0) {
				printf("Healthcare staff %d: The last people,let's start! Please, pay  attention to your social distance and hygiene; use a mask.\n",
						num);
				flag2 = 1;
			}
		}
		while (flag == 0) {
			if (sem_Value(AllDone) == PATIENTS) {
				break;
			}
			if (sem_Value(Rooms[num]) == 3) {
				printf("Healthcare staff %d ventilating the room %d\n", num, num);
				sleep(3);
				sem_post(&Room_Locks[num]);
				flag = 1;
			}
		}
		flag = 0;
		flag2 = 0;
		flag3 = 0;
	}

}

void* patient(void *info) {
	int num = *(int*) info;
	printf("Patient number %d waiting for an empty room\n", num);
	int m = get_Avaible_Room_Number(Rooms);
	while (m == -1) {
		m = get_Avaible_Room_Number(Rooms);
	}

	sem_wait(&Rooms[m]);

	printf("Patient number %d entered the room %d :Preparing for the test and filling out the form\n", num, m);
	int flag = 0;
	while (flag == 0) {
		if (sem_Value(Rooms[m]) == 0) {
			printf("Patient number %d is being tested in room %d\n", num, m);
			sleep(3);
			flag = 1;
		}
	}
	sem_post(&Rooms[m]);

	printf("Patient number %d leaved the room %d\n", num, m);
	sem_post(&AllDone);
}

void randwait() {
	int random = 1;
	srand(time(NULL));
	random = (rand() % 3) + 1;
	sleep(random);
}

int sem_Value(sem_t sem) {
	int result;
	sem_getvalue(&sem, &result);
	return result;
}

int get_Avaible_Room_Number(sem_t Semaphores[]) {
	int available, i, min, min2;
	min = 99;
	min2 = 99;
	available = -1;
	for (i = 0; i < 8; i++) {
		if (sem_Value(Semaphores[i]) != 0 && sem_Value(Room_Locks[i]) != 0) {
			if (sem_Value(Semaphores[i]) <= min) {
				if (sem_Value(is_Room_Used[i]) <= min2) {
					min2 = sem_Value(is_Room_Used[i]);
					min = sem_Value(Semaphores[i]);
					available = i;

				}

			}
		}

	}
	return available;
}

