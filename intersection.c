#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

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
	//distribute startPoint to threads per 1 second
	int one=0, two=0, three=0, four=0, i=0;
	while(1){   //-> until ticks ends
		sleep(1);
		if(i<vehicles){
			switch(startPoint[i]){
				case 1:
					thr1Point[one] = startPoint[i];
					printf("%d\n", thr1Point[one]);
					one++;
					break;
				case 2:
					thr2Point[two] = startPoint[i];
					printf("%d\n", thr2Point[two]);
					two++;
                                        break;
				case 3:
                                        thr3Point[three] = startPoint[i];
					printf("%d\n", thr3Point[three]);
					three++;
                                        break;
                                case 4:
                                        thr4Point[four] = startPoint[i];
					printf("%d\n", thr4Point[four]);
					four++;
                                        break;
			}
			i++;
		}
		else break;
	}

	return 0;
}
