//created by Tomáš Strych at 10.4.2017
//chat client, usage: ./chat_client -u user -i ip_address, program ends on ctrl-c;

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <signal.h>


#define BUFSIZE 1024
//buffer size is size of receiving socket
int sock_f_d; //name of the socket
char * user=NULL; //user from arguments

using namespace std;

struct struct_arg {
    char *i=NULL; //ip address
    char *u=NULL; //user
};  //load arguments from terminal line

struct param {
    int socket;
    char rec_buffer[BUFSIZE];
}; //temp structure params

//error function print on standard error *msg (message) and exit program with with error_code
void error(const char *msg,int error_code)
{
    fprintf (stderr, "%s\n",msg);
    exit(error_code);
}

//function get arguments from terminal into the structure
void get_arguments(int argc, char *argv[], struct struct_arg *structure) {
    int c;
    while ((c = getopt(argc, argv, "u:i:")) != -1) {
        switch (c) {
            case 'u':
                structure->u = optarg; //set user
                break;
            case 'i':
                structure->i = optarg; //set ip address
                break;
            case '?':
            default:
                /* invalid option */
                error("invalid arguments", 1);
                break;
        }
    }
}

//when ctrl-c (sigint) comes then this send logout message to server close socket and ends program
void sig_int_handle(int){

    string log_o = " logged out\r\n";
    string log_out_mess = user + log_o;

    if (send(sock_f_d, log_out_mess.c_str(), strlen(log_out_mess.c_str()), 0) < 0)
        error("ERROR: send failed!", 1);

    close(sock_f_d);
    exit(0);
}

//receive function work in second thread, in endless loop receive massage
// in pointer to p, is saved struct param, but when you work with threads is necessary to do it in this way
void * receive(void * p) {
    struct param *par = (struct param *)p;
    while (1) {
        if (recv(par->socket, par->rec_buffer, BUFSIZE, 0) < 0)
            error("ERROR reading from socket\n", 1);
        cout << par->rec_buffer;    //print message to standard output
        bzero(par->rec_buffer, BUFSIZE); //clear buffer
    }
    return NULL; //never happens
}

//int is_empty(const char *s) {
//    while (*s != '\0') {
//        if (!isspace(*s))
//            return 0;
//        s++;
//    }
//    return 1;
//}

//control if content is fill with white space
int is_string_empty(const char * content) {
    int only_spaces = 1;    //content
        for (const char * y = content; *y != '\0'; ++y)
        {
            if (*y != '\n') if (*y != '\t') if (*y != '\r') if (*y != ' ') if (*y != '\v') if (*y != '\f'){
                only_spaces = 0;    //if content only from blank characters then 0
                break;
            }
        }
    return only_spaces;
}


int main(int argc, char* argv[]) {

    // ------------------- arguments ---------------------//
    struct struct_arg arg;
    if (argc != 5) {
        error("bad number of arguments", 1);
    }

    //load arguments to structure arg
    get_arguments(argc, argv, &arg);
    //control if all parameters are set, here is not any optional parameter
    if (arg.i == NULL or arg.u ==  NULL)
        error("bad arguments", 1);

    // ------------------- connection ---------------------//

    int port_number = 21011; //every time same port number
    struct sockaddr_in server_address;
    struct hostent *server;
    char recv_buffer[BUFSIZE];

    if ((sock_f_d = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
        error("Error: Create socket error.",1);

    std::string str(arg.i);
    string hostname = str;

    server = gethostbyname(hostname.c_str()); //set from arguments
    if (server == NULL)
        error("ERROR: No such host\n",1);

    //clear server address
    bzero((char *) &server_address, sizeof(server_address));
    //set server address
    server_address.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&server_address.sin_addr.s_addr, server->h_length);
    server_address.sin_port = htons(port_number);


    // connect
    if (connect(sock_f_d, (struct sockaddr *) &server_address, sizeof(server_address)) != 0)
        error("ERROR: connect",1);


    bzero(recv_buffer, BUFSIZE);

    //initialize thread
    pthread_t pthread;
    //set parameters to the second thread function receive
    struct param params;
    memcpy(params.rec_buffer, recv_buffer,BUFSIZE);
    params.socket = sock_f_d;
    user = arg.u;


    //handle ctrl-c
    signal(SIGINT, sig_int_handle);

    //create second thread
    if(pthread_create(&pthread, NULL, receive, &params)) {
        error("ERROR: creating thread", 1);
    }

    //send logged in message
    string log_i = " logged in\r\n";
    string log_in_mess = arg.u + log_i;
    if( send(sock_f_d , log_in_mess.c_str(), strlen(log_in_mess.c_str()), 0) < 0)
        error("ERROR: send failed!\n",1);

    string colon= ": "; //constant

    //send messages end by end of line to the server
    //format of message user_name: "content\r\n"
    while(1) {
        string str = arg.u + colon; //user + constant
        string line = "";

        getline(std::cin, line); //read string which ends with end of line into line
        if (!line.empty()) {
            if (!is_string_empty(line.c_str())) { //don' t send empty strings
                str += line; //add content to send message
                str.append("\r\n");
                if (send(sock_f_d, str.c_str(), strlen(str.c_str()), 0) < 0)
                    error("ERROR: send failed!", 1);
            }
        }
    }
}