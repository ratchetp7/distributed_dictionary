////////////////////////
//*********YOLO******///
////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "dictionary.h"

#define FAIL  0
#define SUCCESS 1
#define INVALID -1
#define PENDING -2
#define FILEPATH "dictionary.txt"
#define CLIENTFILEPATH "client"

struct node
{
  char *word;
  char *meaning;
  struct node *next;
};

struct buffer_item
{
	char *word;
	char *meaning;
	int *tv;
	int clnt_no;
	int flag;
	struct buffer_item *next;
	
};


struct node *dict_index [26];
struct buffer_item *wait_buffer_head = NULL;

enum op_code {INSERT = 2, SEARCH, DELETE, CONFIRM_DELETE};


int vector_clock[3] = {0,0,0};
int buffer_length = 0;
int first_run = 1;


int timestamp_compare(int *tv_1, int *tv_2)
{
	int gt = 0, lt = 0;
	for(int i = 0; i < 3 ; i++)
	{
		if (tv_1[i] < tv_2[i])
			gt++;
		else 
		if(tv_1[i] > tv_2[i])
			lt++;
				
	}
	
	if(((gt > 0) && (lt > 0)) || ((gt == 0) && (lt == 0)))
		return 0;
	if(lt > 0)
		return -1;
	return 1;
}


int is_timestamp_valid (int *timestamp, int clnt_no)
{
	if(vector_clock[clnt_no-1] >= timestamp[clnt_no - 1])
	{
		return 0;
	}
	if(timestamp_compare(vector_clock, timestamp) < 1)
	{
		return 0;
	}
	struct buffer_item *scanner = wait_buffer_head;
	while(scanner != NULL)
	{
		if(timestamp_compare(scanner->tv, timestamp) == 0)
			return 0;
		scanner = scanner->next;
	}
	return 1;
}


int insert (char* word1 , char* meaning1) //insert new node in the list
{
	int temp = word1[0];
	int q = temp-97; 
	struct node *new_node, *current;
	
	new_node = (struct node*) malloc(sizeof(struct node));
	new_node->word = (char*) malloc(strlen(word1)+1);
	strcpy(new_node->word,word1);
	new_node->meaning = (char*) malloc(strlen(meaning1)+1);
	strcpy(new_node->meaning,meaning1);
	
	new_node->next = NULL;
	current = dict_index[q];
	

		if(dict_index[q]==NULL)
		   { dict_index[q] = new_node; }

		else 
		{
		  while(current->next != NULL)
		    {
		      current = current->next;
	 	    }
			current->next = new_node;
		 }    
		 return SUCCESS;
}



char* linearSearch(char* key)
{
	struct node* search_node;	
	search_node = dict_index [key[0]-97];
	
	while (search_node != NULL)
	{       
		if( strcmp(search_node->word,key) == 0)//word found			
			return search_node->meaning;

		search_node = search_node->next;
	}
	return "";
}


int deletion(char* key)
{
	struct node* target_node = dict_index [key[0]-97];
	struct node* prev_node = NULL;	
	
	//search for the item
		
	while (target_node != NULL)       
		if( strcmp(target_node->word,key) == 0)//word found
			{break;}
		else
		{
			prev_node = target_node;			
			target_node = target_node->next;
			
		}
	
	//the found_node is the node before the target node
	
	//if word not found then return fail	
	if (target_node == NULL)       
		return FAIL;

	//check if the first node in the list is the target_node
	if (prev_node == NULL)
		dict_index [key[0]-97] = target_node->next;

	else	
	//rearrange pointers
	prev_node->next = target_node->next;
	
	//delete target node
	free(target_node->word);
	free(target_node->meaning);
	free (target_node);
	return SUCCESS;
}


/*
void flush_to_file()
{
	//open the database file
	struct node *read_pointer;
	FILE *db = fopen(FILEPATH, "w");
	for(int i=0; i<26; i++)
	{
		read_pointer = dict_index[i];
		while(read_pointer != NULL)
		{
			//format of the entry will be <word:meaning>
			fputs(read_pointer->word,db);
			fputc('\n',db);
			fputs(read_pointer->meaning, db);
			fputc('\n',db);
			read_pointer = read_pointer->next;
		}
	}
	fclose(db);
}

void load_from_file()
{
	char buffer[200];
	int toggler =0, len = 0;
	char *word, *meaning;
	FILE *db = fopen(FILEPATH, "r");
	while(fgets((buffer), 200, db) != NULL)
	{
		len = strlen(buffer);
		buffer[len-1] = '\0';
		if(!toggler)//word
		{
			word = (char *) malloc(len);
			strcpy(word, buffer);
			
		}else//meaning
		{
			meaning = (char *) malloc(len);
			strcpy(meaning, buffer);
			insert (word, meaning);
			free(word);
			free(meaning);
		}
		toggler ^=1;
	}
	fclose(db);
}*/

