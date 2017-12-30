#define BUFSIZE    2048
#define CMDSIZE    64
#define ARGSIZE    64
#define PASSIVE_ON 0x1

struct ftpcmd{
	char *alias;                                    /* ftp cmd alias */
	char *name;                                     /* ftp cmd name */
	char *args;                                     /* ftp cmd args */
	int  (*handler)(int fd, char *cmd, char *args); /* ftp cmd handler */
};

typedef struct ftpcmd FTPCMD;

static void bail (const char *);
char *trim_left(char *str);
int arg_count(char *args);
int send_ftpcmd (int, const char *, const char *);
char *get_localip (int, struct sockaddr_in *);
int active_listen();
int get_active_port(int);
int list_files(int,char *);
int input_cmd(char *, int);
int check_ftpcmd(char *, char *);
int having_args(char *);
char *trim_right(char *);
int make_port_args(int, struct sockaddr_in *);
int download_file(char *, int);
int upload_file(FILE *, int);
void report(struct timeval *, struct timeval *, int);
int get_ftpcmd_status(int, char *);
int print_final_msg(int, char *);/**/
int make_conn_active(int);
int make_conn_passive(int);
int passive_notify(int);
int active_notify(int);
void replace_delim(char *, char, char);
int parse_port(char *, int);
void init();
char *get_usrname();
void terminal_echo_off(int);
void terminal_echo_on(int);
void ignore_sigtstp();
void unignore_sigtstp();
int do_common_cmd(int, char *, char *);
int do_user(int, char *, char *);
int do_pasv(int, char *, char *);
int do_list_pasv(int, char *, char *);
int do_list_active(int, char *, char *);
int do_get_pasv(int, char *, char *);
int do_get_active(int, char *, char *);
int do_put_pasv(int, char *, char *);
int do_put_active(int, char *, char *);
int do_mdel_pasv(int,char*,char *);
int do_mdel_active(int ,char *,char *);
int do_mget_active(int,char *,char *);
int do_mget_pasv(int,char *,char *);
int do_mput_pasv(int,char *,char *);
int do_mput_active(int,char *,char *);
int do_lchdir(int, char *, char *);
void abort_transfer (int);
void login(struct hostent *, int);
