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



struct node *dict_index [26];

struct node
{
  char *word;
  char *meaning;
  struct node *next;
};


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
			break;
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





dict_data * operation_execute_1_svc(dict_data *node_arg, struct svc_req *srvrqst)
{
	enum op_code {INSERT = 2, SEARCH, DELETE, CONFIRM_DELETE};
	static dict_data result_data;
	char* updated_meaning = (char *) malloc(sizeof(char));
	switch(node_arg->flag)
	{
		//case insert searches for an existing entry, if not found then 
		//make a new entry then add it else append the new meaning 
		//to existing meaning and add new entry to the table and delete old word-meaning pair
		case INSERT:    
				strcpy(updated_meaning , linearSearch(node_arg->word));
				if(strcmp(updated_meaning, "") == 0)          //the word is not present
					updated_meaning = node_arg->meaning;
				else					
				{
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
				free(updated_meaning);			
				break;

		case SEARCH:DELETE:	strcpy(result_data.meaning,linearSearch(node_arg->word));
				        if(strcmp(result_data.meaning,"") == 0) //sending fail flag
					{
						result_data.word = "";
						result_data.flag = FAIL;
					}
					else	
					{
						result_data.word =  "";//->word);
						result_data.flag = SUCCESS;
					}
					break;
		 
		case CONFIRM_DELETE:    if(deletion(node_arg->word))
					{
						result_data.word  = "";
						result_data.meaning = "";
						result_data.flag = SUCCESS;
					}				
					break;  				
	}//switch end


	//return the result
	return (&result_data);
	
}



