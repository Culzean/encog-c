#include "encog.h"
#include <string.h>

char parsedOption[MAX_STR];
char parsedArgument[MAX_STR];

void Usage() {
	puts("\nUsage:\n");
	puts("encog xor");
	puts("encog benchmark");
	puts("encog train [egb file]");
	puts("encog egb2csv [egb file] [csv file]");
	puts("encog csv2egb [csv file] [egb file]");
	puts("");
	puts("Options:");
	puts("");
	puts("/input:## The number of inputs.");
	puts("/ideal:## The number of ideals.");
	puts("/records:## The number of ideals.");
	puts("/iterations:## The number of ideals.");
	puts("/threads:## The number of threads.");
};

void ParseOption(char *str)
{
	char *ptr;

	if( *str=='/' || *str=='-' ) {
		str++;
	}

	ptr = strchr(str,':');

	if( ptr!=NULL ) {
		strncpy(parsedOption,str,MIN(MAX_STR,ptr-str));
		strncpy(parsedArgument,ptr+1,MAX_STR);
	} else {
		strncpy(parsedOption,str,MAX_STR);
		*parsedArgument = 0;
	}
}

void RunBenchmark(INT inputCount, INT idealCount, INT records, INT iterations ) {
	ENCOG_DATA *data;
	ENCOG_NEURAL_NETWORK *net;
	ENCOG_TRAIN_PSO *pso;
	INT i;
	time_t startTime;
	time_t endTime;
	int elapsed;

	printf("\nPerforming benchmark\n");
	printf("Input Count: %i\n",inputCount);
	printf("Ideal Count: %i\n",idealCount);
	printf("Records: %i\n",records);
	printf("Iterations: %i\n",iterations);

	data = EncogDataGenerateRandom(inputCount,idealCount,records,-1,1);

	net = EncogNetworkNew();
    EncogNetworkAddLayer(net,2,&EncogActivationTANH,inputCount);
    EncogNetworkAddLayer(net,3,&EncogActivationTANH,50);
    EncogNetworkAddLayer(net,1,&EncogActivationTANH,idealCount);
    EncogNetworkFinalizeStructure(net);

	EncogNetworkRandomizeRange(net,-1,1);

	pso = EncogTrainPSONew(30, net, data);

	startTime = time(NULL);

	for(i=0;i<iterations;i++) {
		EncogTrainPSOIterate(pso);
	}

	endTime = time(NULL);
	
	elapsed = (int)(endTime - startTime);

	printf("Benchmark time(seconds): %i\n",elapsed);
}

void XORTest() {
	    /* local variables */
    char line[MAX_STR];
    int i, iteration;
    REAL *input,*ideal;
    REAL output[1];
    float error;
    ENCOG_DATA *data;
    ENCOG_NEURAL_NETWORK *net;
    ENCOG_TRAIN_PSO *pso;

/* Load the data for XOR */
    data = EncogDataCreate(2, 1, 4);
    EncogDataAdd(data,"0,0,  0");
    EncogDataAdd(data,"1,0,  1");
    EncogDataAdd(data,"0,1,  1");
    EncogDataAdd(data,"1,1,  0");

/* Create a 3 layer neural network, with sigmoid transfer functions and bias */

    net = EncogNetworkNew();
    EncogNetworkAddLayer(net,2,&EncogActivationSigmoid,1);
    EncogNetworkAddLayer(net,3,&EncogActivationSigmoid,1);
    EncogNetworkAddLayer(net,1,&EncogActivationSigmoid,0);
    EncogNetworkFinalizeStructure(net);

/* Randomize the neural network weights */
    EncogNetworkRandomizeRange(net,-1,1);

/* Create a PSO trainer */
    pso = EncogTrainPSONew(30, net, data);

/* Begin training, report progress. */
    iteration = 1;
    do
    {
        error = EncogTrainPSOIterate(pso);
        *line = 0;
        EncogStrCatStr(line,"Iteration #",MAX_STR);
        EncogStrCatInt(line,iteration,MAX_STR);
        EncogStrCatStr(line,", Error: ",MAX_STR);
        EncogStrCatDouble(line,error,4,MAX_STR);
        puts(line);
        iteration++;
    } while(error>0.01);

/* Pull the best neural network that the PSO found */
    EncogTrainPSOImportBest(pso,net);
    EncogTrainPSODelete(pso);

/* Display the results from the neural network, see if it learned anything */
    printf("\nResults:\n");
    for(i=0; i<4; i++)
    {
        input = EncogDataGetInput(data,i);
        ideal = EncogDataGetIdeal(data,i);
        EncogNetworkCompute(net,input,output);
        *line = 0;
        EncogStrCatStr(line,"[",MAX_STR);
        EncogStrCatDouble(line,input[0],8,MAX_STR);
        EncogStrCatStr(line," ",MAX_STR);
        EncogStrCatDouble(line,input[1],8,MAX_STR);
        EncogStrCatStr(line,"] = ",MAX_STR);
        EncogStrCatDouble(line,output[0],8,MAX_STR);
        puts(line);
    }

/* Obtain the SSE error, display it */
    error = EncogErrorSSE(net, data);
    *line = 0;
    EncogStrCatStr(line,"Error: ",MAX_STR);
    EncogStrCatDouble(line,(double)error,4,MAX_STR);
    puts(line);

/* Delete the neural network */
    EncogNetworkDelete(net);

}

