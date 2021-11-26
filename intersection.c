#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

void *thread_func1(void *arg);
void *thread_func2(void *arg);
void *thread_func3(void *arg);
void *thread_func4(void *arg);


int ticks = 0;
int vehicles = 0;
int *startPoint;
int thr1Point[15] = {0,},thr2Point[15]={0,},thr3Point[15]={0,},thr4Point[15]={0,};

int main(void){
	/***********************************************************/
	//Enter the number and initializing startPoint Array
	printf("Total number of vehicles : ");
	scanf("%d", &vehicles);
	startPoint = (int*) calloc(vehicles, sizeof(int));

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
	if(pthread_create(&tid[0], NULL, thread_func1, NULL) != 0){
		fprintf(stderr, "pthread_create error\n");
		exit(1);
	}
	if(pthread_create(&tid[1], NULL, thread_func2, NULL) != 0){
                fprintf(stderr, "pthread_create error\n");
                exit(1);
        }
	if(pthread_create(&tid[2], NULL, thread_func3, NULL) != 0){
                fprintf(stderr, "pthread_create error\n");
                exit(1);
        }
	if(pthread_create(&tid[3], NULL, thread_func4, NULL) != 0){
                fprintf(stderr, "pthread_create error\n");
                exit(1);
        }
	/***********************************************************/
	//distribute startPoint to threads per 1 second
	int one=0, two=0, three=0, four=0, i=0;
	while(1){   //-> until ticks ends
		sleep(1);
		if(i<vehicles){
				if(startPoint[i] == 1){
					thr1Point[one] = startPoint[i];
					//printf("1 : %d\n", thr1Point[one]);
					one++;
				}
				else if(startPoint[i] == 2){
					thr2Point[two] = startPoint[i];
					//printf("2 : %d\n", thr2Point[two]);
					two++;
                                }
				else if(startPoint[i] == 3){
                                        thr3Point[three] = startPoint[i];
					//printf("3 : %d\n", thr3Point[three]);
					three++;
                                }
				else if(startPoint[i] == 4){
                                        thr4Point[four] = startPoint[i];
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
void *thread_func1(void *arg){
	int i = 0;
	while(1){
		if(thr1Point[i] != 0){
			printf("thread 1 : %d\n", thr1Point[i]);
			i++;
		}
		else continue;
	}
	return NULL;
}
void *thread_func2(void *arg){
        int i = 0;
        while(1){
                if(thr2Point[i] != 0){
                        printf("thread 2 : %d\n", thr2Point[i]);
                        i++;
                }
		else continue;
        }
        return NULL;
}
void *thread_func3(void *arg){
        int i = 0;
        while(1){
                if(thr3Point[i] != 0){
                        printf("thread 3 : %d\n", thr3Point[i]);
                        i++;
                }
		else continue;
        }
        return NULL;
}
void *thread_func4(void *arg){
	int i = 0;
        while(1){
                if(thr4Point[i] != 0){
                        printf("thread 4 : %d\n", thr4Point[i]);
                        i++;
                }
		else continue;
        }
        return NULL;
}

