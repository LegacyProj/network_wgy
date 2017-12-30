/*this is a program which is used to store student score*/
#include "list.h"
#include<stdio.h>
#include<pwd.h>
#include <string.h>
#include <termios.h>
#include<sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<stdlib.h>
#include <strings.h>
#include<errno.h>
#define MAXLINE 4096
#define ARGSIZE 512
#define BUFSIZE 2048
#define NAMESIZE 50
enum command_k{add,del,traverse,invalid};
char argument[ARGSIZE];

LIST_HEAD(head);
struct list_student
{
	struct list_head list;
	char name[20];
	unsigned int score;
};
void get_cmd(char *rec_buffer)
{
    int temp,i;
    char get_input[MAXLINE];
ssss:
    printf("---> ");
    bzero(get_input,sizeof(get_input));
    fgets(get_input,sizeof(get_input),stdin);
    for(temp=0,i=0; temp<strlen(get_input); temp++)
    {
        if(get_input[temp]==' '||get_input[temp]=='\n')i++;
    }
    if(i==strlen(get_input))
    	goto ssss;
    get_input[strlen(get_input)-1]='\0';
    bzero(rec_buffer,MAXLINE);
    snprintf(rec_buffer,MAXLINE,"%s",get_input);
}
enum command_k get_cmd_key(char *rec_buffer)
{
	char *saveptr1,*p;
	enum command_k c;
	p=strtok_r(rec_buffer," ",&saveptr1);
	if(p)
	{
		if(strcmp(p,"add")==0)
			c=add;
		else if(strcmp(p,"del")==0)
			c=del;
		else if(strcmp(p,"traverse")==0)
			c=traverse;
		else 
			c=invalid;
	}
	p=strtok_r(NULL," ",&saveptr1);
	bzero(argument,sizeof(argument));
	while(p)
	{
		strcat(argument,p);
		strcat(argument," ");
		p=strtok_r(NULL," ",&saveptr1);
	}

	return c;
}
void handle_command(enum command_k cmd_key)
{
	char *saveptr2,*p,rec_buffer[ARGSIZE];
	struct list_student *pnode;
	struct list_head *pos;
	int temp;
	
	if(cmd_key==add)
	{
		p=strtok_r(argument," ",&saveptr2);
	if(!p)
	{
		printf("---> please input the name of student and the score:");
		bzero(rec_buffer,sizeof(rec_buffer));
		fgets(rec_buffer,sizeof(rec_buffer),stdin);
		rec_buffer[strlen(rec_buffer)-1]='\0';
		p=strtok_r(rec_buffer," ",&saveptr2);
		if(!p)
			return;
	}
	pnode=(struct list_student *)malloc(sizeof(struct list_student));
	bzero(pnode,sizeof(struct list_student));
	if(pnode)
	{
		list_add(&pnode->list,&head);
		snprintf(pnode->name,sizeof(pnode->name),"%s",p);
		p=strtok_r(NULL," ",&saveptr2);
		while(p)
		{
			if(strlen(p)==1){
				if('0'<=p[0]&&p[0]<='9'){
					pnode->score=atoi(p);
					printf("---> add success!\n");return;
				}
			}
			else 				
				if(strlen(p)==2){
					if('1'<=p[0]&&p[0]<='9'&&'0'<=p[1]&&p[1]<='9'){
						pnode->score=atoi(p);printf("---> add success!\n");return;
					}
				}
			else if(strlen(p)==3){
				if(strcmp(p,"100")==0){
				pnode->score=100;printf("---> add success!\n");
				return;
			}
		}
		printf("---> please input a legal score :");
		bzero(rec_buffer,sizeof(rec_buffer));
		fgets(rec_buffer,sizeof(rec_buffer),stdin);
		rec_buffer[strlen(rec_buffer)-1]='\0';
		p=rec_buffer;
		}
	}
}
if(cmd_key==del)
{
	p=strtok_r(argument," ",&saveptr2);
	if(!p)
	{
		printf("---> please input the name of the student whose information you want to delete:");
		bzero(rec_buffer,sizeof(rec_buffer));
		fgets(rec_buffer,sizeof(rec_buffer),stdin);
		rec_buffer[strlen(rec_buffer)-1]='\0';
		p=strtok_r(rec_buffer," ",&saveptr2);
		if(!p)
			return;
	}
	list_for_each(pos,&head)
	{
		pnode=list_entry(pos,struct list_student,list);
		if(strcmp(pnode->name,p)==0){
			list_del(&pnode->list);
			free(pnode);
			printf("---> del success!\n");
			return;
		}
	}
	printf("---> the student %s does not exist\n",p);
	return ;
}
if(cmd_key==traverse)
{
	list_for_each(pos,&head)
	{
		pnode=list_entry(pos,struct list_student,list);
		printf("---> the student %s's score is %d\n",pnode->name,pnode->score);
	}
	if(list_empty(&head)){
		printf("---> the  table is empty\n");
		return;
	}
	return;
}

}
int main()
{
	char rec_buffer[MAXLINE];
	enum command_k cmd_key;
	printf("---> there are three command\n");
	printf("---> first:add student_name student_score\n");
	printf("---> second:del student_name\n");
	printf("---> third:traverse\n");
	while(1)
	{
		get_cmd(rec_buffer);
		cmd_key=get_cmd_key(rec_buffer);
		if(cmd_key==invalid)
		{
			printf("---> invalide command\n");
			printf("---> there are three command\n");
			printf("---> first:add student_name student_score\n");
			printf("---> second:del student_name\n");
			printf("---> third:traverse\n");
			continue;
		}
		else 
			handle_command(cmd_key);
	}
	return 0;
}




