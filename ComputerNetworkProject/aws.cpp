#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <algorithm>
#include <map>
#include <string>
#include <cstring>
#include <unordered_set>
#include <ctype.h>
#include <iomanip>

#define UDP_PORT 23383
#define TCP_PORT 24383

using namespace std;
vector<string> input;
string id, source, size;
// UDP socket
int udp_sock_fd;
char recv_buffer[1024];
struct sockaddr_in udp_addr_server;
struct sockaddr_in udp_addr_client;
string input_to_B;
string back_to_client;

vector<string> split(string str,string pattern)
{
    string::size_type pos;
    vector<string> result;
    
    str+=pattern;
    int size=str.size();
    
    for(int i=0; i<size; i++)
    {
        pos=str.find(pattern,i);
        if(pos<size)
        {
            string s=str.substr(i,pos-i);
            result.push_back(s);
            i=pos+pattern.size()-1;
        }
    }
    return result;
}

void send_to_server(int port, string server_name, string message){
    memset(&udp_addr_server, 0, sizeof(udp_addr_server));
    udp_addr_server.sin_family = AF_INET;
    udp_addr_server.sin_port = htons(port);
    udp_addr_server.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    sendto(udp_sock_fd, message.c_str(), 1024, 0, (const struct sockaddr *)&udp_addr_server, sizeof(udp_addr_server));
    
    if(server_name == "A"){
        cout<<endl;
        cout << "The AWS has sent map ID and starting vertex to server A using UDP over port <";
        cout << UDP_PORT <<">."<<endl;
    }else{
        cout<<endl;
        cout << "The AWS has sent path length, propagation speed and transmission speed to server B using UDP over port <";
        cout << UDP_PORT <<">."<<endl;
    }
    
}


void receive_from_server_A(){
    char recv_A[1024];
    memset(recv_A, 0, sizeof(recv_A)); // clear
    socklen_t udp_client_len = sizeof(udp_addr_client);
    recvfrom(udp_sock_fd, recv_A, 1024, 0, (struct sockaddr *)&udp_addr_client, &udp_client_len);
    string res = recv_A;
//    cout << "receive from A : "<< res << endl;
    
    vector<string> input_from_A;
    input_from_A = split(res,";");
    vector<string> speed_verticeNum;
    speed_verticeNum = split(input_from_A[0],",");
    int vertice_num = stoi(speed_verticeNum[2]);
//    cout <<"vertice number: "<<vertice_num << endl;
//    cout << "speed and num : " << speed_verticeNum[0] << speed_verticeNum[1] << speed_verticeNum[2]<<endl;
    // speed_verticeNum[0] -> propogation speed
    // speed_verticeNum[1] -> transmission speed
    // speed_verticeNum[2] -> vertice number
    // input_from_A[1] stores destination + min length
    vector<string> dest_min = split(input_from_A[1],",");
    cout<< endl;
    cout << "The AWS has received shortest path from server A:"<<endl;
    cout << "-----------------------------"<<endl;
    cout << "Destination        Min Length"<<endl;
    cout << "-----------------------------"<<endl;
    int j = 0;
    for (int i = 0; i < vertice_num-1; i++){
        cout << dest_min[j];
        if(stoi(dest_min[j])>=10){
            cout << "                 ";
        }else{
            cout << "                  ";
        }
        cout<<dest_min[j+1]<<endl;
        j += 2;
    }
    cout << "-----------------------------"<<endl;
    
    input_to_B = res;
    input_to_B += size;
}

void receive_from_server_B(){
    char recv_B[1024];
    memset(recv_B, 0, sizeof(recv_B)); // clear
    socklen_t udp_client_len = sizeof(udp_addr_client);
    recvfrom(udp_sock_fd, recv_B, 1024, 0, (struct sockaddr *)&udp_addr_client, &udp_client_len);
    string res = recv_B;
//    cout << "receive from B : "<< res << endl;
    vector<string> input_from_B;
    input_from_B = split(res,";");
    int vertice_num = stoi(input_from_B[0]);
    vector<int> dest_store;
    vector<float> delay_trans;
    vector<float> delay_prop;
    vector<float> delay;
    vector<string> temp;
    int j = 1;
    for (int i = 0; i < vertice_num - 1; i++){
        temp = split(input_from_B[j],",");
        dest_store.push_back(stoi(temp[0]));
        delay_trans.push_back(stof(temp[1]));
        delay_prop.push_back(stof(temp[2]));
        delay.push_back(stof(temp[3]));
        temp.clear();
        j++;
    }
    
    cout << endl;
    cout << "The AWS has received delays from server B:" << endl;
    cout << "--------------------------------------------"<< endl;
    cout << "Destination        Tt        Tp        Delay"<< endl;
    cout << "--------------------------------------------"<< endl;
    for (int i = 0; i < vertice_num - 1; i++){
        cout << dest_store[i];
        if (dest_store[i]>=10){
            cout << "                 ";
        }else{
            cout << "                  ";
        }
        
        printf("%.2f",delay_trans[i]);
        cout << "    ";
        printf("%.2f",delay_prop[i]);
        if (delay_prop[i]>=10){
            cout << "     ";
        }else{
            cout << "      ";
        }
        
        printf("%.2f",delay[i]);
        cout << endl;
    }
    cout << "--------------------------------------------"<< endl;
    
    back_to_client = res;
}

