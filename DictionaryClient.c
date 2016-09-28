#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "dictionary.h"

#define FAIL  0
#define SUCCESS 1
#define INSERT_OP 2
#define SEARCH_OP 3
#define DELETE_OP 4
#define CONFIRM_DELETE_OP 5
#define CHUNK 50
#define CLIENT_ID 1
#define INVALID -1
#define PENDING -2


CLIENT *cl1, *cl2;
int is_stdin_empty = 0;
int timestamp[3];
int result_flag ( dict_data *result_data , char* operation);

struct timeval tvl;
//custom function to flush the stdin stream
int flushBuffer()                                                   
{	
	char ch;
	if(!is_stdin_empty)
		while ((ch = getchar()) != '\n' && ch != EOF);
	is_stdin_empty = 1;
}

void setTimeStamp()
{
	printf("Enter the time vector:\n");
	for(int i=0; i<3; i++)
	{
		printf("\n Value for Client %d:", i+1);
		if(!scanf("%d", timestamp + i))
		{
			printf("\nPlease Enter a Valid value!!!\n");
			i--;
		}
		printf("%d\n", timestamp[i]);
		is_stdin_empty = 0;
		flushBuffer(); 
		
	}
}

//read input into variable length strings
char* readinput()
{
   
   char* input = NULL;
   char tempbuf[CHUNK];
   size_t inputlen = 0, templen = 0;
   size_t n;
   int i =0;
	input = (char *)malloc(sizeof(char *));
	*input = '\0';
	char ch;
	flushBuffer();
   do {
       is_stdin_empty = (fgets(tempbuf, CHUNK, stdin) != NULL) ? 1:0;
       templen = strlen(tempbuf)-1;
       tempbuf[templen] = '\0';
       inputlen += templen;
       input = (char *)realloc(input, inputlen+1);
       strcat(input, tempbuf);
       
    } while (templen==CHUNK-1 && tempbuf[CHUNK-2]!='\n');

    return input;
}       


//converts to lower cases
char* verifyAndToSmall(char* inp_string, int str_length)
{
	//verify a valid string of alphabets
	for(int i = 0; i <str_length; i++)
	{
		if(inp_string [i] == '\0' || inp_string [i] == '\n')
			break;
		if((inp_string[i] >= 65) && (inp_string[i] <= 90 ))   //capital letter
			inp_string[i] += 32;
			continue;
		if((inp_string[i] >= 97) && (inp_string [i] <= 122))// do nothing
			continue;
		return NULL;   //return null for error			
	}
	
	return inp_string;
}

//sends and recieves search data
int init_search()
{
	char *Q_word = NULL;
	setTimeStamp();
	dict_data search_data = {"","",0};
	dict_data *search_result_data, insert_data;	
	printf("Enter the WORD that you want to search: ");
	Q_word = readinput();
	search_data.word = verifyAndToSmall(Q_word, strlen(Q_word));
	search_data.meaning = "";
	search_data.flag = SEARCH_OP;
	search_data.clnt_no = CLIENT_ID;
	search_data.clock = timestamp;
	search_result_data = operation_execute_1(&search_data, cl1);
	result_flag(search_result_data , "Search");
	if(search_result_data != NULL && search_result_data->flag == SUCCESS)
	{
		printf("\nThe meaning for your word %s is %s on server 1\n \n ", search_result_data->word ,search_result_data->meaning);
	}
	search_result_data = operation_execute_1(&search_data, cl2);
	result_flag(search_result_data , "Search");
	if(search_result_data != NULL && search_result_data->flag == SUCCESS)
	{
		printf("\nThe meaning for your word %s is %s on server 2\n \n ", search_result_data->word ,search_result_data->meaning);
	}
	
}

