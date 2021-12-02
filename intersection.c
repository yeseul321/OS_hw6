#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>

// #define DEBUG 1

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

typedef struct waiting_all{
        int data;
        struct waiting_all *next;
} waiting_list;

void *thread_func(void *arg);

//bool is_empty_way(int *arr);
bool is_empty_way(struct waiting *waiting);
bool is_vertical(int start);
bool is_same_dir(int startWay);

void until_finish_active(void);
void update_time(void);
int check_passed_car(void);
void check_no_car(void);
void print_situ_result(void);
bool is_work_all_finished(void);
void print_result(int tick);
void way_addData(struct waiting *waiting, int data, int index);
void all_addData(struct waiting_all *waiting, int data);
int check_waiting_length(struct waiting *waiting);
int setWaylist(struct waiting *waiting,int random);
void setAlllist(struct waiting_all *head, int index);


/*
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

typedef struct waiting_all{
	int data;
	struct waiting_all *next;
} waiting_list;
*/
int ticks = 0;
int vehicles = 0;
int *startPoint;
int *wait_queue;
int way_passed[4] = {0,};
int passed_car = 0;
//int thr1Point[15] = {0,},thr2Point[15]={0,},thr3Point[15]={0,},thr4Point[15]={0,};
int *all_waiting_list;
int *p_startPoint[4];
int *each_passed;
bool finish_active[4] = {false, false, false, false};

car running_car = {9, {{0,0,0},{0,0,0}}};
//waiting_list *all_head;
//all_head->next = NULL;

waiting_way *way_head[4];
/*
for(int i = 0;i<4;i++){
	way_head[i] = malloc(sizeof(struct waiting_way));
	way_head[i]->next = NULL;
}
*/

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int main(void){
	//all_head = malloc(sizeof(struct waiting_all));
	//all_head->next = NULL;
	each_passed = calloc(4, sizeof(int));
	for(int i = 0;i<4;i++){
        way_head[i] = malloc(sizeof(struct waiting));
        way_head[i]->next = NULL;
	all_waiting_list = malloc(sizeof(int)*16);
}
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
#ifdef DEBUG
	//startPoint = {2,3,3,4,4,3,4,2,2,2};
	for(int i = 0;i<vehicles;i++){
		printf("%d ", startPoint[i]);
	}
#else
	for(int i=0;i<vehicles;i++){
		startPoint[i] = (rand() % 4) + 1;
		printf("%d ", startPoint[i]);
	}
#endif
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
	int one=0, two=0, three=0, four=0, tick = 0;
	while(1){   //-> until ticks ends
		sleep(1);
		printf("tick %d\n", tick);
		printf("===============================\n");

		if(tick<vehicles){
				if(startPoint[tick] == 1){
					way_addData(way_head[0],1,tick);
					all_waiting_list[tick] = 1;
					one++;
				}
				else if(startPoint[tick] == 2){
					way_addData(way_head[1],2,tick);
					all_waiting_list[tick] = 2;
					two++;
                                }
				else if(startPoint[tick] == 3){
					way_addData(way_head[2],3,tick);
					all_waiting_list[tick] = 3;
					three++;
                                }
				else if(startPoint[tick] == 4){
					way_addData(way_head[3],4,tick);
					all_waiting_list[tick] = 4;
					four++;
                                }
			
		}
		pthread_cond_broadcast(&cond);
		until_finish_active();
		update_time();
		print_situ_result();
		printf("===============================\n");
		check_no_car();

		if(tick>=vehicles){
			if(is_work_all_finished() == true){
				sleep(1);
				printf("tick : %d\n", ++tick);
                		printf("===============================\n");
				print_situ_result();
				break;
			}
		}
		tick++;
	}
	printf("===============================\n");
	print_result(tick);

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

		//printf("i am %d\n", startWay);

		if(!is_empty_way(way_head[startWay-1]) && !is_vertical(startWay))
		{
			waiting_length = check_waiting_length(way_head[startWay-1]);
			//EMPTY ROAD
			if(running_car.deg == NO_CAR)
			{
				running_car.deg = startWay % 2;
				running_car.stat[0].dir = startWay;
				//대기큐 랜덤 뽑기해서 넣기
				random = rand()%(waiting_length-1)+1;
				running_car.stat[0].way = startWay;

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
			  }
			}
		}
		else{//lock을 얻을 조건이 안될 때
		}

		finish_active[startWay-1] = true;
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

bool is_empty_way(struct waiting *way_waiting){
	if(way_waiting->next == NULL){
		return true;
	}
	else return false;
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
			//printf("all finish\n");
			break;
		}
	}
}

void update_time(void){
	for(int i = 0;i < 2;i++){
		if(running_car.stat[i].way != 0){
			running_car.stat[i].time++;
		}
	}
	return;
}

int check_passed_car(void){
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
void check_no_car(void){
	if((running_car.stat[0].dir == 0) && (running_car.stat[1].dir == 0))
		running_car.deg = NO_CAR;
}
void print_situ_result(void){
	int pass = 0;
	struct waiting_all *this = malloc(sizeof(struct waiting_all));
        //this = all_head->next;
	
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
	printf("\n");
}
bool is_work_all_finished(void){
	bool ret = true;
	for(int i = 0; i<16; i++){
		if(all_waiting_list[i] != 0)
			ret = false;
	}
	if(running_car.deg != NO_CAR)
		ret = false;

	return ret;
}
void print_result(int tick){
	printf("Number of vehicles passed from each start point\n");
	for(int i = 0;i < 4; i++){
		printf("P%d : %d times\n", i+1, each_passed[i]);
	}
	printf("Total time : %d ticks\n", tick);
}
void way_addData(struct waiting *waiting, int data, int index){
	struct waiting *newNode = malloc(sizeof(struct waiting));
	newNode->next = waiting->next;
	newNode->data = data;
	newNode->index = index;

	waiting->next = newNode;
}
void all_addData(struct waiting_all *waiting, int data){
	struct waiting_all *newNode = malloc(sizeof(struct waiting_all));
	struct waiting_all *this = malloc(sizeof(struct waiting_all));
	this = waiting;
	newNode->next = NULL;
	if(waiting->next = NULL) {
		waiting->next = newNode;
		newNode->data = data;
	}
	else{
		while(this->next != NULL){
                this = this -> next;
		}
		this->next = newNode;
		newNode->data = data;
        }

        //newNode->next = waiting->next;
        //newNode->data = data;

        //waiting->next = newNode;
}
int check_waiting_length(struct waiting *head){
	int count = 0;
	struct waiting *this = malloc(sizeof(struct waiting));
	this = head;
	while(this != NULL){
		count++;
		this = this -> next;
	}
	return count;
}
int setWaylist(struct waiting *head,int random){
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

	
void setAlllist(struct waiting_all *head, int index){
	struct waiting_all *cur_node = malloc(sizeof(struct waiting_all));
        struct waiting_all *pre_node = malloc(sizeof(struct waiting_all));
        int i = 1;
        if(index==1) head->next = NULL;
        else{
                pre_node = head;
                cur_node = pre_node->next;
                while((cur_node != NULL)&&(i<index-1)){
                        i++;
                        pre_node = cur_node;
                        if(cur_node->next == NULL){
                                pre_node->next = NULL;
                                break;
                        }
                        else cur_node = cur_node->next;
                }
                if(pre_node->next != NULL){
                        pre_node->next = cur_node->next;
                }
        }

}
