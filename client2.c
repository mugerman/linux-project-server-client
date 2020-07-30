#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <math.h>
#include <arpa/inet.h>

#define PORT "9034" // the port client will be connecting to

#define MAXDATASIZE 150 // max number of bytes we can get at once

float check_temperature();
char check_ans(char *tmp);
void recieving_and_sending_questions(int sockfd);

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd, numbytes;
    char ans, tmp[10], buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];


    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

	//revice HELLO
      if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
          perror("recv");
          exit(1);
      }
 	buf[numbytes] = '\0';
	printf("%s",buf);

	//send massage with temp
	float temperature = check_temperature();
	char temperature_converted[6];
	gcvt(temperature,4,temperature_converted);
	char massage[30] ="My temperature is : " ;
	strcat (massage,temperature_converted);
	write(sockfd, massage ,strlen(massage));

	//server's response
	numbytes = read(sockfd,buf ,MAXDATASIZE-1);
	buf[numbytes] = '\0';
	printf("%s\n",buf);

	if(temperature<38) exit(1);

	//get and send ID
	gets(buf);
	while((strlen(buf) == 0) || (strlen(buf) > 9))
	{
	printf("Enter new ID: ");
	gets(buf);
	printf("\n");
	}
	write(sockfd, buf ,strlen(buf));

    recieving_and_sending_questions(sockfd);
    recieving_and_sending_questions(sockfd);
    recieving_and_sending_questions(sockfd);


    printf("\nThe answers were sent and the socket closed!\n\n");



}

float check_temperature()
{
	int temp1;
	float temp2,ret;
	time_t  t;

	srand((unsigned) time(&t));

	temp1 = rand();
	temp2 = temp1%100;
	ret = (temp2/100)*4 + pow(6,2);
	printf("My temperature is: %.1lf degrees \n",ret);
	return ret;
}


char check_ans(char *tmp)
{
    char *t;
    char ss[2] = "-";
    tmp[1]='-';
    t = strtok(tmp,ss);
    while ((strcmp(t,"y"))&&(strcmp(t,"n"))) 
    {
    printf("invalid value\n");
    scanf("%s",tmp);
    tmp[1]='-';
    t = strtok(tmp,ss);
    }
return *t;
}


void recieving_and_sending_questions(int sockfd)
{
    int numbytes;
    const char a = '!';
    char *ch, ans, tmp[10], buf[MAXDATASIZE];
    numbytes = read(sockfd,buf ,MAXDATASIZE-1);
    buf[numbytes] = '\0';
    printf("\n%s ",buf);
    ch = strchr(buf,a);
    if(ch != NULL)
        if(!strcmp(ch,"! timeout"))
        {
            close(sockfd);
            printf("\n\n");
            exit(1);
        }
    
    scanf("%s",&tmp);
    ans = check_ans(tmp);
    write(sockfd, &ans, 2 );
}