void train(char *egbFile) {
	ENCOG_DATA *data;
	ENCOG_NEURAL_NETWORK *net;
	ENCOG_TRAIN_PSO *pso;
	INT iteration;
	float error;
	char line[MAX_STR];
	INT lastUpdate;

	data = EncogDataEGBLoad(egbFile);

	printf("Training\n");
	printf("Input Count: %i\n", data->inputCount);
	printf("Ideal Count: %i\n", data->idealCount);
	printf("Record Count: %ld\n", data->recordCount);	    

/* Create a 3 layer neural network, with sigmoid transfer functions and bias */

    net = EncogNetworkNew();
	EncogNetworkAddLayer(net,data->inputCount,&EncogActivationTANH,1);
    EncogNetworkAddLayer(net,6,&EncogActivationTANH,1);
	EncogNetworkAddLayer(net,data->idealCount,&EncogActivationTANH,0);
    EncogNetworkFinalizeStructure(net);

/* Randomize the neural network weights */
    EncogNetworkRandomizeRange(net,-1,1);

/* Create a PSO trainer */
    pso = EncogTrainPSONew(30, net, data);

/* Begin training, report progress. */
    iteration = 1;
	lastUpdate = 0;
    do
    {
        error = EncogTrainPSOIterate(pso);
		lastUpdate++;
		if( lastUpdate>=100 )
		{
			lastUpdate = 0;
        *line = 0;
        EncogStrCatStr(line,"Iteration #",MAX_STR);
        EncogStrCatInt(line,iteration,MAX_STR);
        EncogStrCatStr(line,", Error: ",MAX_STR);
        EncogStrCatDouble(line,error,4,MAX_STR);
        puts(line);
		}
        iteration++;
    } while(error>0.01);

/* Pull the best neural network that the PSO found */
    EncogTrainPSOImportBest(pso,net);
    EncogTrainPSODelete(pso);
}

void EGB2CSV(char *egbFile, char *csvFile) 
{
	ENCOG_DATA *data;

	data = EncogDataEGBLoad(egbFile);

	printf("Training\n");
	printf("Input Count: %i\n", data->inputCount);
	printf("Ideal Count: %i\n", data->idealCount);
	printf("Record Count: %ld\n", data->recordCount);
	printf("Source File: %s\n", egbFile );
	printf("Target File: %s\n", csvFile );

	EncogDataCSVSave(csvFile,data,10);
	EncogDataDelete(data);

	printf("Conversion done.\n");
}

void CSV2EGB(char *csvFile, char *egbFile, int inputCount, int idealCount) 
{
	ENCOG_DATA *data;

	data = EncogDataCSVLoad(csvFile, inputCount, idealCount);

	printf("Training\n");
	printf("Input Count: %i\n", data->inputCount);
	printf("Ideal Count: %i\n", data->idealCount);
	printf("Record Count: %ld\n", data->recordCount);
	printf("Source File: %s\n", csvFile );
	printf("Target File: %s\n", egbFile );

	EncogDataEGBSave(egbFile,data);
	EncogDataDelete(data);

	printf("Conversion done.\n");
}

int main(int argc, char* argv[])
{
	INT i;
	INT inputCount = 10;
	INT idealCount = 1;
	INT records = 10000;
	INT iterations = 100;
	INT threads = 0;
	INT phase = 0;
	char command[MAX_STR];
	char arg1[MAX_STR];
	char arg2[MAX_STR];

	printf("Encog C/C++ Command Line v0.1\n");
	printf("Running in: %i bit mode\n", (int)(sizeof(void*)*8));

	for(i=1;i<(INT)argc;i++) {
		if( *argv[i]=='/' || *argv[i]=='-' )
		{
			ParseOption(argv[i]);
			if( !EncogUtilStrcmpi(parsedOption,"INPUT") ) {
				inputCount = atoi(parsedArgument);
			} else if( !EncogUtilStrcmpi(parsedOption,"IDEAL") ) {
				idealCount = atoi(parsedArgument);
			} else if( !EncogUtilStrcmpi(parsedOption,"RECORDS") ) {
				records = atoi(parsedArgument);
			} else if( !EncogUtilStrcmpi(parsedOption,"ITERATIONS") ) {
				iterations = atoi(parsedArgument);
			} else if( !EncogUtilStrcmpi(parsedOption,"THREADS") ) {
				threads = atoi(parsedArgument);
				omp_set_num_threads(threads);
			}
			
		}
		else 
		{
			if( phase==0 ) {
				strncpy(command,argv[i],MAX_STR);
				EncogUtilStrlwr(command);
			} else if( phase==1 ) {
				strncpy(arg1,argv[i],MAX_STR);
			} else if( phase==2 ) {
				strncpy(arg2,argv[i],MAX_STR);
			}

			phase++;
		}
	}
	
	EncogUtilInitRandom();
	
	if(!EncogUtilStrcmpi(command,"xor") ) {
		XORTest();
	} else if (!EncogUtilStrcmpi(command,"benchmark") ) {
		RunBenchmark(inputCount,idealCount,records,iterations );
	} else if (!EncogUtilStrcmpi(command,"train") ) {
		train(arg1);
	} else if (!EncogUtilStrcmpi(command,"egb2csv") ) {
		EGB2CSV(arg1,arg2);
	} else if (!EncogUtilStrcmpi(command,"csv2egb") ) {
		CSV2EGB(arg1,arg2,inputCount,idealCount);
	} else {
		Usage();
	}


    return 0;
}
