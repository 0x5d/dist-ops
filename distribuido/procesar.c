#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "mpidefs.hpp"

using namespace std;

 int P1;
 int P2;
 char OP;


void procesarText(char *text, int* p1, int* p2, char* op) {
	char sp1[100];
	char sp2[100];

	int i=1;

	while (text[i]!='=') {
	 	if (text[i]=='+' || text[i]=='-' || text[i]=='*' || text[i]=='/') {
		    *op = text[i];
		    strncpy(sp1, text, i);
		    strncpy(sp2, text+i+1, strlen(text)-1);
		    break;
 		} 
 		else {
	    	i++;
	    }
	}

	*p1 = atoi(sp1);
	*p2 = atoi(sp2);

//printf("%d %c %d\n", *p1, *op, *p2);
}

int operacion(int p1, char op, int p2) {
int result=-1;
if (op=='+')
	result = p1+p2;
else if (op=='-')
	result = p1-p2;
else if (op=='*')
	result = p1*p2;
else if (op=='/')
	result = p1+p2;
return result;
}
 
int main(int argc, char* argv[])
{

	MPI_Status status;

	int RC;

	int TASK_ID, NUM_PROCS, NUM_WORKERS;

	if((RC = MPI_Init(&argc, &argv)) != MPI_SUCCESS){
        fprintf(stderr, "Cannot initialize MPI_Init. Exiting.\n");
        MPI_Abort(MPI_COMM_WORLD, RC);
        exit(1);
    }

    RC = MPI_Comm_rank(MPI_COMM_WORLD, &TASK_ID);
    RC = MPI_Comm_size(MPI_COMM_WORLD, &NUM_PROCS);

    int LEN;
    char NAME[MPI_MAX_PROCESSOR_NAME];
	MPI_Get_processor_name(NAME, &LEN);
    NUM_WORKERS = NUM_PROCS - 1;
	printf("Hello! I'm process %i out of %i processes, on host %s\n.",
           TASK_ID, NUM_PROCS, NAME);

 
 	if(TASK_ID == MASTER){
		char linea_out[100];
	 	FILE *archivo_in;
		FILE *archivo_out;
	 
	 	archivo_in = fopen("data_in.txt","r");
		archivo_out = fopen("datos_out.txt","w");
	 
	 	if (archivo_in == NULL){
	 		exit(1);
	 	}
	 	if(archivo_out == NULL){
	 		exit(2);
	 	}

	 	vector<char*> exps;
	 	while (feof(archivo_in) == 0)
 		{
	 		char* linea_in = new char[100];
	 		fgets(linea_in, 100, archivo_in);
	 		exps.push_back(linea_in);
 		}
	 	int numExpr = 1;
	 	int procID = 1;
 		for (int i = 0; i < exps.size() - 1; i++){
	 		int* params = new int[2];
	 		char op;
 			procesarText(exps[i], &params[0], &params[1], &op);
	 		//printf("%d %d\n", params[0], params[1]);
	 		int numExps = exps.size() - 1;
	 		MPI_Send(&numExps, 1, MPI_INT, procID, FROM_MASTER_N, MPI_COMM_WORLD);
	 		MPI_Send(&params[0], 2, MPI_INT, procID, FROM_MASTER_P, MPI_COMM_WORLD);
	 		MPI_Send(&op, 1, MPI_CHAR, procID, FROM_MASTER_OP, MPI_COMM_WORLD);

	 		numExpr++;
	 		procID = numExpr % NUM_WORKERS + 1;
			
 		}
 		for(numExpr- 1; numExpr > 0; numExpr--){
    		int res;
            MPI_Recv(&res, 1, MPI_INT, MPI_ANY_SOURCE, FROM_WORKER, MPI_COMM_WORLD, &status);
			sprintf(linea_out,"%d%c%d=%d\n",P1, OP, P2,res);
			fputs(linea_out, archivo_out);
 		}

        fclose(archivo_in);
		fclose(archivo_out);

 	}
 	else {
 		int numExprs;
 		MPI_Recv(&numExprs, 1, MPI_INT, MASTER, FROM_MASTER_N, MPI_COMM_WORLD, &status);
 		for(int i = TASK_ID; i < numExprs; i += NUM_PROCS){
	 		int* params = new int[2];
	 		MPI_Recv(&params[0], 2, MPI_INT, MASTER, FROM_MASTER_P, MPI_COMM_WORLD, &status);
	 		char op;
	 		MPI_Recv(&op, 1, MPI_CHAR, MASTER, FROM_MASTER_OP, MPI_COMM_WORLD, &status);
	 		//printf("%d%c%d\n", params[0], op, params[1]);
	 		int res = operacion(params[0], op, params[1]);
	 		MPI_Send(&res, 1, MPI_INT, TASK_ID, FROM_WORKER, MPI_COMM_WORLD);
 		}
 		
 	}
	
    RC = MPI_Finalize();
    exit(0);
}