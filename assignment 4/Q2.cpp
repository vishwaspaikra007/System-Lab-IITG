#include<bits/stdc++.h>
#include<stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include<unistd.h>

#define MSGSZ 50

using namespace std;

map<int, string> gradeDis; //table for storing the grade distribution
typedef struct msgbuf1{
    long mtype;
    char mtext[MSGSZ];
} message_buf;

typedef struct ansBuff{
	long	pid;
    char ansResponse[MSGSZ];
} ansBuff;

typedef struct quesBuff{
    long qType;
	long qID;
    char qText[MSGSZ];
} quesBuff;

void grade(int nQues, string ansKey, ansBuff ans[]) //function to calculate and display the grade
{
	int total = 0;
	printf("******Grading******\n100 to 90 - A\n89 to 80 - B\n79 to 70 - C\n69 to 30 - D\nLess than 30 - FF\n*******************\n");
	printf("ans key :%s\n", ansKey.c_str());
	for (int i = 0; i < nQues; i++) //evaluating the answers
	{
		if(ans[i].ansResponse[0] == ansKey[i]) total++;
	}
	cout << "total : " << total << endl;
	float percentage = ((float)total/nQues)*100;
	cout << "percentage: "<<percentage << endl;
	if(percentage >= 50)		
	{
		printf("grade : A\n");
		gradeDis[ans[0].pid] = "A";
	}
	else if(percentage >= 40) 	
	{
		printf("grade : B\n");
		gradeDis[ans[0].pid] = "B";
	}
	else if(percentage >= 30)	
	{
		printf("grade : C\n");
		gradeDis[ans[0].pid] = "C";
	}
	else if(percentage >= 25)	
	{
		printf("grade : D\n");
		gradeDis[ans[0].pid] = "D";
	}
	else						
	{
		printf("grade : FF\n");
		gradeDis[ans[0].pid] = "FF";
	}
	return;
}

int main()
{
	srand(0);
	unsigned int seed = time(0); 
	int msqid1, msqid2;
    key_t key = 133; //129 		//unique token for every message queue
	key_t key2 = 132; //128		
	int noOfStudents, noOfQues;
	message_buf sbuf, sbuf2, rbuf, rbuf2;	//send and read buffers
	
	cout << "enter the number of students: ";
	cin >> noOfStudents;
	cout << "\nenter the number of questions: ";
	cin >> noOfQues;
	cout << "\nenter the answer keys: ";
	string ansKey = "";
	// scanf("%s", ansKey);
	for(int i = 0; i < noOfQues; i++) //random answer key 
	{
		ansKey += ((rand() % 4  + 1) + 48);
	}
	printf("anskey: %s\n", ansKey.c_str());
	quesBuff qSBuf[noOfQues], qRBuf[noOfQues]; //send and read buffers
	ansBuff aSBuff[noOfQues], aRBuff[noOfQues]; //send and read buffers
	char q[MSGSZ]; //index for each question
	for(int i = 0; i < noOfQues; i++) //creating questions as per user requirement
	{
		strcpy(qSBuf[i].qText, "Question ");
		int index = i+1;
		sprintf(q, "%d", index); 
		strcat(qSBuf[i].qText, q);
	}

	if((msqid1 = msgget(key2, IPC_CREAT|0666)) < 0) // create message queue and get the unique id for sending to child process 
	{
    	perror("msgget");
    	exit(1);
    }

	if(msgsnd(msqid1, &qSBuf, 2*MSGSZ*noOfQues, IPC_NOWAIT) < 0) //sending the questions to child process
	{
		perror("msgsnd123");
		exit(1);
	}
	
	for(int i = 0; i < noOfStudents; i++)
	{
		if((msqid2 = msgget(key, IPC_CREAT|0667)) < 0) // create message queue for child to send answers to parent
		{
			perror("msgget");
			exit(1);
		}
		pid_t cPid = fork();	// fork a child process
		if(cPid == -1) 		// if child process failed to get created
		{
			perror("fork");
			exit(EXIT_FAILURE);
		}	
		if(cPid == 0)
		{
			srand(getpid());
			cout <<"\n";
			if(msgrcv(msqid1, &qRBuf, 2*MSGSZ*noOfQues, 0, 0) < 0) 
				{
					perror("msgrcv");
					exit(1);
				}
			// for(int i = 0; i < noOfQues; i++)
			// {	
				// printf("question %d received from parent :%s\n", i+1, qRBuf[i].qText);
			// }
			printf("questions received from parent\n");

			msgctl(msqid1, IPC_RMID, NULL); //empty the message queue after receiving questions from parent
			int temp1; //random choice from 1 to 4
			for (int i = 0; i < noOfQues; i++) //creating the random answers
			{
				aSBuff[i].pid = getpid();
				temp1 = ((rand()) % 4 ) + 1;
				aSBuff[i].ansResponse[0] = temp1 + 48;
				aSBuff[i].ansResponse[1] = '\0';
			}

			if(msgsnd(msqid2, &aSBuff, 2*noOfQues*MSGSZ, IPC_NOWAIT) < 0) // sending the random answers to parent
			{
				perror("msgsndfromchild");
				exit(1);
			}
			cout << "\n";
			exit(0);
		}
		else 
		{
			//sleep(1);
			if(msgrcv(msqid2, &aRBuff, 2*noOfQues*MSGSZ, 0, 0) < 0) //parent receiving the ans in aRBuff
			{
				perror("msgrcv111");
				exit(1);
			}

			printf("Answers received from child pID %ld \n", aRBuff[0].pid);
			for (int i = 0; i < noOfQues; i++)
			{
				printf("ques %d ans : %s\n", i+1, aRBuff[i].ansResponse);
			}
			grade(noOfQues, ansKey, aRBuff); 
			
			msgctl(msqid2, IPC_RMID, NULL); // destroys the message queue
			
			if((msqid1 = msgget(key2, IPC_CREAT|0666)) < 0) // send the question to the next child 
			{
        		perror("msgget");
        		exit(1);
    		}
			if(msgsnd(msqid1, &qSBuf, 2*MSGSZ*noOfQues, IPC_NOWAIT) < 0) 
			{
				perror("msgsndfromparent");
				exit(1);
			}
		}
	}
	msgctl(msqid1, IPC_RMID, NULL); //destroying the message queue when done
	map<int, string>::iterator it = gradeDis.begin();
	printf("\nGrade Distribution\nChild ID\n");
	while (it != gradeDis.end())
  	{
    	cout<< it->first << "\t\t" << it->second << endl;
    	++it;
  	}
	return 0;
}