//sends and recieve insert data
int init_insert()
{       
	char *x = NULL, *y =NULL;
	char c;
	dict_data *insert_result_data , insert_data = {"","",0};
	setTimeStamp();
	do
	{
		printf("PLease enter the word: ");
 		x = readinput();
		if(strcmp(x, "")==0)
			printf("Please Enter a non-empty value!!!\n");
		else
			break;
			
	}while(1);
	do
	{
		printf("Please enter it's meaning (seperate meanings with commas): ");
		y = readinput();
		if(strcmp(x, "")==0)
			printf("Please Enter a non-empty value!!!\n");
		else
			break;
			
	}while(1);
	
	insert_data.word = verifyAndToSmall(x, strlen(x));
	insert_data.meaning = verifyAndToSmall(y, strlen(y));
	insert_data.flag = INSERT_OP;
	insert_data.clnt_no = CLIENT_ID;
	insert_data.clock = timestamp;
	insert_result_data = operation_execute_1(&insert_data, cl1);
	result_flag(insert_result_data, "Insert");
	printf("on server 1\n");
	insert_result_data = operation_execute_1(&insert_data, cl2);
	result_flag(insert_result_data, "Insert");
	printf("on server 2\n");
	//if(insert_result_data != NULL && insert_result_data->flag == SUCCESS)
	//{
	//	printf("\nOperation Successful!!!\n \n");
	//	printf("\nInserted word %s meaning %s", insert_data.word,insert_data.meaning);
	//}
	//else
	//	printf("\nOperation Failed!!! \n \n");
}

//sends and receive data to be deleted

int init_delete()
{
	char *Q_word = NULL;
	int flag = 0;
	dict_data delete_data , *delete_result_data;
	setTimeStamp();
	printf("Enter the WORD that you want to search: ");
	Q_word = readinput();
	delete_data.word = verifyAndToSmall(Q_word, strlen(Q_word));
	delete_data.meaning = "";
	delete_data.flag = CONFIRM_DELETE_OP;
	delete_result_data = operation_execute_1(&delete_data, cl1); //server 1
	result_flag(delete_result_data , "Delete");
	printf("on server 1\n");
	delete_result_data = operation_execute_1(&delete_data, cl2); //server 2
	result_flag(delete_result_data , "Delete");
	printf("on server 2\n");
	
	/*if(delete_result_data == NULL || delete_result_data->flag != SUCCESS)
	{			
		printf("\nSorry Word Not found!!!\n");	
		return 0;
	}
	printf("\nThe meaning for your word %s is %s \n \n", delete_result_data->word ,delete_result_data->meaning);
	printf("\n You want to delete the word??{0 to refuse, any key to accept}:");
	scanf("%d", &flag);
	flushBuffer();
	is_stdin_empty = 0;
	if(flag!=0)
		delete_data.flag = CONFIRM_DELETE_OP;
	delete_result_data = operation_execute_1(&delete_data, cl);
	
	printf("\n DELETION SUCESSFUL: %4s \n \n", (delete_result_data ->flag)? "Done":"Fail");*/
	
}


int showMenu(){
	enum OP {SEARCH = 1, INSERT, DELETE, EXIT};
	int usr_inp;
	printf("RPC Dictionary \n");
	printf("You want to???\n");
	printf("1. Search a word, or \n2. Insert new word-meaning, or \n3. Delete a word \n4. Enough of this \n");
	printf("Your Choice please - ");

	is_stdin_empty = scanf ("%1d",&usr_inp);
	//check if the scanf failed	
	flushBuffer();
	is_stdin_empty = 0;	

	switch(usr_inp)
	{
		case SEARCH: init_search();
			break;
		case INSERT: init_insert();
			break;
		case DELETE: init_delete();
			break;
		case EXIT: return 1;
		default: printf("\nDUH!!! wrong choice \n");
	}
	clnt_destroy(cl1);
	clnt_destroy(cl2);
	
	return 0;
}


int main (int argc, char **argv)
{
	dict_data temp;
	if (argc !=3)
	 {	 
		printf("\n format of the command is: client <server1> <server2>");
		exit (1);
	 }
	do{
		cl1 = clnt_create(argv[1], DICTIONARY_PROG, DICTIONARY_VERS, "tcp");
		tvl.tv_sec = 100;
		clnt_control(cl1, CLSET_TIMEOUT, (char *)&tvl);
		
		cl2 = clnt_create(argv[2], DICTIONARY_PROG, DICTIONARY_VERS, "tcp");
		clnt_control(cl2, CLSET_TIMEOUT, (char *)&tvl);
		if(cl1 == NULL || cl2 == NULL)
		{
			printf("Connection error with servers!!!");
			exit(0);
		}
	}while(!showMenu());

	return 1;
}
int result_flag ( dict_data *result_data , char* operation)
{
	switch(result_data->flag)
	{
		case FAIL: printf("\n%sFailed!!! " , operation);
			break;
		case SUCCESS: printf("\n%s Successful!!! " , operation);
			break;
		case PENDING: printf("\n%s Pending!!! " , operation);
			break;
		case INVALID: printf("\nThe Vector timestamp is INVALID!!! " );
			break;
	}
	return 0;
}
