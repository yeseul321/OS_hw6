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
        int data;
        int index;
        struct waiting *next;
} waiting_way;

void *thread_func(void *arg);

bool is_empty_way(struct waiting *waiting);
bool is_vertical(int start);
bool is_same_dir(int startWay);
bool is_work_all_finished(void);

void until_finish_active(void);
void update_time(void);
void check_no_car(void);
void print_situ_result(void);
void print_result(int tick);
void way_addData(struct waiting *waiting, int data, int index);

int check_waiting_length(struct waiting *waiting);
int setWaylist(struct waiting *waiting,int random);
int check_passed_car(void);

int vehicles = 0;
int *startPoint;
int way_passed[4] = {0,};
int passed_car = 0;
int *all_waiting_list;
int *p_startPoint[4];
int *each_passed;
bool finish_active[4] = {false, false, false, false};

car running_car = {9, {{0,0,0},{0,0,0}}};

waiting_way *way_head[4];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int main(void){
	each_passed = calloc(4, sizeof(int));
	for(int i = 0;i<4;i++){
        	way_head[i] = malloc(sizeof(struct waiting));
        	way_head[i]->next = NULL;
	}
	all_waiting_list = calloc(16, sizeof(int));
	/***********************************************************/
	//Enter the number and initializing startPoint Array
	printf("Total number of vehicles : ");
	scanf("%d", &vehicles);
	startPoint = (int*) calloc(vehicles, sizeof(int));
	srand((unsigned)time(NULL));

	printf("Start Point : ");
	for(int i = 0;i<vehicles;i++){
		startPoint[i] = (rand() % 4) + 1;
		printf("%d ", startPoint[i]);
	}

	for(int i = 0;i < 4;i++){
                p_startPoint[i] = (int *)calloc(15, sizeof(int));
        }

	printf("\n");
	/***********************************************************/
	//pthread create
	pthread_t tid[4];
	int way_num[4] = {0, };
	for(int i=4;i > 0;i--){
		way_num[i] = i;
		if(pthread_create(&tid[i], NULL, thread_func, (void *)&way_num[i]) != 0){
			fprintf(stderr, "pthread_create error\n");
			exit(1);
		}	
	}

	/***********************************************************/
	//distribute startPoint to threads per 1 second
	int tick = 0;
	while(1){   //-> until ticks ends
		sleep(1);
		printf("tick : %d\n", tick+1);
		printf("===============================\n");

		if(tick<vehicles){
				if(startPoint[tick] == 1){
					way_addData(way_head[0],1,tick);
					all_waiting_list[tick] = 1;
				}
				else if(startPoint[tick] == 2){
					way_addData(way_head[1],2,tick);
					all_waiting_list[tick] = 2;
                                }
				else if(startPoint[tick] == 3){
					way_addData(way_head[2],3,tick);
					all_waiting_list[tick] = 3;
                                }
				else if(startPoint[tick] == 4){
					way_addData(way_head[3],4,tick);
					all_waiting_list[tick] = 4;
                                }
			
		}
		pthread_cond_broadcast(&cond);

		until_finish_active();
		update_time();
		print_situ_result();
		check_no_car();

		if(tick>=vehicles){
			if(is_work_all_finished() == true){
				sleep(1);
				printf("tick : %d\n", (++tick) + 1);
                		printf("===============================\n");
				print_situ_result();
				break;
			}
		}
		tick++;
	}
	print_result(++tick);

	return 0;
}

/***********************************************************/
void *thread_func(void *arg){
	int startWay = *((int *)arg);
	int way_waiting = 0;
	int waiting_length = 0;
	int random = 0;
	int index = 0;
	srand((unsigned)time(NULL));
	while(1){
		pthread_mutex_lock(&mutex);
		pthread_cond_wait(&cond, &mutex);

		if(!is_empty_way(way_head[startWay-1]) && !is_vertical(startWay))
		{
			waiting_length = check_waiting_length(way_head[startWay-1]);
			//EMPTY ROAD
			if(running_car.deg == NO_CAR)
			{
				running_car.deg = startWay % 2;
				running_car.stat[0].dir = startWay;
				running_car.stat[0].way = startWay;
				//대기큐 랜덤 뽑기해서 넣기
				random = rand()%(waiting_length-1)+1;
				//랜덤뽑기한 후 대기큐에 대한 셋팅
				index = setWaylist(way_head[startWay-1],random);
				all_waiting_list[index] = 0;
				running_car.stat[0].time = 0;

				way_waiting++;
			}
			//FACE-TO-FACE CAR
			else if((running_car.deg == startWay % 2) && (!is_same_dir(startWay)))
			{
			  if((running_car.stat[0].dir!=0&&running_car.stat[0].time!=0) || (running_car.stat[1].dir!=0&&running_car.stat[1].time!=0))
			  {
				if(running_car.stat[0].way==0){
					//랜덤
					random = rand()%(waiting_length-1)+1;
                                	running_car.stat[0].way = startWay;

					//랜덤뽑기한 후 대기큐에 대한 셋팅
                                	index = setWaylist(way_head[startWay-1],random);
					all_waiting_list[index] = 0;
					running_car.stat[0].dir = startWay;
					running_car.stat[0].time = 0;
				}
				else if(running_car.stat[1].way==0){
					//랜덤
                                        random = rand()%(waiting_length-1)+1;
                                        running_car.stat[1].way = startWay;
                                        
					//랜덤뽑기한 후 대기큐에 대한 셋팅
                                        index = setWaylist(way_head[startWay-1],random);
					all_waiting_list[index] = 0;
                                        running_car.stat[1].dir = startWay;
                                        running_car.stat[1].time = 0;
					
                                }
				way_waiting++;
			  }
			  else{ //마주보는 방향이지만 이미 출발한 차 존재
				  //DO NOTHING
			  }
			}
		}
		else{//lock을 얻을 조건이 안될 때
			//DO NOTHING
		}

		finish_active[startWay-1] = true;
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

bool is_empty_way(struct waiting *way_waiting){ //각 쓰레드의 waiting list가 비어있는지 검사
	if(way_waiting->next == NULL){
		return true;
	}
	else return false;
}
bool is_vertical(int start){ //현재 선택된 차량이 도로 위의 차량과 수직위치인지 검사
	if(running_car.deg == (start % 2)) return false; //Same Degree
	else if(running_car.deg == NO_CAR) return false; //No Car On The Road
	else return true;
}
bool is_same_dir(int startWay){ //현재 선택된 차량이 도로 위의 차량과 완전 같은 방향인지 검사
	for(int i = 0 ;i<2;i++){
		if(running_car.stat[i].dir == startWay) return true;
	}
	return false;
}

void until_finish_active(void){ //각 쓰레드가 모두 일을 끝내고 다시 대기상태로 돌아갈 때 까지 기다리는 함수
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
			break;
		}
	}
}

