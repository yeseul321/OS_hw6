#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>

#define NO_CAR 9
#define VERTICAL 1
#define HORIZONTAL 0
#define NORTH 1
#define WEST 2
#define SOUTH 3
#define EAST 4

void *thread_func(void *arg);
void *thread_func2(void *arg);
void *thread_func3(void *arg);
void *thread_func4(void *arg);

bool is_empty_way(int *arr);
bool is_vertical(int start);
bool is_same_dir(int startWay);

struct stat {
	int dir;
	int time;
	int way;
};

typedef struct car{
	int deg;
	struct stat stat[2];
} car;

int ticks = 0;
int vehicles = 0;
int *startPoint;
int *wait_queue;
int way_passed[4] = {0,};
//int thr1Point[15] = {0,},thr2Point[15]={0,},thr3Point[15]={0,},thr4Point[15]={0,};
int *p_startPoint[4];
car running_car = {9, {{0,0,0},{0,0,0}}};

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int main(void){
	/***********************************************************/
	//Enter the number and initializing startPoint Array
	printf("Total number of vehicles : ");
	scanf("%d", &vehicles);
	startPoint = (int*) calloc(vehicles, sizeof(int));
	for(int i = 0;i < 4;i++){
		p_startPoint[i] = (int *)calloc(15, sizeof(int));
	}

	srand((unsigned)time(NULL));
	printf("Start Point : ");
	for(int i=0;i<vehicles;i++){
		startPoint[i] = (rand() % 4) + 1;
		printf("%d ", startPoint[i]);
	}
	printf("\n");
	/***********************************************************/
	//pthread create
	pthread_t tid[4];
	int way_num[4] = {0, };
	for(int i=0;i < 4;i++){
		way_num[i] = i+1;
		if(pthread_create(&tid[i], NULL, thread_func, (void *)&way_num[i]) != 0){
			fprintf(stderr, "pthread_create error\n");
			exit(1);
		}	
	}
	/*
	if(pthread_create(&tid[1], NULL, thread_func, NULL) != 0){
                fprintf(stderr, "pthread_create error\n");
                exit(1);
        }
	if(pthread_create(&tid[2], NULL, thread_func, NULL) != 0){
                fprintf(stderr, "pthread_create error\n");
                exit(1);
        }
	if(pthread_create(&tid[3], NULL, thread_func, NULL) != 0){
                fprintf(stderr, "pthread_create error\n");
                exit(1);
        }
	*/

	/***********************************************************/
	//distribute startPoint to threads per 1 second
	int one=0, two=0, three=0, four=0, i=0;
	while(1){   //-> until ticks ends
		sleep(1);
		printf("=============ticks : %d===============\n", i);
		pthread_cond_broadcast(&cond);

		if(i<vehicles){
				if(startPoint[i] == 1){
					p_startPoint[0][one] = startPoint[i];
					//printf("1 : %d\n", thr1Point[one]);
					one++;
				}
				else if(startPoint[i] == 2){
					p_startPoint[1][two] = startPoint[i];
					//printf("2 : %d\n", thr2Point[two]);
					two++;
                                }
				else if(startPoint[i] == 3){
                                        p_startPoint[2][three] = startPoint[i];
					//printf("3 : %d\n", thr3Point[three]);
					three++;
                                }
				else if(startPoint[i] == 4){
                                        p_startPoint[3][four] = startPoint[i];
					//printf("4 : %d\n", thr4Point[four]);
					four++;
                                }
			
			i++;
		}
		else break;
	}

	return 0;
}

/***********************************************************/
void *thread_func(void *arg){
	int startWay = *((int *)arg);
	int i = 0;
	while(1){
		pthread_mutex_unlock(&mutex);
		pthread_cond_wait(&cond, &mutex);
		printf("%d\n", startWay);
		if(!is_empty_way(p_startPoint[startWay-1]) && !is_vertical(startWay))
		{
			//EMPTY ROAD
			if(running_car.deg == NO_CAR)
			{
				printf("EMPTY\n");
				running_car.deg = startWay % 2;
				running_car.stat[0].dir = startWay;
				running_car.stat[0].way = p_startPoint[startWay-1][i];
				running_car.stat[0].time = 0;
			}
			//FACE-TO-FACE CAR
			else if((running_car.deg == startWay % 2) && (!is_same_dir(startWay)))
			{
				printf("start way : %d\n", startWay);
				if(running_car.stat[0].way!=0){
					running_car.stat[0].way = p_startPoint[startWay-1][i];
					running_car.stat[0].dir = startWay;
					running_car.stat[0].time = 0;
				}
				else if(running_car.stat[1].way!=0){
                                        running_car.stat[1].way = p_startPoint[startWay-1][i];
                                        running_car.stat[1].dir = startWay;
                                        running_car.stat[1].time = 0;
                                }
			}
		}
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

bool is_empty_way(int *arr){
	for(int i = 0;i < 15;i++){
		if(arr[i] == 0)
			continue;
		else
			return false;
	}
	return true;
}
bool is_vertical(int start){
	if(running_car.deg == (start % 2)) return false; //Same Degree
	else if(running_car.deg == NO_CAR) return false; //No Car On The Road
	else return true;
}
bool is_same_dir(int startWay){
	for(int i = 0 ;i<2;i++){
		if(running_car.stat[i].dir == startWay) return true;
	}
	return false;
}
