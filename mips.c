/* 
Shrivinayak Bhat
UFID: 47971819
University of Florida


On my honor, I have neither given nor received unauthorized aid on this 
assignment 

*/

//Simplified MIPS Simulator

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int registers[16];
int regchanged[16];

struct buffer{
	char Op[4];
	int D,F,S;
	int flag;
	struct buffer* next;
};

struct memory{
	int addr,val;
	struct memory* next;
}*DAM;

struct buffer *INM=NULL,*INB=NULL,*AIB=NULL,*SIB=NULL,*PRB=NULL,*ADB=NULL,*REB=NULL,*RGF=NULL;


void buffPrint(FILE *fp,struct buffer * buff){
	while(buff){
		if(buff->flag==0 && strcmp(buff->Op,"ST")==0)
			fprintf(fp,"<%s,R%d,R%d,%d>",buff->Op,buff->D,buff->F,buff->S);
		else if(buff->flag==0)
			fprintf(fp,"<%s,R%d,R%d,R%d>",buff->Op,buff->D,buff->F,buff->S);
		else if(buff->flag==1)
			fprintf(fp,"<%s,R%d,%d,%d>",buff->Op,buff->D,buff->F,buff->S);
		else if(buff->flag==2)
			fprintf(fp,"<R%d,%d>",buff->D,buff->F);
		buff = buff->next;
		if(buff)
			fprintf(fp,",");
	}
}

void printHelper(FILE* fp){
	static int step = 0;

	fprintf(fp,"STEP %d:\n", step++);

	fprintf(fp,"INM:");
	buffPrint(fp,INM);
	fprintf(fp,"\n");

	fprintf(fp,"INB:");
	buffPrint(fp,INB);
	fprintf(fp,"\n");

	fprintf(fp,"AIB:");
	buffPrint(fp,AIB);
	fprintf(fp,"\n");

	fprintf(fp,"SIB:");
	buffPrint(fp,SIB);
	fprintf(fp,"\n");

	fprintf(fp,"PRB:");
	buffPrint(fp,PRB);
	fprintf(fp,"\n");

	fprintf(fp,"ADB:");
	buffPrint(fp,ADB);
	fprintf(fp,"\n");

	fprintf(fp,"REB:");
	buffPrint(fp,REB);
	fprintf(fp,"\n");

	fprintf(fp,"RGF:");
	for(int i=0;i<16;i++){
		if(regchanged[i]){
			if(i!=0)
			fprintf(fp,",");
			fprintf(fp,"<R%d,%d>",i,registers[i]);
		}
	}
	fprintf(fp,"\n");
	fprintf(fp,"DAM:");
	struct memory * iter=DAM;
	while(iter){
		fprintf(fp,"<%d,%d>",iter->addr,iter->val);
		if(iter->next!=NULL)
			fprintf(fp,",");
		iter = iter->next;
	}

	fprintf(fp,"\n");
	fprintf(fp,"\n");
}

struct buffer* pop(struct buffer* first){
	struct buffer* temp = first;
	first=first->next;
	temp->next = NULL;
	return temp;
}


struct buffer* push(struct buffer* first, struct buffer* temp){
	if(first==NULL)
		return temp;

	struct buffer* iter = first;
	while(iter->next!=NULL)
		iter=iter->next;

	iter->next=temp;
	temp->next = NULL;
	return first;
}




void errorFunc(char *err){
	printf("Error: %s\n",err );
	exit(1);
}


struct buffer* makeNode(char* string){
	//allocating new token
	struct buffer * temp;
	char* temp2;
	temp = (struct buffer *) malloc(sizeof(struct buffer));
	
	//extract opcode
	strcpy(temp->Op,strtok(&string[1],","));
	//extract Destination Reg
	temp2= strtok(NULL,",");
	temp->D = atoi(&temp2[1]);
	//extract First Operand
	temp2= strtok(NULL,",");
	temp->F = atoi(&temp2[1]);
	//extract Second Operand
	temp2= strtok(NULL,">");
	if(temp2[0]=='R')
		temp->S = atoi(&temp2[1]);
	else
		temp->S = atoi(temp2);
	temp->flag = 0;
	temp->next = NULL;
}



struct memory* store(struct memory * in,int addr,int val){

	struct memory *new = (struct memory *)malloc(sizeof(struct memory));
	new->addr = addr;
	new->val = val;
	new->next = NULL;
	struct memory *iter=in,*last=NULL;

	if(in==NULL)
		return new;
	else{
		while(iter){
			if(iter->addr == new->addr){
				iter->val = new->val;
				free(new);
				break;
			}
			if(iter->addr > addr){
				new->next = iter;
				if(last==NULL){
					new->next = iter;
					in = new;
					break;
				}
				last->next = new;
				break;
			}

			if(iter->next == NULL){
				iter->next = new;
				break;
			}
			last = iter;
			iter = iter->next;
		}
	}
	
	return in;
}

