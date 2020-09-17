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
#include <set>
#include <ctype.h>

#define PORT 21383
#define DSET_IP_ADDR "127.0.0.1"

using namespace std;

const int INF = 1000000000;
map<int, char> store_letter;
map<int,set<int> > num_vertices;
map<int,int> num_edges;

int n,m,s; //点，边，源点
map<int, map<int, int> > G;
map<int,int> d;//起点到达各点的最短路径长度
map<int,bool> visited;
map<int,int> pre;//到终点x的最短路径上v的前一个点
double store_speed[2];
char message_buff[1024]; // send back to aws msg

vector<string> input;
string send_back_message;
string id, source;

void Dijkstra(int s, int select_map){ // s is source, n is number of vertices
    for(auto i : num_vertices[select_map]){
        visited[i] = false;
        // cout << i << " : "<< visited[i]<<endl;
    }
    
    for(auto i : num_vertices[select_map]){
        d[i] = INF;
        pre[i] = i;
    }
    
    // for(auto i : num_vertices[select_map]) pre[i] = i;
    d[s] = 0;
    for (auto i : num_vertices[select_map]){
        int u = -1, MIN = INF;
        for (auto j : num_vertices[select_map]){
            if (visited[j] == false && d[j] < MIN){
                u = j;
                MIN = d[j];
            }
        }
        if (u == -1) return;
        visited[u] = true;
        for (auto v : num_vertices[select_map]){

            if (visited[v] == false && G[u][v] && d[u]+G[u][v]<d[v]){
                d[v] = d[u] + G[u][v];
                pre[v] = u;
                
            }
        }
    }
}

int countMap(){
    ifstream infile("map.txt");
    int count = 0;
    int i = 0;
    if(!infile)
    {
        cout << "Can't open fie map.txt";
        return -1;
    }
    while(infile)
    {
        char letter = infile.get();
        if(isalpha(letter)){
            store_letter[i] = letter;
            count++;
            i++;
        }
    }
    //    for (int j = 0; j < i; j++){
    //        cout << j << " : " << store_letter[j] << endl;
    //    }
    return count;
}


bool cmp(const pair<int,int> &p1,const pair<int,int> &p2)//要用常数，不然编译错误
{
    return p1.second<p2.second;
}

