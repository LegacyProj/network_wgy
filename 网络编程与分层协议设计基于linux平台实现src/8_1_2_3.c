#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <signal.h>
#include <termios.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include "ftpclient.h"

FTPCMD ftp_cmd[] =
{
    {"dir", "LIST",  NULL, NULL},
    {"ls", "LIST",   NULL, NULL},
    {"get", "RETR",  "Remote file", NULL},
    {"put",  "STOR",  "Local file", NULL},
    {"mdelete","NLST","Remote files",NULL},
    {"mget","NLST","Remote files",NULL},
    {"mput", "STOR","Local files",NULL},
    {"cd", "CWD", "Remote directory", do_common_cmd},
    {"delete", "DELE", "Remote file", do_common_cmd},
    {"lcd",  "LCD",  NULL, do_lchdir},
    {"mkdir","MKD", "Remote directory", do_common_cmd},
    {"rmdir","RMD", "Remote directory", do_common_cmd},
    {"passive", "PASV", NULL, do_pasv},
    {"pwd",  "PWD",  NULL, do_common_cmd},
    {"binary", "TYPE I",  NULL, do_common_cmd},
    {"ascii", "TYPE A",  NULL, do_common_cmd},
    {"bye", "QUIT",  NULL, do_common_cmd},
    {"user", "USER", "Username", do_user}
};

#define CMD_NUM sizeof(ftp_cmd)/sizeof(ftp_cmd[0])
enum transf_type {ASCII_MODE,BINARY_MODE};
enum transf_type current_type;

int mode;   					/* transfer mode, either active or passive */
int sockfd_cmd;    				/* socket for ftp command */
volatile sig_atomic_t ctrl_z;
int data_flag;                  /* 1, data transfer is going on */
char args[ARGSIZE];               /* the ftp cmd args */
char ip_args[ARGSIZE];            /* ip string used in PORT command, f.e. 192,168,1,137, */
int ipstr_len;
struct sockaddr_in local_addr;    /* local socket addr for this client */
struct sockaddr_in server_addr;   /* ftp server socket address */
char cmd_str[CMDSIZE];            /* the input ftp cmd string */
char cmd[CMDSIZE];                /* the ftp cmd derived from the cmd_str */

