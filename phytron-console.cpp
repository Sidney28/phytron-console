#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>

char ctrln='0';

void printx(const char * string);

char crc(const char * string);

void print_response(const char * res);

int main(int argc, const char ** argv){
	char buffer1[FILENAME_MAX];
	char buffer2[FILENAME_MAX];

	int sockd = 0;
	int port = 0;
	int readc = 0;

	sockaddr_in serv_addr;
	hostent * server;

	if( argc > 3 || argc < 2 ){
		fprintf(stderr, "Usage: %s IP [PORT]", argv[0]);
		exit(-1);
	}
	if( argc == 3 )
	{
		port = atoi(argv[2]);
	}else{
		port = 22222;
	}
		
	sockd = socket(AF_INET, SOCK_STREAM, 0);
	if ( sockd < 0){
		perror("Fail to create socket");
		exit(-1);
	}
	
	server = gethostbyname(argv[1]);
	if( server == NULL ) {
		fprintf(stderr, "No such host");
		exit(-1);
	}
	
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(port);

	if (connect(sockd,(sockaddr*) &serv_addr, sizeof(serv_addr)) < 0){
		  perror("Failed to connetct the server");
		  exit(-1);
	}

	while(true){
		printf(" >>> ");
		if(fgets(buffer1,FILENAME_MAX,stdin)==NULL){
			printf("\n");
			exit(0);
		}
		buffer1[FILENAME_MAX-1]=0;
		for(char * p = buffer1; *p!=0; p++){
			if(*p == '\n'){
			       	*p=0;
				break;
			}
		}
		if(buffer1[0]==0){
			printf("Empty string!\n");
			continue;
		}
		if(strcmp(buffer1,"EXIT") == 0 ){
			printf("Bye!\n");
			exit(0);
		}

		snprintf(buffer2,FILENAME_MAX,"\x02%c%s:%02X\x03",ctrln,buffer1,(int)(crc(buffer1)^':'^ctrln));
		//printf("request : \'");
		//printx(buffer2);
		//printf("\'\n");
		
		readc = write(sockd,buffer2,strlen(buffer2));
		if (readc < 0){
			perror("Failed to write socket");
			exit(-1);
		}

		bzero(buffer1,FILENAME_MAX);
		readc=read(sockd, buffer1,FILENAME_MAX);
		if(readc < 0){
			perror("Faild to read socket");
			exit(-1);
		}

		//printf("response : \'");
		//printx(buffer1);
		//printf("\'\n");
		print_response(buffer1);
	}
	return 0;
}


void printx(const char * string){
	for(const char *p = string; *p != 0 ; p++){
		if ( *p<0x20 ){
			printf("\\x%02X",*p);
		}else{
			printf("%c",*p);
		}
	}
}

char crc(const char * string){
	char ret = 0;
	for(const char *p = string; *p != 0; p++){
		ret ^= *p;
	}
	return ret;
}

void print_response(const char * res){
	int index=0;
	if (res[index] != 0x02){
		fprintf(stderr, "ERROR: response start with none 0x02 char!\n");
		printx(res);
		printf("\n");
		return;
	}
	
	if(res[index] == 0){
		fprintf(stderr,"Enpty request\n");
		return;
	}

	index++;
	if (res[index] == 0x06) {
		if(res[index+1]!=':'){
			printf("OK : ");
		}else{
			printf("OK\n");
			return;
		}
	}else if(res[index] == 0){
		fprintf(stderr, "Enpty request\n");
		return;
	}else if(res[index] == 0x15){ 
		printf("ERR\n");
		return;
	}else{
		printf("0x%02x : ",(int)res[index]);
	}
	
	for(++index; res[index]!=0 && res[index]!=0x03; index++){
		if(res[index]==':'){
			index=index+3;
			continue;
		}
		printf("%c", res[index]);
	}

	printf("\n");
}