int log_event (char *word, char* meaning, char *operation, int client_id)
{
	FILE *client_file, *global_file;
	char *filename = (char *) malloc(sizeof(char)*18); //for the format "logs/client<no>.txt"
	strcpy(filename, CLIENTFILEPATH);
	filename[6] = (char)(48+client_id);
	filename[7] = '\0';
	strcat(filename, ".txt");
	client_file = fopen(filename,"a");
	fputs(operation,client_file);
	fputs(". ",client_file);
	fputs(word,client_file);
	fputs(": ",client_file);
	fputs(meaning,client_file);
	fputs("\n",client_file);
	free(filename);
	fclose(client_file);
	
	//write global log
	global_file = fopen("Log.txt", "a");	
	fputs("Client",global_file);
	fputc((char)(48+client_id),global_file);
	fputc('\t',global_file);
	fputs(operation,global_file);
	fputs(". ",global_file);
	fputs(word,global_file);
	fputs(": ",global_file);
	fputs(meaning,global_file);
	fputs("\n",global_file);
	fclose(global_file);
}


int execute_pending_operation(struct buffer_item *current)
{
	char *updated_meaning, *temp;
	switch(current -> flag)
	{
		case INSERT: 
				temp = linearSearch(current->word);
				if(strcmp(temp, "") == 0)			 //the word is not present
				{         
					updated_meaning = (char *) malloc(sizeof(strlen(current->meaning))+1);
					strcpy(updated_meaning,current->meaning);
				}
				else					
				{
					updated_meaning = (char *) malloc(sizeof(temp)+sizeof(current->meaning)+3);//add 3 for ", " and '0/'
					strcpy(updated_meaning,temp);
					strcat(updated_meaning,", ");
					strcat(updated_meaning,current->meaning);
					deletion(current->word); // delete the old node after insertion of the new				
				}
				log_event(current->word, updated_meaning, "Insertion", current->clnt_no);
				free(updated_meaning);			
				break;
				
		case SEARCH:
				temp = linearSearch(current->word);
				if(strcmp(temp,"") == 0) //sending fail flag
					log_event(current->word, "Not Found", "Search", current->clnt_no);
				else	
					log_event(current->word, temp, "Search", current->clnt_no);
				break;
		
		case CONFIRM_DELETE: 
				if(deletion(current->word))
					log_event(current->word, "Successfully Deleted", "Delete", current->clnt_no);
				else	
					log_event(current->word, "Not Found", "Delete", current->clnt_no);
				break;
	}
	printf("\nExecuted operation from client:%d with timestamp %d, %d, %d",current->clnt_no, current -> tv[0],current -> tv[1],current -> tv[2]);
	++(vector_clock[(current -> clnt_no) -1]);
	printf("\nVector Clock updated to %d %d %d", vector_clock[0],vector_clock[1],vector_clock[2]);

}

int check_pending_operations()
{
	struct buffer_item *current = wait_buffer_head;
	while(current != NULL)
	{
		for(int i = 0; i<3 ;i++)
		{
			if(i == current->clnt_no - 1)
			{
				if((current -> tv)[i] -1 != vector_clock [i] )
					return 0;				
			}
			else
			{
				if((current -> tv)[i] > vector_clock [i])
					return 0;
			}	
		}
		//execute the operation
		execute_pending_operation(current);
		wait_buffer_head = wait_buffer_head->next;
		free(current->word);
		free(current->meaning);
		free(current->tv);
		free(current);
		current = wait_buffer_head;
			
	}
}