int main(){
    // creat UDP socket
    udp_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock_fd == -1){
        perror("AWS UDP socket");
        exit(1);
    }
    memset(&udp_addr_server, 0, sizeof(sockaddr_in));  //每个字节都用0填充
    udp_addr_server.sin_family = AF_INET; // ipv4 addr
    udp_addr_server.sin_port = htons(UDP_PORT); // port
    udp_addr_server.sin_addr.s_addr = inet_addr("127.0.0.1"); // set ip addr
    
    if (bind(udp_sock_fd, (struct sockaddr *)&udp_addr_server, sizeof(udp_addr_server)) < 0){
        perror("AWS UDP bind");
        exit(1);
    }
    
    // boot up 时输出
//    cout<< "The AWS is up and running using UDP on port <" << UDP_PORT << ">. "<< endl;
    
    // creat TCP socket
    int tcp_socket_fd;
    struct sockaddr_in tcp_addr_server;
    struct sockaddr_in tcp_addr_client;
    
    tcp_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket_fd == -1){
        perror("AWS TCP socket");
        exit(1);
    }
    
    memset(&tcp_addr_server, 0, sizeof(tcp_addr_server));
    tcp_addr_server.sin_family = AF_INET;
    tcp_addr_server.sin_port = htons(TCP_PORT);
    tcp_addr_server.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if(bind(tcp_socket_fd, (struct sockaddr*)&tcp_addr_server, sizeof(tcp_addr_server)) < 0){
        perror("AWS TCP bind");
        exit(1);
    }
    
//    cout<< "The AWS is up and running using TCP on port <" << TCP_PORT << ">. "<< endl;
    
    cout << "The AWS is up and running."<<endl;
    
    listen(tcp_socket_fd, 5); // 5 -> backlog 是进入的队列（incoming queue）中所允许的连接数目
    
    
    while (1) {
//        cout<< endl;
//        cout << "AWS server wait for input : " <<endl;
        
        socklen_t tcp_len = sizeof(tcp_addr_client);
        int a = accept(tcp_socket_fd, (struct sockaddr*)&tcp_addr_client, &tcp_len);
        if ( a < 0){
            perror("AWS TCP accept fail");
            exit(1);
        }
        // receive from client
        char input_buff[1024];
        memset(input_buff, 0, sizeof(input_buff)); // clear
        int r = recv(a, input_buff, sizeof(input_buff), 0);
        if (r < 0){
            perror("AWS reading stream message error");
            exit(1);
        }else if (r == 0){
            cout << "ending connection" <<endl;
        }
        
        string str = input_buff;
        input = split(str, ",");//AWS接收client传来的message
        id = input[0];
        source = input[1];
        size = input[2];
        
        getsockname(tcp_socket_fd, (struct sockaddr*)&tcp_addr_client, &tcp_len);
        cout<< endl;
        cout << "The AWS has received map ID <"<< id.c_str();
        cout <<">, start vertex <"<< source.c_str() <<"> and file size <"<<size;
        cout <<"> from the client using TCP over port <"<< TCP_PORT <<">" <<endl;
        
        char input_to_A_buff[1024];
        string input_to_A = id + "," + source;
        strcpy(input_to_A_buff,input_to_A.c_str());
//        cout << input_to_A_buff << endl;
        send_to_server(21383, "A", input_to_A);
        receive_from_server_A();
        send_to_server(22383, "B", input_to_B);
        receive_from_server_B();
        
//        back_to_client
        // send back to client
//        char recv_to_client[1024];
//        memset(recv_to_client, 0, sizeof(recv_to_client)); // clear
//        recv_to_client = back_to_client.c_str();
        sendto(a, back_to_client.c_str(), 1024,0,(struct sockaddr*)&tcp_addr_client, sizeof(tcp_addr_client));
        getsockname(tcp_socket_fd, (struct sockaddr*)&tcp_addr_client, &tcp_len);
        cout << "The AWS has sent calculated delay to client using TCP over port <";
        cout << TCP_PORT <<">."<<endl;
        
    }
    return 0;
}
