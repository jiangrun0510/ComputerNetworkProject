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

#define awsPORT 24383

using namespace std;

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

int main(int argc, char const *argv[]){
    string id = argv[1];
    string source = argv[2];
    string size = argv[3];
//    cout << "id: "<<id << " source : "<< source << " size : "<< size << endl;
    char buff[1024];
    string str = id + "," + source + "," + size;
    strcpy(buff,str.c_str());
//    cout << "buff: " << buff<<endl;
    
    int sock_fd;
    struct sockaddr_in addr_server;
    struct sockaddr_in addr_client;
    
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd == -1){
        perror("Client TCP socket error");
        exit(1);
    }
    
    memset(&addr_server, 0, sizeof(addr_server));
    
    addr_server.sin_family = AF_INET;
    addr_server.sin_port = htons(awsPORT);
    addr_server.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    cout << "The client is up and running."<<endl;
    
    if(connect(sock_fd, (struct sockaddr*)&addr_server, sizeof(addr_server)) < 0){
        perror("Client TCP connect fail");
        exit(1);
    }
    
//    cout << "The client is connected to aws server."<<endl;
    if (send(sock_fd, buff, sizeof(buff), 0) < 0){
        perror("Client TCP send fail");
        close(sock_fd);
        exit(1);
    }
    cout << endl;
    cout << "The client has sent query to AWS using TCP over port <" << awsPORT;
    cout << ">: start vertex <"<< source.c_str();
    cout <<">; map <" << id.c_str() <<">; file size <"<< size.c_str() <<">. "<<endl;
    
    char recv_buff[1024];
    if(recv(sock_fd, recv_buff, sizeof(recv_buff), 0) < 0){
        perror("Client TCP receive fail");
        close(sock_fd);
        exit(1);
    }
    string str_recv = recv_buff;
    if(str_recv.length() > 0){
        vector<string> input_from_aws;
        input_from_aws = split(str_recv,";");
        int vertice_num = stoi(input_from_aws[0]);
        vector<int> dest_store;
        vector<float> delay_trans;
        vector<float> delay_prop;
        vector<float> delay;
        vector<int> min_length;
        vector<string> temp;
        int j = 1;
        for (int i = 0; i < vertice_num - 1; i++){
            temp = split(input_from_aws[j],",");
            dest_store.push_back(stoi(temp[0]));
            delay_trans.push_back(stof(temp[1]));
            delay_prop.push_back(stof(temp[2]));
            delay.push_back(stof(temp[3]));
            min_length.push_back(stoi(temp[4]));
            temp.clear();
            j++;
        }
        cout << endl;
        cout << "The client has received results from AWS:" <<endl;
        cout << "----------------------------------------------" <<endl;
        cout << "Destination    Min Length    Tt     Tp    Delay" <<endl;
        cout << "----------------------------------------------" <<endl;
        for (int k = 0; k < vertice_num - 1; k++){
            cout << dest_store[k];
            if (dest_store[k]>=10){
                cout << "             ";
            }else{
                cout << "              ";
            }
            
            cout << min_length[k];
            if (min_length[k] >= 10){
                cout << "            ";
            }else{
                cout << "             ";
            }
//            cout << setprecision(2) << delay_trans[k];
            printf("%.2f",delay_trans[k]);
//            cout << "  ";
            if(delay_trans[k]>=10){
                cout << " ";
            }else{
                cout << "  ";
            }
//            cout << setprecision(2) << delay_prop[k];
            printf("%.2f",delay_prop[k]);
            if(delay_prop[k]>=10){
                cout << " ";
            }else{
                cout << "  ";
            }
//            cout << "  ";
            printf("%.2f",delay[k]);
            cout <<endl;
        }
        cout << "----------------------------------------------" <<endl;
    }else{
        cout << "Not Found links" << endl;
    }
    
    close(sock_fd);
    return 0;
}