int main (int argc, char *argv[])
{
    struct hostent *host;
    int found;                      /* 1, the input ftp cmd is valid */
    int port = 21;

    if (argc < 2)
    {
        fprintf (stderr, "Usage: %s    hostname   [portnumber]\a\n", argv[0]);
        exit (1);
    }
    if ((host = gethostbyname (argv[1])) == NULL)
    {
        fprintf (stderr, "Gethostname error\n");
        exit (1);
    }
    if ((argc == 3) && (port = atoi (argv[2])) < 0)
    {
        fprintf (stderr, "Usage: %s hostname [portnumber]\a\n", argv[0]);
        exit (1);
    }

    /* create client socket for ftp command communication */
    if ((sockfd_cmd = socket (PF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf (stderr, "Socket Error: %s\a\n", strerror (errno));
        exit (1);
    }

    init();

    /* login to ftp server*/
    login(host, port);

    /* 生成当前ip地址的ftp port命令参数, 具体port值在后面适当时机产生 get local_addr*/
    ipstr_len = make_port_args(sockfd_cmd, &local_addr);

    int size = sizeof(cmd);
    /* main ftp command loop */
    for (;;)
    {
        printf("ftp> ");

        if(!input_cmd(cmd_str, size))  /*get commmand line input(including everything), and  replace '\n' with '\0'*/
        {
            continue;                  /*the return value is the length of the cmd_str*/
        }
        trim_right(cmd_str);/*replace ' ' of the right side of cmd_str with '\0'*/

        /* 检查该输入命令是否为合法ftp命令, 若合法则全局变量cmd中将存放此命令 */
        found = check_ftpcmd(cmd_str, cmd);/*now the protocol command is in the string pointed by cmd,and with the argument if there is */
        if(found < 0)                         /*remove the additional space between the arguement*/
        {
            printf("Invalid command.\n");
            continue;
        }

        /* 检查该ftp命令是否需要参数 */
        if(ftp_cmd[found].args)
        {
            /* 若该ftp命令输入时，没有直接带上参数 */
            /*
             * return 0, 未携带必要的参数

             * return 1, 携带了必要的参数

             */
            if(!having_args(cmd)) /*let the argument of the command pointed by args*/
            {
                printf("%s ", ftp_cmd[found].args);

                /* get parameter for this cmd */
                input_cmd(args, size);
                trim_right(args);
            }
        }
        /* 执行该ftp命令 */
        ftp_cmd[found].handler(sockfd_cmd, cmd, args);
    }
    return 0;
}

void init()/*1*/
{
    /* set the default mode as PASSIVE */
    mode = PASSIVE_ON;
    ctrl_z = 0;
    data_flag = 0;
    current_type=ASCII_MODE;
    memset(args, 0, sizeof(args));

    ftp_cmd[0].handler = do_list_pasv;
    ftp_cmd[1].handler = do_list_pasv;
    ftp_cmd[2].handler = do_get_pasv;
    ftp_cmd[3].handler = do_put_pasv;
    ftp_cmd[4].handler = do_mdel_pasv;
    ftp_cmd[5].handler = do_mget_pasv;
    ftp_cmd[6].handler = do_mput_pasv;
    ignore_sigtstp();
}

void login(struct hostent *host, int portnumber)/*2*/
{
    char res_buffer[BUFSIZE];
    int status;

    /* prepare the ftp server socket address */
    memset (&server_addr, 0, sizeof server_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons (portnumber);
    server_addr.sin_addr = *((struct in_addr *) host->h_addr);

    /* connect to ftp server */
    if (connect (sockfd_cmd, (struct sockaddr *) (&server_addr),
                 sizeof server_addr) == -1)
    {
        fprintf (stderr, "Connect Error: %s\a\n", strerror (errno));
        exit (1);
    }

    get_ftpcmd_status(sockfd_cmd, res_buffer);
    printf ("Name (%s:%s): ", inet_ntoa(server_addr.sin_addr), get_usrname());
    /* input user name */
    fgets(cmd, sizeof(cmd), stdin);
    cmd[strlen(cmd) - 1] = 0;

    send_ftpcmd(sockfd_cmd, "USER", cmd);
    status = get_ftpcmd_status(sockfd_cmd, res_buffer);

    /*input password*/
    printf("Password: ");
    terminal_echo_off(STDIN_FILENO);
    fgets(cmd, sizeof(cmd), stdin);
    terminal_echo_on(STDOUT_FILENO);
    printf("\n");
    cmd[strlen(cmd) - 1] = 0;

    send_ftpcmd(sockfd_cmd, "PASS", cmd);
    status = get_ftpcmd_status(sockfd_cmd, res_buffer);
}

/*
 * 构建PORT命令所需要的ip地址字符串, 即 "192,168,1,137,"
 * 返回值：所构建的ip字符串长度
 */
int make_port_args(int fd, struct sockaddr_in *local_addr)/*3*/
{

    char *ipstr;
    int len;
    memset(ip_args, 0, ARGSIZE);

    /* get local ip for this ftp client */
    ipstr = get_localip (fd, local_addr);

    replace_delim(ipstr, '.', ',');
    len = strlen(ipstr);
    /* copy the transformed ip address string into ip_args */
    strncpy (ip_args, ipstr, len);
    strcat (ip_args, ",");

    return len + 1;
}

int send_ftpcmd(int fd, const char *cmd, const char *args)/*4*/
{
    char buf[128];
    int z, len;
    memset(buf, 0, sizeof buf);
    strncpy(buf, cmd, strlen(cmd));

    if(args)
    {
        strcat(buf, " ");
        strcat(buf, args);
    }
    if(strcmp(cmd,"PASS")==0)
        printf("---> PASS XXXX\n");
    else printf("---> %s\n",buf);
    strcat(buf, "\r\n");
    z = write(fd, buf, strlen(buf));

    if(z == -1)
    {
        bail("write");
        exit(1);
    }

    return z;
}

int input_cmd(char *cmd, int size)/*5*/
{
    int len;
    memset(cmd, 0, size);

    fgets(cmd, size, stdin);

    len = strlen(cmd);
    if(len)
    {
        cmd[len - 1] = 0; /* remove the last '\n' char */
        len--;
    }
    return len;
}

char *trim_right(char *cmd_str)/*6*/
{
    int  i, len;

    len = strlen(cmd_str);
    i = 1;
    while(*(cmd_str + len - i) == 32 )
    {
        *(cmd_str + len - i)=0;
        i++;
    }

    return cmd_str;
}
char *trim_left(char *str)
{
    int len,i=0;
    len=strlen(str);
    while(*(str+i)==32)
    {
        i++;
    }
    return str+i;
}

int arg_count(char *args)
{
    char *p;
    char temp[ARGSIZE];
    int i=0;
    trim_right(args);
    bzero(temp,sizeof(temp));
    snprintf(temp,sizeof(temp),"%s",args);
    p= strtok(temp, " ");
    while(p)
    {
        i++;
        p = strtok(NULL, " ");
    }
    return i;
}
/*
 * 根据输入的ftp命令字串传cmd_str，在命令对照表ftp_cmd[i]中搜索
 * 是否存在此输入的ftp命令，若找到，则将此命令字符串拷贝到全局
 * 变量cmd中
 *
 * @cmd_str, the input ftp cmd string
 * return -1, not found this cmd in ftp_cmd[i]
 *        others, the index in ftp_cmd[i]
 */
int check_ftpcmd(char *cmd_str, char *cmd)/*7*/
{
    int found, i,j;
    int withargs=0;
    char *p = NULL;

    i=j=0;

    /* 清空用于存储ftp命令的缓存字符数组cmd */
    memset(cmd, 0, CMDSIZE);
    memset(args, 0, CMDSIZE);

    /* trim left space in cmd_str --note here */
    while(*(cmd_str + j) == 32)
        j++;

    /* 如果命令字符串中没有携带参数， 则直接使用cmd_str，否则去掉参数部分 */
    if(!strstr(cmd_str + j, " "))
        /* not found parameter*/
        p = cmd_str + j;
    else
    {
        /* 获得第1个命令参数前的命令字符串 */
        p = strtok(cmd_str + j, " ");/*return a pointer to a null-terminated containing the next token,this string does not include the delimiting string*/
        withargs = 1;
    }

    /* 在ftp命令字符串结构数组中遍历检查是否匹配输入的命令字串 */
    while(i < CMD_NUM)
    {
        if(!strcmp(p, ftp_cmd[i].alias))
        {
            found = i;
            break;
        }
        i++;
    }
    if(i == CMD_NUM)
        /* 没有找到匹配该输入字串的ftp命令 */
        found = -1;

    else
    {
        strncpy(cmd, ftp_cmd[i].name, strlen(ftp_cmd[i].name));/*not be null-terminated automatically*/
        /*put the ftp protocol command into cmd*/
        if(withargs)
        {

            /* 获得输入命令字符串cmd_str中剩下可能有的命令参数 */
            while(p = strtok(NULL, " "))
            {
                strcat(cmd, " ");
                strcat(cmd, p);
            }
        }
    }

    return found ;
}
/*
 * 判断当前需要进行数据传输的ftp命令是否携带了必要的参数
 *
 * return 0, 未携带必要的参数
 *
 * return 1, 携带了必要的参数
 */
int having_args(char *cmd)/*8*/
{
    char *p = NULL;

    memset(args, 0, sizeof(args));

    if(!(p = strstr(cmd, " ")))
        return 0;

    /* 将全局变量cmd中代表参数的空格位置设为0 */
    *p++ = 0;

    /* 将参数拷贝到全局变量args中 */
    strncpy(args, p, strlen(p));/*because of the call of the function memset, the string will be null-terminated*/

    return 1;
}

/*
 * 创建用于主动模式下进行数据传输的本地socket，并在此socket
 * 上进行监听
 *
 * 返回值-1，     失败
 * 返回值sockfd，成功
 */
int active_listen()/*9*/
{
    int sockfd;

    if ((sockfd = socket (PF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf (stderr, "Socket error: %s\a\n", strerror (errno));
        return -1;
    }

    /*set protocol address for sockfd_act_listen*/
    memset (&local_addr, 0, sizeof local_addr);
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl (INADDR_ANY);
    local_addr.sin_port = 0;

    /*bind client's data socket to local_addr to listen data connection from ftp server*/
    if ((bind (sockfd, (struct sockaddr *) (&local_addr), sizeof local_addr)) == -1)
    {
        fprintf (stderr, "Bind error: %s\a\n", strerror (errno));
        return -1;
    }

    if(listen(sockfd, 5))
    {
        fprintf(stderr, "Listen error: %s\n\a", strerror(errno));
        return -1;
    }

    return sockfd;
}

/*
 * 获得用于当前数据传输所使用的socket正在监听的端口
 * 该函数必须在当前socket已经开始监听后
 *
 * 返回值，正在监听的port
 */
int get_active_port(int fd_listen)/*10*/
{
    unsigned short port;
    socklen_t sin_size;

    sin_size = sizeof (struct sockaddr_in);
    getsockname(fd_listen, (struct sockaddr*)&local_addr, &sin_size);
    port=ntohs(local_addr.sin_port);

    return port;
}

/*
 * get the local host's IP with which the ftp cmd
 * is sent
 */
char * get_localip(int fd, struct sockaddr_in *local_addr)/*11*/
{
    int size = sizeof (struct sockaddr_in);
    getsockname(fd, (struct sockaddr *)local_addr, &size);

    return inet_ntoa(local_addr->sin_addr);

}

/*
 * replace the orig char with the substitue char
 *
 * @ipstr, the ip string
 * @orig, the original char in ipstr
 * @substitue, the char used to replace @orig
 */
void replace_delim(char *ipstr, char orig, char substitute)/*12*/
{
    char *temp;

    temp = ipstr;

    while (*temp != 0)
    {
        if (*temp == orig)
            *temp = substitute;
        temp++;
    }
}

int get_ftpcmd_status(int sockfd, char *res_buffer)/*13*/
{
    char code[5], *msg;
    int z, status;
    do
    {
        memset(res_buffer, 0, BUFSIZE);
redo:
        z = read(sockfd, res_buffer, BUFSIZE);
        if (z < 0 && errno == EINTR)
            goto redo;
        msg = strtok(res_buffer, "\r\n");
        printf ("%s\n", msg);
        memset(code, 0, sizeof(code));
        status = atoi(strncpy(code, msg, 4));
        if(code[3] != ' ' || status <= 0)
        {
            status = -1;/*test every time read*/
        }
        if(status == -1)
        {
            while(msg = strtok(NULL, "\r\n"))
            {
                printf("%s\n", msg);
                memset(code, 0, sizeof(code));
                status = atoi(strncpy(code, msg, 4));
                if(code[3] != ' ')
                {
                    status = -1;
                }
            }
        }
        if(status > 0)
        {
            break;
        }
    }
    while(1);
    return status;
}

/*
 * 用于主动模式下向ftp服务器发送port命令
 *
 * 返回值0,                  表示port命令发送失败
 * 返回值sockfd_listen_act,  表示port命令发送i成功
 *
 */
int active_notify(int fd)/*14*/
{
    int status;
    int sockfd_listen_act;
    char res_buffer[BUFSIZE];
    unsigned short port;
    char quot[4], resi[4];

    /* restore ip_args as the original value */
    memset(ip_args + ipstr_len, 0, ARGSIZE - ipstr_len);

    /* returns the client listen socket port */
    sockfd_listen_act = active_listen();

    if(sockfd_listen_act == -1)
        return 0;                               /* listen fails */

    port = get_active_port(sockfd_listen_act);

    snprintf(quot, sizeof(quot), "%d", port/256);
    snprintf(resi, sizeof(resi), "%d", port%256);

    strcat(ip_args, quot);
    strcat(ip_args, ",");
    strcat(ip_args, resi);

    send_ftpcmd (fd, "PORT", ip_args);
    status = get_ftpcmd_status(fd, res_buffer);
    if( status != 200 )
        /* 如果端口命令不成功 */
        return 0;

    return sockfd_listen_act;
}

/*
 * 用于被动模式下向ftp服务器发送pasv命令
 *
 * 返回值port_pasv, 表示正确收到服务器发回的port
 * 返回值0        , 表示port命令发送失败
 */
int passive_notify(int fd)/*15*/
{
    int status;
    int port_pasv;
    char res_buffer[BUFSIZE];

    send_ftpcmd (fd, "PASV", NULL);
    status = get_ftpcmd_status(fd, res_buffer);
    if( status != 227 )
        /* 如果PASV命令不成功 */
        return 0;

    port_pasv = parse_port(res_buffer, strlen(res_buffer));

    return port_pasv;
}

int parse_port(char *s, int len)/*16*/
{
    char *p;
    char *parm[6];
    int port, resi, quot;
    int i = 0;

    memset(s + len - 1, 0, 1);

    p = strstr(s, "(");
    p++;
    parm[i++] = strtok(p, ",");
    while(parm[i++] = strtok(NULL, ","));
    resi = atoi(parm[5]);
    quot = atoi(parm[4]);

    port = quot * 256 + resi;

    return port;
}

int make_conn_active(int fd_listen)/*17*/
{
    int sockfd;
    socklen_t sin_size;

    /* 等待ftp服务器连接过来 */
    if((sockfd = accept(fd_listen, (struct sockaddr *)(&local_addr), &sin_size))== -1)
    {
        fprintf(stderr, "Accept error: %s\a\n", strerror(errno));
        return -1;
    }

    return sockfd;
}

int make_conn_passive(int port)/*18*/
{
    int sockfd;   				/* passive mode socket for ftp data transfer */

    /* create client socket for passive mode */
    if ((sockfd = socket (PF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf (stderr, "Socket Error: %s\a\n", strerror (errno));
        return 1;
    }

    server_addr.sin_port = htons (port);

    /* connect to ftp server */
    if (connect (sockfd, (struct sockaddr *) (&server_addr), sizeof (server_addr)) == -1)
    {
        fprintf (stderr, "Connect Error: %s\a\n", strerror (errno));
        return 0;
    }

    return sockfd;
}
/*
 * 列表文件
 */
int list_files(int sockfd,char *rec_buffer)/*19*/
{
    unsigned char data[BUFSIZE];	/* for incoming data */
    int z=0,t=0;

    if(rec_buffer)bzero(rec_buffer,BUFSIZE);
    /* 允许对CTRL+Z信号进行处理 */
    unignore_sigtstp();

    for (;;)
    {
        memset(data, 0, sizeof(data));
        t=t+z;
        /* only reads BUFSIZE bytes once a time */
        z = read (sockfd, data, sizeof(data));
redo:
        if (z < 0 && !ctrl_z)   /**/
        {
            bail ("read()");
            exit(1);

        }
        else if(ctrl_z)                           /* CTRL+Z input */
        {
            ctrl_z = 0;                           /* restore CTRL+Z flag */
            /* 不允许对CTRL+Z信号进行处理 */
            ignore_sigtstp();

            return 0;
        }

        /* ftp server close data connection */
        if (z == 0)
        {
            if(ctrl_z)                             /* 已经调用了CTRL+Z信号处理函数 */
            {
                ctrl_z = 0;
                ignore_sigtstp();

                return 0;
            }
            break;
        }
        printf("%s", data);
        if(rec_buffer)snprintf(rec_buffer+t,BUFSIZE,"%s",data);
    }

    ignore_sigtstp();
    data_flag = 0;/**/
    return 1;
}

int download_file(char *filename, int sockfd)/*20*/
{
    FILE *fp = NULL;
    unsigned char data[BUFSIZE];
    int z, file_size = 0;
    char cur_dir[256];  /**/
    char filepath[256];
    const char *p;

    memset(filepath, 0, sizeof(filepath));
    memcpy(filepath, "./", strlen("./"));

    /* 获得文件路径中最后的文件名 */
    p = filename + strlen(filename);
    while(*p != '/' && p != filename)
        p--;
    if(p != filename)
        /* 文件名中包含路径 */
        p++;
    else
        /* 文件名中不包含路径 */
        p = filename;

    if(!fp)
    {
        strcat(filepath, p);
        fp=fopen(filepath, "a");
    }

    /* 开始进入数据传输阶段 */
    unignore_sigtstp();
    for (;;)
    {
        memset(data, 0, sizeof(data));

        /* only reads BUFSIZE bytes once a time */
        z = read (sockfd, data, sizeof(data));
        if (z < 0 && !ctrl_z)
        {
            bail ("download_file");
            exit(1);

        }
        else if(ctrl_z)                           /* CTRL+Z input */
        {
            ctrl_z = 0;                           /* restore CTRL+Z flag */
            fclose(fp);
            ignore_sigtstp();

            return 0;
        }

        /* ftp server close data connection, eof is met */
        if (z == 0)
        {
            /* 退出数据传输阶段 */
            data_flag = 0;
            fclose(fp);
            ignore_sigtstp();

            if(ctrl_z)                             /* 若刚好在全部下载完毕时收到CTRL+Z信号 */
            {
                ctrl_z = 0;
                return 0;
            }
            break;
        }

        file_size += z;

        if(fp)
        {
            fwrite(data, z, 1, fp);
        }
    }

    ignore_sigtstp();
    return file_size;
}

int upload_file(FILE *fp, int sockfd)
{
    unsigned char data[BUFSIZE];
    int zr;
    int zw;
    int file_size = 0;

    unignore_sigtstp();
    for (;;)
    {
        memset(data, 0, sizeof(data));

        zr = fread(data, 1, BUFSIZE, fp);
        if (zr < 0 && !ctrl_z)
        {
            bail ("read()");
            exit(1);
        }
        else if(ctrl_z)
        {
            ctrl_z = 0;
            fclose(fp);
            ignore_sigtstp();

            return 0;
        }

        if (zr == 0)
        {
            data_flag = 0;
            fclose(fp);
            ignore_sigtstp();

            if(ctrl_z)    /* 若刚好在全部上载完毕时收到CTRL+Z信号 */
            {
                ctrl_z = 0;
                return 0;
            }
            break;
        }

        /* write zr bytes */
        zw = write (sockfd, data, zr);
        if(zw < 0 && !ctrl_z)
        {
            bail("write()");
            exit(1);
        }

        file_size += zw;
    }

    ignore_sigtstp();
    return file_size;
}

void report(struct timeval *start_time, struct timeval *finish_time, int fsize)
{

    double dtime;
    char outstr[8], *p;

    memset(outstr, 0, sizeof(outstr));

    dtime = (finish_time->tv_sec - start_time->tv_sec) \
            + (finish_time->tv_usec - start_time->tv_usec)/(1000*1000.0);
    sprintf(outstr, "%-6.2f", dtime);

    p = strtok(outstr, " ");
    if(p)
        printf("%d bytes received in %s secs.\n", fsize, p);
    else
        printf("%d bytes received in %-6.2f secs.\n", fsize, outstr);
}

int do_user(int fd, char *cmd, char *args)
{
    char res_buffer[BUFSIZE];
    int status;

    send_ftpcmd(fd, cmd, args);
    status = get_ftpcmd_status(fd, res_buffer);

    if(status != 331)
        return 1;      /* fails */

    /* input password */
    printf("Password: ");
    terminal_echo_off(STDIN_FILENO);
    fgets(cmd, CMDSIZE, stdin);
    terminal_echo_on(STDOUT_FILENO);
    printf("\n");
    cmd[strlen(cmd) - 1] = 0;

    send_ftpcmd(sockfd_cmd, "PASS", cmd);
    get_ftpcmd_status(sockfd_cmd, res_buffer);

    return 0;
}

int do_common_cmd(int fd, char *cmd, char *args)
{
    char res_buffer[BUFSIZE];
    int status;

    if(strlen(args))
        send_ftpcmd(fd, cmd, args);
    else
    {
        send_ftpcmd(fd, cmd, NULL);
        if(strcmp(cmd,"TYPE I")==0)current_type=BINARY_MODE;
        if(strcmp(cmd,"TYPE A")==0)current_type=ASCII_MODE;
    }
    status = get_ftpcmd_status(fd, res_buffer);

    if(status == 250 || status == 257
            || status == 200 || status == 230
            || status == 331)
        return 0;                               /* success */

    if(status == 221 || status == 421)          /* bye input */
    {
        close(fd);
        exit(0);
    }

    return -1;
}

int do_pasv(int fd, char *cmd, char *args)
{
    mode = !mode;
    if(mode)
    {
        printf("Passive mode on.\n");

        ftp_cmd[0].handler = do_list_pasv;      /* LIST command */
        ftp_cmd[1].handler = do_list_pasv;      /* DIR command */
        ftp_cmd[2].handler = do_get_pasv;       /* GET command */
        ftp_cmd[3].handler = do_put_pasv;       /* PUT command */
        ftp_cmd[4].handler = do_mdel_pasv;
        ftp_cmd[5].handler = do_mget_pasv;
        ftp_cmd[6].handler = do_mput_pasv;
    }
    else
    {
        printf("Passive mode off.\n");

        ftp_cmd[0].handler = do_list_active;    /* LIST command */
        ftp_cmd[1].handler = do_list_active;    /* DIR command */
        ftp_cmd[2].handler = do_get_active;     /* GET command */
        ftp_cmd[3].handler = do_put_active;     /* PUT command */
        ftp_cmd[4].handler = do_mdel_active;
        ftp_cmd[5].handler = do_mget_active;
        ftp_cmd[6].handler = do_mput_active;
    }

    return mode;
}

int do_list_active(int fd, char *cmd, char *args)
{
    char res_buffer[BUFSIZE];
    int status;
    int sockfd_act_listen;     		/* active mode socket listening for ftp data connection */
    int sockfd_act;   				/* active mode socket for ftp data transfer */
    int ret;

    if(current_type==BINARY_MODE)
    {
        send_ftpcmd(fd,"TYPE A",NULL);
        current_type=ASCII_MODE;
        status = get_ftpcmd_status(fd, res_buffer);
        if(status != 200)
            return 1;
    }
    data_flag = 1;
    /* send PORT command to the server */
    sockfd_act_listen = active_notify(fd);

    if(!sockfd_act_listen)
        return -1;             /* PORT cmd fails */

    /* 发送LIST命令给ftp服务器 */
    send_ftpcmd(fd, cmd, args);
    status = print_final_msg(fd, res_buffer);/**/

    /* if any error happens */
    if(status / 100 == 5)
        return 1;

    sockfd_act = make_conn_active(sockfd_act_listen);
    if(sockfd_act == -1)
        return -1;

    ret = list_files(sockfd_act,NULL);

    close(sockfd_act);
    close(sockfd_act_listen);

    if(!ret)
        return 1;/* 在list命令过程中，用户输入了CTRL+Z */

    status = print_final_msg(fd, res_buffer);
out:
    data_flag = 0;
    return 0;
}

int do_list_pasv(int fd, char *cmd, char *args)
{
    int port_pasv;
    char res_buffer[BUFSIZE];
    int status;
    int sockfd_pasv;
    int ret;

    if(current_type==BINARY_MODE)
    {
        send_ftpcmd(fd,"TYPE A",NULL);
        current_type=ASCII_MODE;
        status = get_ftpcmd_status(fd, res_buffer);
        if(status != 200)
            return 1;
    }

    data_flag = 1;
    /* 向ftp服务器发送pasv命令 */
    port_pasv = passive_notify(fd);
    if(!port_pasv)
        return -1;                   /* PASV cmd fails */

    /* 连接到ftp服务器 */
    sockfd_pasv = make_conn_passive(port_pasv);
    if(!sockfd_pasv)
        return -1;

    /* 发送LIST命令给ftp服务器 */
    send_ftpcmd(fd, cmd, NULL);
    status = get_ftpcmd_status(fd, res_buffer);
    if(status / 100 == 5)
        return 1;                  /* LIST cmd fails */

    ret = list_files(sockfd_pasv,NULL);
    close(sockfd_pasv);

    if(!ret)
        return 1;

    status = print_final_msg(fd, res_buffer);
out:
    data_flag = 0;
    return 0;
}

int do_get_pasv(int fd, char *cmd, char *args)
{
    FILE *fp = NULL;
    int port_pasv;
    char res_buffer[BUFSIZE];
    int status;
    int fz;
    int sockfd_pasv;
    long off_set;
    struct timeval 	start_time, finish_time;
    char filepath[256];
    const char *p;
    memset(filepath, 0, sizeof(filepath));
    memcpy(filepath, "./", strlen("./"));
    /* 获得文件路径中最后的文件名 */
    p = args + strlen(args);
    while(*p != '/' && p != args)
        p--;
    if(p != args)
        /* 文件名中包含路径 */
        p++;
    else
        /* 文件名中不包含路径 */
        p = args;
    if(!fp)
    {
        strcat(filepath, p);
        fp=fopen(filepath, "a");
    }
    off_set=ftell(fp);
    bzero(res_buffer,sizeof(res_buffer));
    snprintf(res_buffer,sizeof(res_buffer),"%d",off_set);

    if(current_type==ASCII_MODE)
    {
        send_ftpcmd(fd,"TYPE I",NULL);
        current_type=BINARY_MODE;
        status = get_ftpcmd_status(fd, res_buffer);
        if(status != 200)
            return 1;
    }
    data_flag = 1;
    /* 向ftp服务器发送pasv命令 */
    port_pasv = passive_notify(fd);
    if(!port_pasv)
        return -1;                               /* PASV cmd fails */
    /* 连接到ftp服务器 */
    sockfd_pasv = make_conn_passive(port_pasv);
    send_ftpcmd(fd, "REST", res_buffer);
    status = get_ftpcmd_status(fd, res_buffer);
    /* if any error happens */
    if(status / 100 == 5)
        return 1;

    fclose(fp);
    /* 发送GET命令给ftp服务器 */
    send_ftpcmd(fd, cmd, args);
    status = get_ftpcmd_status(fd, res_buffer);
    /* if any error happens */
    if(status / 100 == 5)
        return 1;
    gettimeofday(&start_time, NULL);
    fz = download_file(args, sockfd_pasv);
    close(sockfd_pasv);

    if(!fz)
        /* 文件下载过程中被用户ctrl+z终止 */
        return 1;

    status = get_ftpcmd_status(fd, res_buffer);
out:
    gettimeofday(&finish_time, NULL);
    report(&start_time, &finish_time, fz);

    data_flag = 0;
    return 0;
}

int do_mget_pasv(int fd,char *cmd ,char *args)
{

    int port_pasv,sockfd_pasv,i,ret;
    char res_buffer[BUFSIZE];
    int status,number_of_arg=arg_count(args);
    char *rec_list[number_of_arg];
    char *p;
    char *saveptr1,*saveptr2;
    p=strtok_r(args," ",&saveptr1);
    if(current_type==BINARY_MODE)
    {
        send_ftpcmd(fd,"TYPE A",NULL);
        current_type=ASCII_MODE;
        status = get_ftpcmd_status(fd, res_buffer);
        if(status != 200)
            return 1;
    }
    for(i=0; i<number_of_arg; i++)
    {
        if((rec_list[i]=(char *)malloc(sizeof(char)*BUFSIZE))==NULL)
        {
            perror("memory allocate error");
            return 1;
        }
        bzero(rec_list[i],sizeof(rec_list[i]));
        if(i>0)p=strtok_r(NULL," ",&saveptr1);
        data_flag = 1;
        /* 向ftp服务器发送pasv命令*/
        port_pasv = passive_notify(fd);
        if(!port_pasv)
        {
            return -1;                               /* PASV cmd fails*/
        }
        /* 连接到ftp服务器*/
        sockfd_pasv = make_conn_passive(port_pasv);
        send_ftpcmd(fd, cmd, p);
        status = get_ftpcmd_status(fd, res_buffer);
        if(status / 100 == 5)return 1;
        ret = list_files(sockfd_pasv,rec_list[i]);
        close(sockfd_pasv);
        if(!ret)return 1;
        print_final_msg(fd, res_buffer);
        data_flag = 0;
    }
    for(i=0; i<number_of_arg; i++)
    {
        p=strtok_r(rec_list[i],"\r\n",&saveptr2);
        while(p)
        {
            bzero(res_buffer,sizeof(res_buffer));
            printf("mget %s?",p);
            fgets(res_buffer,sizeof(res_buffer),stdin);
            if(!strcmp(res_buffer,"y\n")||!strcmp(res_buffer,"Y\n")||!strcmp(res_buffer,"YES\n")||!strcmp(res_buffer,"yes\n")||!strcmp(res_buffer,"\n"))
            {
                status=do_get_pasv(fd,"RETR",p);
                if(status !=0)
                {
                    p=strtok_r(NULL,"\r\n",&saveptr2);
                    continue;
                }
            }
            else if(!strcmp(res_buffer,"n\n")||!strcmp(res_buffer,"N\n")||!strcmp(res_buffer,"no\n")||!strcmp(res_buffer,"NO\n"))
            {
                p=strtok_r(NULL,"\r\n",&saveptr2);
                continue;
            }
            else continue;
            p=strtok_r(NULL,"\r\n",&saveptr2);
        }

    }

    return 0;
}
int do_get_active(int fd, char *cmd, char *args)
{
    char res_buffer[BUFSIZE];
    int status;
    int fz,off_set;
    int sockfd_act_listen;     		/* active mode socket listening for ftp data connection */
    int sockfd_act;   				/* active mode socket for ftp data transfer */
    struct timeval 	start_time, finish_time;
    char filepath[256];
    FILE *fp;
    const char *p;
    memset(filepath, 0, sizeof(filepath));
    memcpy(filepath, "./", strlen("./"));
    /* 获得文件路径中最后的文件名 */
    p = args + strlen(args);
    while(*p != '/' && p != args)
        p--;
    if(p != args)
        /* 文件名中包含路径 */
        p++;
    else
        /* 文件名中不包含路径 */
        p = args;
    if(!fp)
    {
        strcat(filepath, p);
        fp=fopen(filepath, "a");
    }
    off_set=ftell(fp);
    bzero(res_buffer,sizeof(res_buffer));
    snprintf(res_buffer,sizeof(res_buffer),"%d",off_set);
    if(current_type==ASCII_MODE)
    {
        send_ftpcmd(fd,"TYPE I",NULL);
        current_type=BINARY_MODE;
        status = get_ftpcmd_status(fd, res_buffer);
        if(status != 200)
            return 1;
    }
    data_flag = 1;
    /* send PORT command to the server */
    sockfd_act_listen = active_notify(fd);

    if(!sockfd_act_listen)
        return -1;                               /* PORT cmd fails */


    send_ftpcmd(fd, "REST", res_buffer);
    status = get_ftpcmd_status(fd, res_buffer);
    /* if any error happens */
    if(status / 100 == 5)
        return 1;
    fclose(fp);
    /* 发送GET命令给ftp服务器 */
    send_ftpcmd(fd, cmd, args);
    status = get_ftpcmd_status(fd, res_buffer);

    /* if any error happens */
    if(status / 100 == 5)
        return 1;

    sockfd_act = make_conn_active(sockfd_act_listen);
    if(sockfd_act == -1)
        return -1;

    gettimeofday(&start_time, NULL);
    fz = download_file(args, sockfd_act);

    close(sockfd_act);
    close(sockfd_act_listen);

    if(!fz)
        /* 文件下载过程中被用户ctrl+z终止 */
        return 1;

    status = print_final_msg(fd, res_buffer);
out:
    gettimeofday(&finish_time, NULL);
    report(&start_time, &finish_time, fz);

    data_flag = 0;
    return 0;
}

int do_mget_active(int fd,char *cmd ,char *args)
{
    int i,ret;
    int sockfd_act;   				/* active mode socket for ftp data transfer */
    int sockfd_act_listen;     		/* active mode socket listening for ftp data connection */
    int status,number_of_arg=arg_count(args);
    char *p;
    char res_buffer[BUFSIZE];
    char *rec_list[number_of_arg];
    char *saveptr1,*saveptr2;
    p=strtok_r(args," ",&saveptr1);
    if(current_type==BINARY_MODE)
    {
        send_ftpcmd(fd,"TYPE A",NULL);
        current_type=ASCII_MODE;
        status = get_ftpcmd_status(fd, res_buffer);
        if(status != 200)
            return 1;
    }
    for(i=0; i<number_of_arg; i++)
    {
        if((rec_list[i]=(char *)malloc(sizeof(char)*BUFSIZE))==NULL)
        {
            perror("memory allocate error");
            return 1;
        }
        bzero(rec_list[i],sizeof(rec_list[i]));
        if(i>0)p=strtok_r(NULL," ",&saveptr1);
        data_flag = 1;
        /* send PORT command to the server */
        sockfd_act_listen = active_notify(fd);
        if(!sockfd_act_listen)
            return -1;             /* PORT cmd fails */

        /* 发送LIST命令给ftp服务器 */
        send_ftpcmd(fd, cmd, p);
        status = print_final_msg(fd, res_buffer);
        /* if any error happens */
        if(status / 100 == 5)
            return 1;
        sockfd_act = make_conn_active(sockfd_act_listen);
        if(sockfd_act == -1)
            return -1;
        ret = list_files(sockfd_act,rec_list[i]);
        close(sockfd_act);
        close(sockfd_act_listen);
        if(!ret)
            return 1;/* 在list命令过程中，用户输入了CTRL+Z */
        status = print_final_msg(fd, res_buffer);
        data_flag = 0;
    }
    for(i=0; i<number_of_arg; i++)
    {
        p=strtok_r(rec_list[i],"\r\n",&saveptr2);
        while(p)
        {
            bzero(res_buffer,sizeof(res_buffer));
            printf("mget %s?",p);
            fgets(res_buffer,sizeof(res_buffer),stdin);
            if(!strcmp(res_buffer,"y\n")||!strcmp(res_buffer,"Y\n")||!strcmp(res_buffer,"YES\n")||!strcmp(res_buffer,"yes\n")||!strcmp(res_buffer,"\n"))
            {
                status=do_get_active(fd,"RETR",p);
                if(status !=0)
                {
                    p=strtok_r(NULL,"\r\n",&saveptr2);
                    continue;
                }
            }
            else if(!strcmp(res_buffer,"n\n")||!strcmp(res_buffer,"N\n")||!strcmp(res_buffer,"no\n")||!strcmp(res_buffer,"NO\n"))
            {
                p=strtok_r(NULL,"\r\n",&saveptr2);
                continue;
            }
            else continue;
            p=strtok_r(NULL,"\r\n",&saveptr2);
        }

    }
    return 0;
}


int do_put_pasv(int fd, char *cmd, char *args)
{
    int port_pasv;
    char res_buffer[BUFSIZE],temp[BUFSIZE];
    int status;
    int fz,remote_fz=-1,local_fz;
    int sockfd_pasv;
    struct timeval 	start_time, finish_time;
    char cur_dir[256],*saveptr,*token;
    FILE *fp;

    data_flag = 1;

    memset(cur_dir, 0, sizeof(cur_dir));
    memcpy(cur_dir, "./", strlen("./"));

    strcat(cur_dir, args);
    if((fp=fopen(cur_dir, "r")) == NULL)
    {
        printf("file %s not exists\n", cur_dir);
        return 1;
    }

    /* 向ftp服务器发送pasv命令 */
    port_pasv = passive_notify(fd);
    if(!port_pasv)
        return -1;                               /* PASV cmd fails */

    /* 连接到ftp服务器 */
    sockfd_pasv = make_conn_passive(port_pasv);
    if(current_type==BINARY_MODE)
    {
        send_ftpcmd(fd,"TYPE A",NULL);
        current_type=ASCII_MODE;
        status = get_ftpcmd_status(fd, res_buffer);
        if(status != 200)
            return 1;
    }
    /* 发送LIST命令给ftp服务器 */
    send_ftpcmd(fd, "LIST", args);
    status = get_ftpcmd_status(fd, res_buffer);
    if(status / 100 == 5)
        return 1;                  /* LIST cmd fails */
    bzero(res_buffer,sizeof(res_buffer));
    read(sockfd_pasv,res_buffer,sizeof(res_buffer));
    status = get_ftpcmd_status(fd, temp);
    if(status / 100 == 5)
        return 1;
    if(strlen(res_buffer))
    {
        strtok_r(res_buffer," ",&saveptr);
        strtok_r(NULL," ",&saveptr);
        strtok_r(NULL," ",&saveptr);
        strtok_r(NULL," ",&saveptr);
        token=strtok_r(NULL," ",&saveptr);
        if(token)
        {
            remote_fz=atoi(token);
        }
    }

    if(current_type==ASCII_MODE)
    {
        send_ftpcmd(fd,"TYPE I",NULL);
        current_type=BINARY_MODE;
        status = get_ftpcmd_status(fd, res_buffer);
        if(status != 200)
            return 1;
    }
    /* 向ftp服务器发送pasv命令 */
    port_pasv = passive_notify(fd);
    if(!port_pasv)
        return -1;                               /* PASV cmd fails */

    /* 连接到ftp服务器 */
    sockfd_pasv = make_conn_passive(port_pasv);
    if(remote_fz==-1)
    {
        /* 发送PUT命令给ftp服务器 */
        send_ftpcmd(fd, cmd, args);
    }
    else
    {
        fseek(fp,0,SEEK_END);
        local_fz=ftell(fp);
        if(remote_fz>local_fz)send_ftpcmd(fd,cmd,args);
        send_ftpcmd(fd,"APPE",args);
        fseek(fp,remote_fz,SEEK_SET);
    }
    status = get_ftpcmd_status(fd, res_buffer);

    /* if any error happens */
    if(status / 100 == 5)
        return 1;

    gettimeofday(&start_time, NULL);
    fz=upload_file(fp, sockfd_pasv);
    close(sockfd_pasv);
    if(!fz)
        /* 文件下载过程中被用户ctrl+z终止 */
        return 1;
    status = print_final_msg(fd, res_buffer);

out:       /**/
    gettimeofday(&finish_time, NULL);
    report(&start_time, &finish_time, fz);

    data_flag = 0;
    return 0;
}

int do_mput_pasv(int fd,char *cmd,char *args)
{
    int port_pasv,sockfd_pasv,i,ret;
    char res_buffer[BUFSIZE];
    int status,number_of_arg=arg_count(args);
    char *p;
    char *saveptr;
    p=strtok_r(args," ",&saveptr);
    for(i=0; i<number_of_arg; i++)
    {
        bzero(res_buffer,sizeof(res_buffer));
        printf("mput %s?",p);
        fgets(res_buffer,sizeof(res_buffer),stdin);
        if(!strcmp(res_buffer,"y\n")||!strcmp(res_buffer,"Y\n")||!strcmp(res_buffer,"YES\n")||!strcmp(res_buffer,"yes\n")||!strcmp(res_buffer,"\n"))
        {
            status=do_put_pasv(fd,cmd,p);
            if(status !=0)
            {
                p=strtok_r(NULL," ",&saveptr);
                continue;
            }
        }
        else if(!strcmp(res_buffer,"n\n")||!strcmp(res_buffer,"N\n")||!strcmp(res_buffer,"no\n")||!strcmp(res_buffer,"NO\n"))
        {
            p=strtok_r(NULL," ",&saveptr);
            continue;
        }
        else continue;
        p=strtok_r(NULL," ",&saveptr);
    }




}

int do_put_active(int fd, char *cmd, char *args)
{
    char res_buffer[BUFSIZE],temp[BUFSIZE];
    int status;
    int fz,remote_fz=-1,local_fz;
    int sockfd_act_listen;     		/* active mode socket listening for ftp data connection */
    int sockfd_act;   				/* active mode socket for ftp data transfer */
    FILE *fp;
    struct timeval 	start_time, finish_time;
    char cur_dir[256],*saveptr,*token;

    data_flag = 1;

    memset(cur_dir, 0, sizeof(cur_dir));
    memcpy(cur_dir, "./", strlen("./"));

    strcat(cur_dir, args);
    if((fp=fopen(cur_dir, "r")) == NULL)
    {
        printf("file %s not exists\n", cur_dir);
        return 1;
    }
       if(current_type==BINARY_MODE)
    {
        send_ftpcmd(fd,"TYPE A",NULL);
        current_type=ASCII_MODE;
        status = get_ftpcmd_status(fd, res_buffer);
        if(status != 200)
            return 1;
    }
     /* send PORT command to the server */
    sockfd_act_listen = active_notify(fd);

    if(!sockfd_act_listen)
        return -1;                               /* PORT cmd fails */
    send_ftpcmd(fd, "LIST", args);
    status = get_ftpcmd_status(fd, res_buffer);
    if(status / 100 == 5)
        return 1;                  /* LIST cmd fails */
    sockfd_act = make_conn_active(sockfd_act_listen);
    if(sockfd_act == -1)
        return -1;
    bzero(res_buffer,sizeof(res_buffer));
    read(sockfd_act,res_buffer,sizeof(res_buffer));
    status = get_ftpcmd_status(fd, temp);
    if(status / 100 == 5)
        return 1;
    if(strlen(res_buffer))
    {
        strtok_r(res_buffer," ",&saveptr);
        strtok_r(NULL," ",&saveptr);
        strtok_r(NULL," ",&saveptr);
        strtok_r(NULL," ",&saveptr);
        token=strtok_r(NULL," ",&saveptr);
        if(token)
        {
            remote_fz=atoi(token);
        }
    }
        if(current_type==ASCII_MODE)
    {
        send_ftpcmd(fd,"TYPE I",NULL);
        current_type=BINARY_MODE;
        status = get_ftpcmd_status(fd, res_buffer);
        if(status != 200)
            return 1;
    }
    /* send PORT command to the server */
    sockfd_act_listen = active_notify(fd);

    if(!sockfd_act_listen)
        return -1;                               /* PORT cmd fails */

        if(remote_fz==-1)
    {
        /* 发送PUT命令给ftp服务器 */
        send_ftpcmd(fd, cmd, args);
    }
    else
    {
        fseek(fp,0,SEEK_END);
        local_fz=ftell(fp);
        if(remote_fz>local_fz)send_ftpcmd(fd,cmd,args);
        send_ftpcmd(fd,"APPE",args);
        fseek(fp,remote_fz,SEEK_SET);
    }
    status = get_ftpcmd_status(fd, res_buffer);

    /* if any error happens */
    if(status / 100 == 5)
        return 1;

    sockfd_act = make_conn_active(sockfd_act_listen);
    if(sockfd_act == -1)
        return -1;

    gettimeofday(&start_time, NULL);
    fz = upload_file(fp, sockfd_act);
    close(sockfd_act);

    if(!fz)
        /* 文件下载过程中被用户ctrl+z终止 */
        return 1;
    status = print_final_msg(fd, res_buffer);

out:
    gettimeofday(&finish_time, NULL);
    report(&start_time, &finish_time, fz);
    data_flag = 0;
    return 0;
}


int do_mput_active(int fd,char *cmd,char *args)
{
    int port_pasv,sockfd_pasv,i,ret;
    char res_buffer[BUFSIZE];
    int status,number_of_arg=arg_count(args);
    char *p;
    char *saveptr;
    p=strtok_r(args," ",&saveptr);
    for(i=0; i<number_of_arg; i++)
    {
        while(p)
        {
            bzero(res_buffer,sizeof(res_buffer));
            printf("mput %s?",p);
            fgets(res_buffer,sizeof(res_buffer),stdin);
            if(!strcmp(res_buffer,"y\n")||!strcmp(res_buffer,"Y\n")||!strcmp(res_buffer,"YES\n")||!strcmp(res_buffer,"yes\n")||!strcmp(res_buffer,"\n"))
            {
                status=do_put_active(fd,cmd,p);
                if(status !=0)
                {
                    p=strtok_r(NULL," ",&saveptr);
                    continue;
                }
            }
            else if(!strcmp(res_buffer,"n\n")||!strcmp(res_buffer,"N\n")||!strcmp(res_buffer,"no\n")||!strcmp(res_buffer,"NO\n"))
            {
                p=strtok_r(NULL," ",&saveptr);
                continue;
            }
            else continue;
            p=strtok_r(NULL," ",&saveptr);
        }

    }

}

int do_mdel_pasv(int fd,char *cmd,char *args)
{

    int port_pasv,sockfd_pasv,i,ret;
    char res_buffer[BUFSIZE];
    int status,number_of_arg=arg_count(args);
    char *rec_list[number_of_arg];
    char *p;
    char *saveptr1,*saveptr2;
    p=strtok_r(args," ",&saveptr1);
    if(current_type==BINARY_MODE)
    {
        send_ftpcmd(fd,"TYPE A",NULL);
        current_type=ASCII_MODE;
        status = get_ftpcmd_status(fd, res_buffer);
        if(status != 200)
            return 1;
    }
    for(i=0; i<number_of_arg; i++)
    {
        if((rec_list[i]=(char *)malloc(sizeof(char)*BUFSIZE))==NULL)
        {
            perror("memory allocate error");
            return 1;
        }
        bzero(rec_list[i],sizeof(rec_list[i]));
        if(i>0)p=strtok_r(NULL," ",&saveptr1);
        data_flag = 1;
        /* 向ftp服务器发送pasv命令*/
        port_pasv = passive_notify(fd);

        if(!port_pasv)
        {
            return -1;                               /* PASV cmd fails*/
        }
        /* 连接到ftp服务器*/
        sockfd_pasv = make_conn_passive(port_pasv);
        send_ftpcmd(fd, cmd, p);
        status = get_ftpcmd_status(fd, res_buffer);
        if(status / 100 == 5)return 1;
        ret = list_files(sockfd_pasv,rec_list[i]);
        close(sockfd_pasv);
        if(!ret)return 1;
        print_final_msg(fd, res_buffer);
        data_flag = 0;
    }

    for(i=0; i<number_of_arg; i++)
    {
        p=strtok_r(rec_list[i],"\r\n",&saveptr2);
        while(p)
        {
            bzero(res_buffer,sizeof(res_buffer));
            printf("mdelete %s?",p);
            fgets(res_buffer,sizeof(res_buffer),stdin);
            if(!strcmp(res_buffer,"y\n")||!strcmp(res_buffer,"Y\n")||!strcmp(res_buffer,"YES\n")||!strcmp(res_buffer,"yes\n")||!strcmp(res_buffer,"\n"))
            {
                send_ftpcmd(fd, "DELE", p);
                status = get_ftpcmd_status(fd, res_buffer);
                if(status != 250)
                {
                    p=strtok_r(NULL,"\r\n",&saveptr2);
                    continue;
                }
            }
            else if(!strcmp(res_buffer,"n\n")||!strcmp(res_buffer,"N\n")||!strcmp(res_buffer,"no\n")||!strcmp(res_buffer,"NO\n"))
            {
                p=strtok_r(NULL,"\r\n",&saveptr2);
                continue;
            }
            else continue;
            p=strtok_r(NULL,"\r\n",&saveptr2);
        }

    }


    return 0;
}
int do_mdel_active(int fd,char *cmd,char *args)
{
    char res_buffer[BUFSIZE];
    int status,number_of_arg=arg_count(args);
    int sockfd_act_listen;     		/* active mode socket listening for ftp data connection */
    int sockfd_act;   				/* active mode socket for ftp data transfer */
    int ret,i;
    char *rec_list[number_of_arg];
    char *p;
    char *saveptr1,*saveptr2;
    p=strtok_r(args," ",&saveptr1);
    if(current_type==BINARY_MODE)
    {
        send_ftpcmd(fd,"TYPE A",NULL);
        current_type=ASCII_MODE;
        status = get_ftpcmd_status(fd, res_buffer);
        if(status != 200)
            return 1;
    }
    for(i=0; i<number_of_arg; i++)
    {
        if((rec_list[i]=(char *)malloc(sizeof(char)*BUFSIZE))==NULL)
        {
            perror("memory allocate error");
            return 1;
        }
        bzero(rec_list[i],sizeof(rec_list[i]));
        if(i>0)p=strtok_r(NULL," ",&saveptr1);
        data_flag = 1;
        /* send PORT command to the server */
        sockfd_act_listen = active_notify(fd);
        if(!sockfd_act_listen)
            return -1;             /* PORT cmd fails */

        /* 发送LIST命令给ftp服务器 */
        send_ftpcmd(fd, cmd, p);
        status = print_final_msg(fd, res_buffer);
        /* if any error happens */
        if(status / 100 == 5)
            return 1;
        sockfd_act = make_conn_active(sockfd_act_listen);
        if(sockfd_act == -1)
            return -1;
        ret = list_files(sockfd_act,rec_list[i]);
        close(sockfd_act);
        close(sockfd_act_listen);
        if(!ret)
            return 1;/* 在list命令过程中，用户输入了CTRL+Z */
        status = print_final_msg(fd, res_buffer);
        data_flag = 0;

    }

    for(i=0; i<number_of_arg; i++)
    {
        p=strtok_r(rec_list[i],"\r\n",&saveptr2);
        while(p)
        {
            bzero(res_buffer,sizeof(res_buffer));
            printf("mdelete %s?",p);
            fgets(res_buffer,sizeof(res_buffer),stdin);
            if(!strcmp(res_buffer,"y\n")||!strcmp(res_buffer,"Y\n")||!strcmp(res_buffer,"YES\n")||!strcmp(res_buffer,"yes\n")||!strcmp(res_buffer,"\n"))
            {
                send_ftpcmd(fd, "DELE", p);
                status = get_ftpcmd_status(fd, res_buffer);
                if(status != 250)
                {
                    p=strtok_r(NULL,"\r\n",&saveptr2);
                    continue;
                }
            }
            else if(!strcmp(res_buffer,"n\n")||!strcmp(res_buffer,"N\n")||!strcmp(res_buffer,"no\n")||!strcmp(res_buffer,"NO\n"))
            {
                p=strtok_r(NULL,"\r\n",&saveptr2);
                continue;
            }
            else continue;
            p=strtok_r(NULL,"\r\n",&saveptr2);
        }

    }



    return 0;


}
int do_lchdir(int fd, char *cmd, char *args)
{
    char *cd;
    char buf[256];

    memset(buf, 0, sizeof(buf));

    strtok(cmd, " ");
    cd = strtok(NULL, cmd);

    if(!cd)
    {
        getcwd(buf, sizeof(buf));
        printf("Local directory now %s\n", buf);
    }
    else if((chdir(cd)) == -1)
    {
        bail("Change dir ");
        return -1;
    }
    else
        printf("Local directory now %s\n", cd);

    return 0;
}

void ignore_sigtstp()
{
    struct sigaction abort_action;

    memset(&abort_action, 0, sizeof(abort_action));
    abort_action.sa_flags = 0;

    if(sigaction(SIGTSTP, NULL, &abort_action) == -1)
        perror("Failed to get old handler for SIGTSTP");

    abort_action.sa_handler = SIG_IGN;
    if (sigaction(SIGTSTP, &abort_action, NULL) == -1)
        perror("Failed to ignore SIGTSTP");
}

void unignore_sigtstp()
{
    struct sigaction abort_action;

    memset(&abort_action, 0, sizeof(abort_action));

    if(sigaction(SIGTSTP, NULL, &abort_action) == -1)
        perror("Failed to get old handler for SIGTSTP");

    abort_action.sa_handler = &abort_transfer;
    if (sigaction(SIGTSTP, &abort_action, NULL) == -1)
        perror("Failed to unignore SIGTSTP");
}

char *get_usrname()
{
    struct passwd *ppasswd;

    ppasswd= getpwuid(getuid());
    return ppasswd->pw_name;
}

void terminal_echo_off(int fd)
{
    struct termios old_terminal;
    tcgetattr(fd, &old_terminal);
    old_terminal.c_lflag &= ~ECHO;
    tcsetattr(fd, TCSAFLUSH, &old_terminal);
}

void terminal_echo_on(int fd)
{
    struct termios old_terminal;
    tcgetattr(fd, &old_terminal);
    old_terminal.c_lflag |= ECHO;
    tcsetattr(fd, TCSAFLUSH, &old_terminal);
}

void abort_transfer (int signal_number)
{
    char res_buffer[BUFSIZE];
    int status;

    if(data_flag)     /* 信号发生时正在传输数据 */
    {
        ctrl_z = 1;
        send_ftpcmd(sockfd_cmd, "ABOR", NULL);
        /* for 426 abort command successful */
        status = get_ftpcmd_status(sockfd_cmd, res_buffer);

    }
}

void bail(const char *on_what)
{
    fputs(strerror(errno), stderr);
    fputs(": ", stderr);
    fputs(on_what, stderr);
    fputc('\n', stderr);
}
int print_final_msg(int fd, char *res_buffer)
{
    char code[5], *msg;
    int status;

    status = -1;
    while(msg = strtok(NULL, "\r\n"))
    {
        printf("%s\n", msg);
        memset(code, 0, sizeof(code));
        status = atoi(strncpy(code, msg, 4));
        if(code[3] != ' ')
        {
            status = -1;
        }
    }
    if(status > 0)
    {
        return status;
    }
    else
    {
        return get_ftpcmd_status(fd, res_buffer);
    }
}
