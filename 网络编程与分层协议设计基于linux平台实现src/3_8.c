/*this is a program which is used to store user phonenumber*/
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
#define char_false -1
enum command_k{add,del,display,invalid};
struct hlist_head htables[26];
char argument[ARGSIZE];
struct hlist_user
{
struct hlist_node list;
char *name;
char phonenumber[15];
};
static int hash(char k)
{
int temp;
for(temp=0;temp<26;temp++)
{
if((k==temp+65)||(k==temp+97))return temp;
}
return char_false;
}
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
    if(i==strlen(get_input))goto ssss;
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
if(strcmp(p,"add")==0)c=add;
else if(strcmp(p,"del")==0)c=del;
else if(strcmp(p,"display")==0)c=display;
else c=invalid;
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
struct hlist_user * pnode;
struct hlist_node * pos;
struct hlist_head * head;
int index,temp,i;
p=strtok_r(argument," ",&saveptr2);
if(cmd_key==add)
{
if(!p)
{
printf("---> please input the user_name and the phonenumber:");
bzero(rec_buffer,sizeof(rec_buffer));
fgets(rec_buffer,sizeof(rec_buffer),stdin);
rec_buffer[strlen(rec_buffer)-1]='\0';
p=strtok_r(rec_buffer," ",&saveptr2);
if(!p)return;
}
while(!(('a'<=p[0]&&p[0]<='z')||('A'<=p[0]&&p[0]<='Z')))
{
printf("---> please input a legal user_name and  phonenumber:");
bzero(rec_buffer,sizeof(rec_buffer));
fgets(rec_buffer,sizeof(rec_buffer),stdin);
rec_buffer[strlen(rec_buffer)-1]='\0';
p=strtok_r(rec_buffer," ",&saveptr2);
if(!p)return;
}
pnode=(struct hlist_user *)malloc(sizeof(struct hlist_user));
bzero(pnode,sizeof(struct hlist_user));
if(pnode)
{
index=hash(p[0]);
head=htables+index;
hlist_add_head(&pnode->list, head);
pnode->name=malloc(strlen(p)+1);
bzero(pnode->name,strlen(p)+1);
if(pnode->name)snprintf(pnode->name,strlen(p)+1,"%s",p);
p=strtok_r(NULL," ",&saveptr2);
while(!p)
{
printf("---> please input the phonenumber:");
bzero(rec_buffer,sizeof(rec_buffer));
fgets(rec_buffer,sizeof(rec_buffer),stdin);
rec_buffer[strlen(rec_buffer)-1]='\0';
p=strtok_r(rec_buffer," ",&saveptr2);
}
while(p)
{
for(i=0,temp=0;temp<strlen(p);temp++)
{
if('0'<=p[temp]&&p[temp]<='9')i++;
}
if(i==strlen(p))break;
bzero(rec_buffer,sizeof(rec_buffer));
while(strlen(rec_buffer)==0)
{
printf("---> please input a legal phonenumber:");
bzero(rec_buffer,sizeof(rec_buffer));
fgets(rec_buffer,sizeof(rec_buffer),stdin);
rec_buffer[strlen(rec_buffer)-1]='\0';
}
p=strtok_r(rec_buffer," ",&saveptr2);
}
snprintf(pnode->phonenumber,sizeof(pnode->phonenumber),"%s",p);
printf("---> add success!\n");
return ;
}

}
if(cmd_key==del)
{
if(!p)
{
printf("---> please input the user_name:");
bzero(rec_buffer,sizeof(rec_buffer));
fgets(rec_buffer,sizeof(rec_buffer),stdin);
rec_buffer[strlen(rec_buffer)-1]='\0';
p=strtok_r(rec_buffer," ",&saveptr2);
if(!p)return;
}
while(!(('a'<=p[0]&&p[0]<='z')||('A'<=p[0]&&p[0]<='Z')))
{
printf("--- >please input a legal user_name:");
bzero(rec_buffer,sizeof(rec_buffer));
fgets(rec_buffer,sizeof(rec_buffer),stdin);
rec_buffer[strlen(rec_buffer)-1]='\0';
p=strtok_r(rec_buffer," ",&saveptr2);
if(!p)return;
}
index=hash(p[0]);
head=htables+index;
hlist_for_each(pos, head)
{
pnode=list_entry(pos,struct hlist_user,list);
if(strcmp(pnode->name,p)==0){hlist_del(&pnode->list);free(pnode);printf("---> del success!\n");return ;}
}
printf("---> the student %s does not exist\n",p);
}
if(cmd_key==display)
{
if(!p)
{
printf("---> please input the user_name:");
bzero(rec_buffer,sizeof(rec_buffer));
fgets(rec_buffer,sizeof(rec_buffer),stdin);
rec_buffer[strlen(rec_buffer)-1]='\0';
p=strtok_r(rec_buffer," ",&saveptr2);
if(!p)return;
}
while(!(('a'<=p[0]&&p[0]<='z')||('A'<=p[0]&&p[0]<='Z')))
{
printf("---> please input a legal user_name:");
bzero(rec_buffer,sizeof(rec_buffer));
fgets(rec_buffer,sizeof(rec_buffer),stdin);
rec_buffer[strlen(rec_buffer)-1]='\0';
p=strtok_r(rec_buffer," ",&saveptr2);
if(!p)return;
}
index=hash(p[0]);
head=htables+index;
hlist_for_each(pos, head)
{
pnode=list_entry(pos,struct hlist_user,list);
if(strcmp(pnode->name,p)==0){printf("---> the user %s's phonenumber is %s\n",pnode->name,pnode->phonenumber);return ;}
}
printf("---> the student %s does not exist\n",p);
}
}
int main()
{
char rec_buffer[MAXLINE];
enum command_k cmd_key;
int temp;
for(temp=0;temp<26;temp++)
{
INIT_HLIST_HEAD(htables+temp);
}
printf("---> there are three command\n");
printf("---> first:add user_name user_phonenumber\n");
printf("---> second:del user_name\n");
printf("---> third:display user_name\n");
while(1)
{
get_cmd(rec_buffer);
cmd_key=get_cmd_key(rec_buffer);
if(cmd_key==invalid)
{
printf("---> invalide command\n");
printf("---> there are three command\n");
printf("---> first:add user_name user_phonenumber\n");
printf("---> second:del user_name\n");
printf("---> third:display user_name\n");
continue;
}
else handle_command(cmd_key);
}

}
