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

bool is_empty_way(int *arr);
bool is_vertical(int start);
bool is_same_dir(int startWay);

void until_finish_active(void);
void update_time(void);
int check_passed_car(void);
void check_no_car(void);
void print_situ_result(void);
bool is_work_all_finished(void);
void print_result(void);

struct stat {
	int dir;
	int time;
	int way;
};

typedef struct car{
	int deg;
	struct stat stat[2];
} car;

typedef struct waiting{
	int car;
	int index_queue;
} waiting_way;

int ticks = 0;
int vehicles = 0;
int *startPoint;
int *wait_queue;
int way_passed[4] = {0,};
int passed_car = 0;
//int thr1Point[15] = {0,},thr2Point[15]={0,},thr3Point[15]={0,},thr4Point[15]={0,};
int *p_startPoint[4];
bool finish_active[4] = {false, false, false, false};

car running_car = {9, {{0,0,0},{0,0,0}}};
waiting_way way_waiting_car[4] = {{0,0},{0,0},{0,0},{0,0}};

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

	/***********************************************************/
	//distribute startPoint to threads per 1 second
	int one=0, two=0, three=0, four=0, tick = 0;
	while(1){   //-> until ticks ends
		sleep(1);
		//printf("===============================\n");
		printf("tick %d\n", tick);
		printf("===============================\n");
		//pthread_cond_broadcast(&cond);

		if(tick<vehicles){
				if(startPoint[tick] == 1){
					p_startPoint[0][one] = startPoint[tick];
					one++;
				}
				else if(startPoint[tick] == 2){
					p_startPoint[1][two] = startPoint[tick];
					two++;
                                }
				else if(startPoint[tick] == 3){
                                        p_startPoint[2][three] = startPoint[tick];
					three++;
                                }
				else if(startPoint[tick] == 4){
                                        p_startPoint[3][four] = startPoint[tick];
					four++;
                                }
			
			tick++;
		}
		else break;
		pthread_cond_broadcast(&cond);
		until_finish_active();
		update_time();
		if((passed_car = check_passed_car())!=0){
			printf("passed car : %d\n", passed_car);
		}
		print_situ_result();
		printf("===============================\n");
		check_no_car();

		if(is_work_all_finished() == true){
			print_situ_result();
			break;
		}
	}
	print_result();

	return 0;
}

/***********************************************************/
void *thread_func(void *arg){
	int startWay = *((int *)arg);
	int way_waiting = 0;
	while(1){
		pthread_mutex_unlock(&mutex);
		pthread_cond_wait(&cond, &mutex);
		//printf("%d -> ", startWay);
		//for(int i = 0;i<15;i++){
		//	printf("%d ", p_startPoint[startWay-1][i]);
		//}
		//printf("\n");

		if(!is_empty_way(p_startPoint[startWay-1]) && !is_vertical(startWay))
		{
			//EMPTY ROAD
			if(running_car.deg == NO_CAR)
			{
				printf("EMPTY\n");
				running_car.deg = startWay % 2;
				running_car.stat[0].dir = startWay;
				//대기큐 랜덤 뽑기해서 넣기
				running_car.stat[0].way = p_startPoint[startWay-1][way_waiting];
				//랜덤뽑기한 후 대기큐에 대한 셋팅
				running_car.stat[0].time = 0;
				//이건 아마 없애도 될 듯
				p_startPoint[startWay-1][way_waiting] = 0;
				way_waiting++;
			}
			//FACE-TO-FACE CAR
			else if((running_car.deg == startWay % 2) && (!is_same_dir(startWay)))
			{
			  if((running_car.stat[0].dir!=0&&running_car.stat[0].time!=0) || (running_car.stat[0].dir!=0&&running_car.stat[0].time!=0))
			  {
				printf("start way : %d\n", startWay);
				if(running_car.stat[0].way==0){
					//
					running_car.stat[0].way = p_startPoint[startWay-1][way_waiting];
					//
					running_car.stat[0].dir = startWay;
					running_car.stat[0].time = 0;
					//
					p_startPoint[startWay-1][way_waiting] = 0;
				}
				else if(running_car.stat[1].way==0){
					//
                                        running_car.stat[1].way = p_startPoint[startWay-1][way_waiting];
					//
                                        running_car.stat[1].dir = startWay;
                                        running_car.stat[1].time = 0;
					//
					p_startPoint[startWay-1][way_waiting] = 0;
                                }
				way_waiting++;
			  }
			  else{ //마주보는 방향이지만 이미 출발한 차 존재
			  }
			}
		}
		else{//lock을 얻을 조건이 안될 때
			//각 방향 대기큐에 차 넣어주고 전체 대기큐에도 넣어줌.
			//각 방향 대기큐 숫자 늘리기
		}
		//way_waiting++;
		finish_active[startWay-1] = true;
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

void until_finish_active(void){
	bool finished;
	while(1){
		finished = true;
		for(int i = 0;i < 4;i++){
			if(finish_active[i] == false){
				finished = false;
				break;
			}
		}
		if(finished == true){
			printf("all finish\n");
			break;
		}
	}
}

void update_time(void){
	for(int i = 0;i < 2;i++){
		if(running_car.stat[i].way != 0){
			printf("car %d is updated to %d\n", running_car.stat[i].way, running_car.stat[i].time+1);
			running_car.stat[i].time++;
		}
	}
	//printf("update\n");
	return;
}

int check_passed_car(void){
	int pass_car = 0;
	for(int i = 0; i<2;i++){
		if(running_car.stat[i].time == 2){
			pass_car = running_car.stat[i].way;
			running_car.stat[i].dir = 0;
			running_car.stat[i].time = 0;
			running_car.stat[i].way = 0;
			return pass_car;
		}
	}
	return 0;
}
void check_no_car(void){
	if((running_car.stat[0].dir == 0) && (running_car.stat[1].dir == 0))
		running_car.deg = NO_CAR;
}
void print_situ_result(void){
	//각 tick마다 출력될 결과들 여기서 출력
}
bool is_work_all_finished(void){
	//모든 일이 끝났는지 확인. 도로가 비어있고 대기큐에 아무것도 없는지
}
void print_result(void){
	//제일 마지막 결과 출력해주는 함수
}