void update_time(void){ //운행중인 차량의 운행 시간을 늘려주는 함수
	for(int i = 0;i < 2;i++){
		if(running_car.stat[i].way != 0){
			running_car.stat[i].time++;
		}
	}
	return;
}

int check_passed_car(void){ //차량이 도로를 완전히 통과했는지 확인하고 그 차량의 번호를 리턴하는 함수
	int pass_car = 0;
	for(int i = 0; i<2;i++){
		if(running_car.stat[i].time == 2){
			pass_car = running_car.stat[i].way;
			each_passed[running_car.stat[i].dir-1]++;
			running_car.stat[i].dir = 0;
			running_car.stat[i].time = 0;
			running_car.stat[i].way = 0;
			return pass_car;
		}
	}
	return 0;
}
void check_no_car(void){ //도로위에 운행중인 차량이 전혀 없는지 확인하는 함수
	if((running_car.stat[0].dir == 0) && (running_car.stat[1].dir == 0))
		running_car.deg = NO_CAR;
}
void print_situ_result(void){ //각 tick마다 현 상황을 출력하는 함수
	int pass = 0;
	
	printf("Passed Vehicle\n");
	printf("Car ");
	if((passed_car = check_passed_car())!=0){
                        printf("%d\n", passed_car);
                }
	else printf("\n");

	printf("Waiting Vehicle\n");
	printf("Car ");
	for(int i = 0;i < 16; i++){
		if(all_waiting_list[i] != 0)
			printf("%d ", all_waiting_list[i]);
	}
	printf("\n===============================\n");
}
bool is_work_all_finished(void){ //모든 작업이 끝났는 지 검사하는 함수
	bool ret = true;
	for(int i = 0; i<16; i++){
		if(all_waiting_list[i] != 0)
			ret = false;
	}
	if(running_car.deg != NO_CAR)
		ret = false;

	return ret;
}
void print_result(int tick){ //모든 작업이 끝난 후 전체 결과를 출력하는 함수
	printf("Number of vehicles passed from each start point\n");
	for(int i = 0;i < 4; i++){
		printf("P%d : %d times\n", i+1, each_passed[i]);
	}
	printf("Total time : %d ticks\n", tick);
}
void way_addData(struct waiting *waiting, int data, int index){ //각 way에 대한 쓰레드의 waiting list에 값을 추가하는 함수
	struct waiting *newNode = malloc(sizeof(struct waiting));
	newNode->next = waiting->next;
	newNode->data = data;
	newNode->index = index;
	waiting->next = newNode;
}
int check_waiting_length(struct waiting *head){ //각 쓰레드의 waiting list의 길이를 구하는 함수
	int count = 0;
	struct waiting *this = malloc(sizeof(struct waiting));
	this = head;
	while(this != NULL){
		count++;
		this = this -> next;
	}
	return count;
}
int setWaylist(struct waiting *head,int random){ //각 쓰레드의 차량이 운행에 들어간다면 waiting list를 재정비 해주는 함수
	struct waiting *cur_node = malloc(sizeof(struct waiting));
	struct waiting *pre_node = malloc(sizeof(struct waiting));
	int i = 1;
	int len = check_waiting_length(head);
	int ret = 0;
	if(random==1){ 
		if(len == 1){
			ret = head->next->index;
			head->next = NULL;
		}
		else{
			ret = head->next->index;
			head->next = head->next->next;
		}
	}
	else{
		pre_node = head;
		cur_node = pre_node->next;
		while((cur_node != NULL)&&(i<random-1)){
			i++;
			pre_node = cur_node;
			if(cur_node->next == NULL){
				ret = pre_node->index;
				pre_node->next = NULL;
				break;
			}
			else cur_node = cur_node->next;
		}
		if(pre_node->next != NULL){
			ret = cur_node->index;
			pre_node->next = cur_node->next;
		}
	}
	return ret;
}
