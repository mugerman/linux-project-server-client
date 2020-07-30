#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <signal.h>
#include <poll.h>

#define PORT "9034"  // the port users will be connecting to

#define BACKLOG 10   // how many pending connections queue will hold

#define MAXDATASIZE 150 // max number of bytes we can get at once

#define TIMEOUT 10000 //milisec

#define PATH "client_data.txt"

void client_txt(int new_fd,float temperature);
void sigchld_handler(int s);
void *get_in_addr(struct sockaddr *sa);



int main(void)
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    char id[15];


    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("\nserver: got connection from %s\n", s);




	//forkkkk
	if (!fork()){ // this is the child process
        close(sockfd); // child doesn't need the listene
        if (send(new_fd, "Hello\n", 7, 0) == -1)
        perror("send");
	send(new_fd, "Hello\n", 7, 0);
	int numbytes;
	char buf[MAXDATASIZE];

	//read massage with temp
	numbytes = read(new_fd,buf,MAXDATASIZE-1);
	buf[numbytes] = '\0';
	printf("%s\n",buf);

	//conv the temp and do case
	float temperature;
	char temperature_str[6];
	strtok(buf,":");
	strcpy(temperature_str,strtok(NULL,":"));
	temperature = (float)atof(temperature_str);
	if (temperature<37){
		strcpy(buf , "Your temperature is fine\n");
		write(new_fd, buf, strlen(buf));
	}
	else if (temperature>37 && temperature<38){
		strcpy (buf , "You have heat, check your temperature in half hour\n");
		write(new_fd, buf, strlen(buf));
	}
	else{
		strcpy(buf , "You have high heat. you will recieve a few questions to deny COVID 19\nplease enter your ID:");
		write(new_fd, buf, strlen(buf));
		client_txt(new_fd,temperature);
	}

            printf("The socket closed!\n");
            close(new_fd);
            exit(0);
        }//end fork
        close(new_fd);  // parent doesn't need this
 }

    return 0;
}




void client_txt(int new_fd,float temperature)
{
	char buf[MAXDATASIZE];
    char if_weakness[2];
    char if_coughing[2];
    char if_headaches[2];

    struct pollfd fds[1];
    fds[0].fd = new_fd;
    fds[0].events = 0;
    fds[0].events |= POLLIN;

    fd_set readfds;
    struct timeval timeout_select;

    char timeout_massage[60]="You have waited for too long, please try again! timeout";

    if (!poll(fds,1,TIMEOUT)){
        printf("timeout\n");
        send(new_fd,timeout_massage,strlen(timeout_massage),0);
        close(new_fd);
        exit(0);
    }
    else{
    printf("waiting for client's ID\n");

	//read ID
	int numbytes = read(new_fd,buf,MAXDATASIZE-1);
	buf[numbytes] = '\0';
    char cID[10];
    strcpy(cID,buf);
    printf("Client ID: %s\n",buf);
    printf("Questions were sent to the client, waiting for an answer\n");


    strcpy(buf , "Your heat is too high, you have to answer 3 questions:\nDo you feel weakness? (y or n)");
    write(new_fd, buf, strlen(buf));

    FD_ZERO(&readfds);
    FD_SET(new_fd,&readfds);
    timeout_select.tv_sec = TIMEOUT / 1000; 
    timeout_select.tv_usec = 0;

    int sret = select(8,&readfds,NULL,NULL,&timeout_select);
    if(sret == 0)
    {
        printf("Client ID: %s - timeout\n",cID);
        send(new_fd,timeout_massage,strlen(timeout_massage),0);
        close(new_fd);
        exit(0);
    }
    else
    {
    numbytes = read(new_fd,if_weakness,2);
    if_weakness[numbytes-1] = '\0';
    printf("\nif_weakness: %s\n",if_weakness);
    

    strcpy(buf , "Are you coughing? (y or n)");
    write(new_fd, buf, strlen(buf));
    numbytes = read(new_fd,if_coughing,2);
    if_coughing[numbytes-1] = '\0';
    printf("if_coughing: %s\n",if_coughing);
    
    strcpy(buf , "Do you have headaches? (y or n)");
    write(new_fd, buf, strlen(buf));
    numbytes = read(new_fd,if_headaches,2);
    if_headaches[numbytes-1] = '\0';
    printf("if_headaches: %s\n",if_headaches);

    }

    // create text file
    FILE *fp;
    int i = 0;
    remove(PATH);
    fp = fopen(PATH, "w+");
    fprintf(fp, "client ID: %-12s \ntemperature: %-.1f \nweakness: %-2s \ncoughing: %-2s \nheadaches: %-2s",cID, temperature, if_weakness, if_coughing, if_headaches);
    fclose(fp);
    printf("\nText file has created and located at /home/client_data.txt\n\n");
    }

}



void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