dict_data * operation_execute_1_svc(dict_data *node_arg, struct svc_req *srvrqst)
{
	if(first_run)
	{
		FILE *client_file_1, *client_file_2,*client_file_3,*log_file;
		client_file_1 = fopen("client1.txt","w+");
		client_file_2 = fopen("client2.txt","w+");
		client_file_3 = fopen("client3.txt","w+");
		log_file = fopen("Log.txt","w+");
		putc('\n',log_file);
		fclose(client_file_1);
		fclose(client_file_2);
		fclose(client_file_3);
		fclose(log_file);
		first_run = 0;
	}
	static dict_data result_data;
	char  *updated_meaning, *temp;
	int should_wait = 0;
	//verify the timestamps validity
	if(is_timestamp_valid(node_arg -> clock, node_arg -> clnt_no))
	{	
		//check if the operation is apt to be executed now
		//else add it to the wait_buffer
		for(int i = 0; i<3 ;i++)
		{
			if(i == node_arg -> clnt_no - 1)
			{
				if((node_arg -> clock)[i] - 1 != vector_clock [i])
					should_wait = 1;				
			}
			else
			{
				if((node_arg -> clock)[i] > vector_clock [i])
					should_wait = 1;
			}	
		}
		
		if(!should_wait)
		{
			switch(node_arg->flag)
			{
				//case insert searches for an existing entry, if not found then 
				//make a new entry then add it else append the new meaning 
				//to existing meaning and add new entry to the table and delete old word-meaning pair
				case INSERT:    
						temp = linearSearch(node_arg->word);
						if(strcmp(temp, "") == 0)			 //the word is not present
						{         
							updated_meaning = (char *) malloc(sizeof(strlen(node_arg->meaning))+1);
							strcpy(updated_meaning,node_arg->meaning);
						}
						else					
						{
							updated_meaning = (char *) malloc(sizeof(temp)+sizeof(node_arg->meaning)+3);//add 3 for ", " and '0/'
							strcpy(updated_meaning,temp);
							strcat(updated_meaning,", ");
							strcat(updated_meaning,node_arg->meaning);
							deletion(node_arg->word); // delete the old node after insertion of the new				
						}

						if(insert(node_arg->word, updated_meaning))
						{
							result_data.word="";
							result_data.meaning = "";
							result_data.flag = SUCCESS;
					
						}			
						log_event(node_arg->word, updated_meaning, "Insertion", node_arg->clnt_no);	
						free(updated_meaning);
						break;

				case SEARCH:DELETE:	result_data.meaning = linearSearch(node_arg->word);
							if(strcmp(result_data.meaning,"") == 0) //sending fail flag
							{
								result_data.word = "";
								result_data.flag = FAIL;
								log_event(node_arg->word, "Not Found", "Search", node_arg->clnt_no);
							}
							else	
							{
								result_data.word =  "";//->word);
								result_data.flag = SUCCESS;
								log_event(node_arg->word, temp, "Search", node_arg->clnt_no);
							}
							break;
				 
				case CONFIRM_DELETE:    if(deletion(node_arg->word))
							{
								result_data.word  = "";
								result_data.meaning = "";
								result_data.flag = SUCCESS;
								log_event(node_arg->word, "Successfully Deleted", "Delete", node_arg->clnt_no);
							}	
							else
							{
								log_event(node_arg->word, "Not Found", "Delete", node_arg->clnt_no);			
								result_data.word  = "";
								result_data.meaning = "";
								result_data.flag = FAIL;
							}
							break;  				
			}//switch end
			printf("\nExecuted operation from client:%d with timestamp %d, %d, %d",node_arg->clnt_no, node_arg -> clock[0],node_arg -> clock[1],node_arg -> clock[2]);
			++(vector_clock[(node_arg -> clnt_no) -1]);
			printf("\nVector Clock updated to %d %d %d", vector_clock[0],vector_clock[1],vector_clock[2]);
			check_pending_operations();
			
		}
		else
		{
			struct buffer_item *item = (struct buffer_item *) malloc(sizeof(struct buffer_item));
			item->word = (char*)malloc(sizeof(node_arg->word) + 1);
			strcpy(item->word, node_arg->word);
			item->meaning = (char*)malloc(sizeof(node_arg->meaning) + 1);
			strcpy(item->meaning, node_arg->meaning);
			item->tv = (int*)malloc(3*sizeof(int));
			item -> flag = node_arg ->flag;
			item -> clnt_no = node_arg -> clnt_no;
			for(int j = 0; j<3; j++)
				(item->tv)[j] = *(node_arg->clock + j);
			
			//add to the wait_buffer
			if(wait_buffer_head == NULL)//when list is empty
			{
				wait_buffer_head = item;
				item->next = NULL;
			}
			else 
			{
				if(timestamp_compare(wait_buffer_head->tv, item -> tv) == -1)
				{
					item->next = wait_buffer_head;
					wait_buffer_head = item;
				}
				else //first find the correct position in the sorted list
				{
					struct buffer_item *search_pointer = wait_buffer_head;
					while(search_pointer->next != NULL && timestamp_compare((search_pointer->next)->tv, item -> tv) == 1 )
					{
							search_pointer = search_pointer->next;
					}
					item -> next = search_pointer -> next;
					search_pointer -> next = item;
					//print buffer contents
				}
			}
			result_data.word="";
			result_data.meaning = "";
			result_data.flag = PENDING;
			buffer_length++;	
		}
	}
	else //when timestamp in the message is not valid
	{
		result_data.word="";
		result_data.meaning = "";
		result_data.flag = INVALID;
	}
	printf("\nRecieved message from client:%d with timestamp %d, %d, %d",node_arg->clnt_no, node_arg -> clock[0],node_arg -> clock[1],node_arg -> clock[2]);
	fflush(stdout);
	//return the result
	return (&result_data);
	
}