void after_Dijkstra(int select_map){
    cout<< endl;
    cout << "The Server A has identified the following shortest paths:" << endl;
    cout << "-----------------------------" << endl;
    cout << "Destination  Min Length" << endl;
    cout << "-----------------------------" << endl;
    string message = "";
    message = to_string(store_speed[0]) + "," + to_string(store_speed[1])+ "," + to_string(n) + ";"; // n -> vertice's number
    
    
    // sort from small to large and exclude source
    // 从小到大排序输出，不输出source点

    // 存 终点->最短距离
    vector<pair<int,int> > arr;
    for (auto i : d)
    {
        arr.push_back(make_pair(i.first,i.second));
    }
    // sort(arr.begin(),arr.end(),cmp);// 按value从小到大 first=key，second=value
    for (vector<pair<int,int> >::iterator it=arr.begin();it!=arr.end();it++)
    {
        if(it->second == 0)
            continue;
        cout << it->first;
        if (it->first >= 10){
            cout <<"           ";
        }else{
            cout <<"            ";
        }
        cout<< it->second<<endl;
        message += to_string(it->first) + "," + to_string(it->second);
        if (it == arr.end()-1){
            message += ";";
        }else{
            message += ",";
        }
        
    }
    cout << "-----------------------------" << endl;
    // cout<< "message : " <<message<<endl;
    
    // arr 存 终点->最短距离，按value从小到大 first=key，second=value
    map<int, vector<int> > path_length; // 终点 -> 中间各个小路径长度
    vector<int> temp_store; // 路径上的各个节点
    vector<int>::iterator ret;
    for (auto i : arr){
        temp_store.push_back(i.first);
        int t1 = i.first; // temp[i] 号终点的路径信息
        for(int j = 0; j < sizeof(pre); j++){
            int t2 = pre[t1];
            ret = std::find(temp_store.begin(), temp_store.end(), t2);
            if(ret == temp_store.end())
                temp_store.push_back(t2);
            t1 = t2;
        }
        
        // 输出各个节点，从source到各个destination temp_store[0]
        //        for (int k = temp_store.size()-1; k >= 0; k--){
        //            cout <<temp_store[k]<< " ";
        //        }
        //        cout << endl;
        
        for (int k = temp_store.size()-1; k > 0; k--){
            path_length[i.first].push_back(G[temp_store[k]][temp_store[k-1]]);
        }
        // 输出各个小path的长度 从source到各个destination
        //        cout << "path" << endl;
        for (int k = 0; k < path_length[i.first].size(); k++){
            //            cout << path_length[temp[i]][k] << " ";
            message += to_string(path_length[i.first][k]);
            if (k == path_length[i.first].size()-1){
                message += ";";
            }else{
                message += ",";
            }
        }
        //        cout << endl;
        //        cout << "++++++++++++"<<endl;
        temp_store.clear();
    }
    // cout<<"message : "<<message<<endl;
    strcpy(message_buff, message.c_str());
    
}
// 通过map id找到对应数字下标， eg: A -> 0, B -> 1
int find_map_id_num(char map_id){
    int i = 0;
    for (i = 0; i < store_letter.size(); i++){
        if (store_letter[i] == map_id){
            break;
        }
    }
    return i;
}

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
    int sock_fd;
    char recv_buffer[1024];
    struct sockaddr_in addr_server;
    struct sockaddr_in addr_client;
    // creat UDP socket
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd == -1){
        perror("serverA UDP socket");
        exit(1);
    }
    memset(&addr_server, 0, sizeof(sockaddr_in));  //每个字节都用0填充
    addr_server.sin_family = AF_INET; // ipv4 addr
    addr_server.sin_port = htons(PORT); // port
    addr_server.sin_addr.s_addr = inet_addr(DSET_IP_ADDR); // set ip addr

    if (::bind(sock_fd, (struct sockaddr *)&addr_server, sizeof(addr_server)) < 0){
        perror("serverA UDP bind");
        exit(1);
    }
    
    // boot up 时输出
    cout<< "The Server A is up and running using UDP on port <" << PORT << ">. "<< endl;
    
    
    
    
    int numMap;
    numMap = countMap();
    map<int, map<int, map<int, int> > > original_graph;
    double speed[numMap][2];
    /*
    // 原始图初值赋为 INF
    for(int i = 0; i < numMap; i++){
        for(auto j : num_vertices[i]){
            for (auto v : num_vertices[i]){
                original_graph[i][j][v] = INF;
                //cout << "original: "<<j<<" "<< v <<" " <<original_graph[i][j][v] <<endl;
            }
            
        }
    }
    */
    // read map.txt
    ifstream infile("map.txt");
    
    string c;
    string c1, c2;//speed
    string c3, c4, c5;
    int k = 0;// 0 -> A, 1 ->B,...
    num_edges[k] = 0;
    
    infile >> c;
    
    do {
        infile >> c1 >> c2;
        //        cout << "c1 " << c1 << " c2 "<< c2 << endl;
        double t1 = stod(c1), t2 = stod(c2);
        speed[k][0] = t1; // propagation speed
        speed[k][1] = t2; // transmission speed
        //        cout << speed[k][0] << " " << speed[k][1] << endl;
        do{
            infile >> c3;
            if (isalpha (c3[0])){
                c = c3;
                k++;
                num_edges[k] = 0;
                set<int> num_vertices_each;
                num_vertices[k] = num_vertices_each;
                break;
            }
            infile >> c4;
            infile >> c5;
            int t3 = stoi(c3), t4 = stoi(c4), t5 = stoi(c5);
            original_graph[k][t3][t4] = t5;
            original_graph[k][t4][t3] = t5; //无方向的边
            //            cout << t3 << " "<< t4 << " "<< t5 << endl;
            num_vertices[k].insert(t3);
            num_vertices[k].insert(t4);
            num_edges[k]++;
        } while(infile);
        
    } while(infile);
    
    num_edges[k]--; // 里层循环多存了最后一条边 （ 怎么解决？）
    
    cout<< endl;
    cout << "The Server A has constructed a list of <number> maps: "<< endl;
    cout << "-------------------------------------------" << endl;
    cout << "Map ID" << "  " <<  "Num Vertices"<< "  " << "Num Edges" << endl;
    cout << "-------------------------------------------" << endl;
    for (int i = 0; i <= k; i++){
        cout <<store_letter[i];
        cout <<"       ";
        cout << num_vertices[i].size();
        if (num_vertices[i].size() >= 10){
            cout << "            ";
        }else{
            cout << "             ";
        }
        
        cout <<num_edges[i]<< endl;
    }
    cout << "-------------------------------------------" << endl;
    // end of read map
    
    while (1) {
//        cout<< endl;
//        cout << "server A wait for input : " <<endl;
        socklen_t udp_len = sizeof(addr_client);
        if (recvfrom(sock_fd, recv_buffer, 1024, 0, (struct sockaddr *)&addr_client, &udp_len) < 0){
            perror("serverA reveive fail");
            exit(1);
        }
        
        //        cout << "Server A received: " << recv_buffer << endl;
        
        // 得到 map id 和 source
        string str = recv_buffer;
        input = split(str,",");
        id = input[0];
        source = input[1];
        //        cout <<"id : "<< id << endl;
        //        cout << "source: " << source<<endl;
        
       
        int select_map =  find_map_id_num(id[0]);// 0 -> A
        s = stoi(source); // source
        
        cout <<endl;
        cout << "The Server A has received input for finding shortest paths: starting vertex <" << s << "> of map <" << id[0] << ">." <<endl;
        n = num_vertices[select_map].size(); // vertices' number
        m = num_edges[0]; // edges' number
        
        for(auto i : num_vertices[select_map]){
            for(auto j:num_vertices[select_map]){
                G[i][j] = original_graph[select_map][i][j];
                //cout << i << " "<< j << " : "<< G[i][j]<<endl;
            }
        }
        store_speed[0] = speed[select_map][0];
        store_speed[1] = speed[select_map][1];
        
        Dijkstra(s,select_map);
        after_Dijkstra(select_map);
        
        if (sendto(sock_fd, message_buff, 1024, 0, (struct sockaddr *)&addr_client,sizeof(addr_client)) < 0){
            perror("serverA response fail");
            exit(1);
        }
        cout << "The Server A has sent shortest paths to AWS." << endl;
        
        // clear
        input.clear();
        id.clear();
        source.clear();
        
        
    }

    

    
    
    return 0;
}