void loadReg(){
	FILE *fp;
	char buff[100];
	if(!(fp = fopen("registers.txt","r")))
		errorFunc("Error: Unable to open registers.txt");
	char* token;
	int regno;
	while(1){
		if(!fgets(buff,100,fp))
			break;
		token = strtok(buff,"R");
		regno = atoi(strtok(NULL,","));
		registers[regno] = atoi(strtok(NULL,">"));
		regchanged[regno] = 1;

	}
	fclose(fp);
}

void loadMem(){

	FILE *fp;
	char buff[100];
	if(!(fp = fopen("datamemory.txt","r")))
		errorFunc("Error: Unable to open datamemory.txt");
	char* token;
	int regno;
	int addr,val;
	while(1){
		if(!fgets(buff,100,fp))
			break;
		addr = atoi(strtok(&buff[1],","));
		val = atoi(strtok(NULL,">"));
		DAM = (struct memory *)store(DAM,addr,val);
	}

}

struct buffer* decoder(struct buffer* INB,struct buffer* temp){
	temp->F = registers[temp->F];
	temp->flag = 1;

	if(strcmp(temp->Op,"ST")!=0)
		temp->S = registers[temp->S];

	INB = push(INB,temp);
	
	return INB;
}

struct buffer* issue1(struct buffer * in, struct buffer* temp){
	if(strcmp(temp->Op,"ST")!=0){
		in = push(in,temp);	
	
	}
	return in;
}


struct buffer* issue2(struct buffer * in, struct buffer* temp){
	if(strcmp(temp->Op,"ST")==0){
		in = push(in,temp);
	}
	return in;
	
}

struct buffer* asu(struct buffer * in,struct buffer* temp){
	if(strcmp(temp->Op,"MUL")!=0){
		
		temp->flag = 2;
		if(strcmp(temp->Op,"ADD")==0)
			temp->F = temp->F+temp->S;
		else if(strcmp(temp->Op,"SUB")==0)
			temp->F = temp->F-temp->S;
		else
			errorFunc(strcat(temp->Op,"Invalid Opcode at ASU"));

		in = push(in,temp);
	}
	
	return in;

}

struct buffer* mlu1(struct buffer * in,struct buffer* temp){

	if(strcmp(temp->Op,"MUL")==0){
		in = push(in,temp);
	}
	
	return in;

}

struct buffer* mlu2(struct buffer * in,struct buffer* temp){

	temp->flag = 2;
	
	temp->F = temp->F*temp->S;
	
	in = push(in,temp);
	
	return in;
}


struct buffer* addr(struct buffer * in,struct buffer* temp){
	temp->flag = 2;
	temp->F = temp->F+temp->S;
	
	in = push(in,temp);
	
	return in;
}



struct buffer* cwrite(struct buffer * in,struct buffer* temp){

	
	registers[temp->D] = temp->F;
	regchanged[temp->D] = 1;
	
	return in;
}

void main(){

	//create pointers to all the buffers
	FILE *fps = fopen ("simulation.txt","w");
	struct buffer* temp=NULL,*temp2;

	int flag=1;
	// load register values from the file.
	loadReg(); 
	loadMem();
	
	//read from instructions.txt
	FILE *fp;
	char buff[100];
	if(!(fp = fopen("instructions.txt","r")))
		errorFunc("Error: Unable to open instructions.txt");
	
	for(int i=0;i<16;i++){
		//load the first 16 instruction tokens in INM buffer
		if(!fgets(buff,100,fp)){
			flag = 0;
			break;
		}

		//push new token
		INM = push(INM,makeNode(buff));
	}
	
	printHelper(fps);
	while(1){
		if(INM==NULL && INB==NULL && AIB==NULL && PRB==NULL && SIB==NULL && REB==NULL && ADB==NULL){

			break;
		}
		if(REB){
			temp = REB;
			REB = REB->next;
			temp->next=NULL;
		
			RGF = cwrite(RGF,temp);
		}

		if(ADB)
		{	
			temp = ADB;
			ADB = ADB->next;
			temp->next=NULL;

			DAM = (struct memory *)store(DAM,temp->F,registers[temp->D]);
			free(temp);
		}			

		
		if(SIB){
			temp2 = SIB;
			SIB = SIB->next;
			temp2->next = NULL;
			ADB = addr(ADB,temp2);
		}


		

		if(PRB){
			temp = PRB;
			PRB = PRB->next;
			temp->next = NULL;
			REB = mlu2(REB,temp);
		}
		

		if(AIB){
			temp = AIB;
			AIB = AIB->next;
			temp->next = NULL;
			PRB = mlu1(PRB,temp);
			REB = asu(REB,temp);

		}



		if(INB){
			temp = INB;
			INB = INB->next;
			temp->next=NULL;

			SIB = issue2(SIB,temp);
			AIB = issue1(AIB,temp);
		}
		if(INM){
			temp = INM;
			INM = INM->next;
			temp->next = NULL;
			INB = decoder(INB,temp);
		}


		
		printHelper(fps);

		
		if(flag){
			if(!fgets(buff,100,fp))
				flag = 0;
			else
				INM = push(INM,makeNode(buff)); 				//push new token
		}

	}
}

