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
#include <fstream>
#include <cstdio>
#include <algorithm>
#include <map>
#include <string>
#include <cstring>
#include <unordered_set>
#include <ctype.h>
#include <iomanip>
#define PORT 22383
#define DSET_IP_ADDR "127.0.0.1"

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

int main(){
    // UDP socket
    int sock_fd;
    char recv_buffer[1024];
    struct sockaddr_in addr_server;
    struct sockaddr_in addr_client;
    // creat UDP socket
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd == -1){
        perror("serverB UDP socket");
        exit(1);
    }
    memset(&addr_server, 0, sizeof(sockaddr_in));  //每个字节都用0填充
    addr_server.sin_family = AF_INET; // ipv4 addr
    addr_server.sin_port = htons(PORT); // port
    addr_server.sin_addr.s_addr = inet_addr(DSET_IP_ADDR); // set ip addr
    
    if (bind(sock_fd, (struct sockaddr *)&addr_server, sizeof(addr_server)) < 0){
        perror("serverB UDP bind");
        exit(1);
    }
    
    // boot up 时输出
    cout<< "The Server B is up and running using UDP on port <" << PORT << ">. "<< endl;
    
    while (1) {
//        cout<< endl;
//        cout << "server B wait for input : " <<endl;
        socklen_t udp_len = sizeof(addr_client);
        if (recvfrom(sock_fd, recv_buffer, 1024, 0, (struct sockaddr *)&addr_client, &udp_len) < 0){
            perror("serverB reveive fail");
            exit(1);
        }
        
//        cout << "Server B received: " << recv_buffer << endl;
        string res = recv_buffer;
        vector<string> input_from_AWS;
        input_from_AWS = split(res,";");
        
        
                              
        vector<string> speed_verticeNum;
        speed_verticeNum = split(input_from_AWS[0],",");

        // speed_verticeNum[0] -> propogation speed
        // speed_verticeNum[1] -> transmission speed
        // speed_verticeNum[2] -> vertice number
        double prop_speed = stod(speed_verticeNum[0]);
        double trans_speed = stod(speed_verticeNum[1]);
        int vertice_num = stoi(speed_verticeNum[2]);
        
        // get size
        string size;
        size = input_from_AWS[1+vertice_num];
//        cout << "size: " << size <<endl;
        long long int size_int = atoll(size.c_str());
        double delay_trans = double(size_int)/(8*trans_speed);
//        cout << "transmission delay: " << delay_trans << endl;
        // input_from_AWS[1] stores destination + min length
        vector<string> dest_min;
        dest_min = split(input_from_AWS[1],",");
        vector<int> dest_store;
        vector<int> min_length;
        
        int j = 0;
        for (int i = 0; i < vertice_num-1; i++){
            dest_store.push_back(stoi(dest_min[j]));
            min_length.push_back(stoi(dest_min[j+1]));
//            cout << "min len : " << min_length[i] << endl;
            j += 2;
        }
        
        // input_from_AWS[2..] stores paths
        map<int,vector<int> > paths_store;
        vector<string> path_temp;
        j = 0;
        for (int i = 2; i < 2 + vertice_num-1; i++){//存vertice_num-1条全路径
            if(input_from_AWS[i].length() == 1){
                paths_store[j].push_back(stoi(input_from_AWS[i]));
            }else{
                path_temp = split(input_from_AWS[i],",");
                for (int k = 0; k < path_temp.size(); k++){
                    paths_store[j].push_back(stoi(path_temp[k]));
                }
            }
            path_temp.clear();
            j++;
        }
//        for (int i = 0; i < vertice_num-1; i++){
//            cout << i<< " : "<<paths_store[i].size() <<endl;
//        }
        cout<< endl;
        cout << "The Server B has received data for calculation:"<<endl;
        cout << "* Propagation speed: <"<< std::fixed << setprecision(2)<<prop_speed << "> km/s;"<<endl;
        cout << "* Transmission speed <"<< std::fixed <<setprecision(2)<< trans_speed << "> Bytes/s;"<<endl;
        
        for (int i = 0; i <vertice_num-1; i++){
            cout << "* Path length for destination <";
            cout << dest_store[i] << "> : <";
            for(int k = 0; k < paths_store[i].size(); k++){
                if (k == paths_store[i].size()-1){
                    cout <<paths_store[i][k]<<">;"<<endl;
                }else{
                    cout <<paths_store[i][k] << " ";
                }
            }
        }
       
        // calculate delays
        vector<double> delays;
        vector<double> delay_prop;
        for (int k = 0; k < vertice_num-1; k++){
            delays.push_back(delay_trans);//前面已求出。问题：一张图只有一种delay_trans,但是有几个呢?
            delay_prop.push_back(double(min_length[k])/prop_speed);
            delays[k] += delay_prop[k];
//            delays[k] = round(delays[k]*100)/100.0;
//            cout << k << "'s delay : " << delays[k] << endl;
//            cout << k << "'s delay round : " << round(delays[k]*100)/100.0 << endl;
        }

        cout<< endl;
        cout << "The Server B has finished the calculation of the delays:" <<endl;
        cout << "------------------------" <<endl;
        cout << "Destination        Delay" <<endl;
        cout << "------------------------" <<endl;
        for (int i = 0; i < vertice_num-1; i++){
            cout << dest_store[i];
            if (dest_store[i]>=10){
                cout << "                 ";
            }else{
                cout << "                  ";
            }
            
            printf("%.2f",delays[i]);
            cout << endl;
        }
        cout << "------------------------" <<endl;
        
        
        // send message back to AWS
        string message = to_string(vertice_num) + ";";
        for(int i = 0; i < vertice_num-1; i++){
            message += to_string(dest_store[i]) + ",";
            message += to_string(delay_trans) + ",";
            message += to_string(delay_prop[i]) + ",";
            message += to_string(delays[i]) + ",";
            message += to_string(min_length[i]) + ";";
        }
        message += "end";
        if (sendto(sock_fd, message.c_str(), 1024, 0, (struct sockaddr *)&addr_client, sizeof(addr_client)) < 0){
            perror("serverB response fail");
            exit(1);
        }
        cout<< endl;
        cout << "The Server B has finished sending the output to AWS" << endl;
    }
    
    return 0;
}
