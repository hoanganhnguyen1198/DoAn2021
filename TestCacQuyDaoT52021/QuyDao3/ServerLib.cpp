#include "ServerLib.h"

ServerSocket::ServerSocket(){};
ServerSocket::~ServerSocket(){};

void ServerSocket::ServerInit()
{
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Attach socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Allow maximum 3 clients to connect to the server
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Establish a connection with the first request on the queue of pending connection
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
}
float* ServerSocket::GetCoordinates(float last_x, float last_y)
{
    int valread;
    char buffer[1024] = {0};
    float* p=new float[3]; //2
    valread = read(new_socket, buffer, 1024);
		char *s1=strtok(buffer," ");
		p[0] = atof(s1);
		char *s2=strtok(NULL," ");
		p[1]=atof(s2);
        /*
		char *s3=strtok(NULL," ");
		p[2]=atof(s3);
        */
        p[2]=(float)atan((p[1]-last_y)/( p[0] - last_x));
    memset(buffer, 0, sizeof(buffer));
    return p;
}
void ServerSocket::start()
{
    while(1)
    {
        int check;
        char buffer[1024] = {0};
        memset(buffer, 0, sizeof(buffer));
		check = read(new_socket, buffer, 1024);
		 if (strcmp(buffer,"start")==0)
				{break;}
    }
}
int ServerSocket::CheckStop()
{
    int p;
    int check;
    char buffer[1024] = {0};
        memset(buffer, 0, sizeof(buffer));
		check = read(new_socket, buffer, 1024);
		 if (strcmp(buffer,"tat")==0)
				{p=1;}
         else
         {p=2;}
    return p;
}
