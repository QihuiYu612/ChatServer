#include "json.hpp"
using json = nlohmann::json;

#include <iostream>
#include <vector>
#include <map>
#include <string>
using namespace std;

string func1(){
    json js;
    js["msg_type"] = 2;
    js["from"] = "zhang san";
    js["to"] = "li si";
    js["msg"] = "hello";

    // 使用链式Hash表，无序存储
    // {"from":"zhang san","msg":"hello","msg_type":2,"to":"li si"}
    // cout<<js<<endl;
    string sendBuf = js.dump();
    // cout<<sendBuf.c_str()<<endl;
    return sendBuf;
}

string func2(){
    json js;
    // 添加数组
    js["id"] = {1,2,3,4,5}; 
    // 添加key-value
    js["name"] = "zhang san"; 
    // 添加对象
    js["msg"]["zhang san"] = "hello world";
    js["msg"]["liu shuo"] = "hello china"; 
    // 上面等同于下面这句一次性添加数组对象
    js["msg"] = {{"zhang san", "hello world"}, {"liu shuo", "hello china"}};
    // cout << js << endl;
    return js.dump();
}

string func3(){
    json js;
    // 直接序列化一个vector容器
    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(5);
    js["list"] = vec;
    // 直接序列化一个map容器
    map<int, string> m;
    m.insert({1, "黄山"});
    m.insert({2, "华山"});
    m.insert({3, "泰山"});
    js["path"] = m;

    // Json数据对象 -> 序列化 Json字符串
    string sendBuf = js.dump();
    // cout<<sendBuf.c_str()<<endl;
    return sendBuf;
}

int main(){
    string recvBuf = func3();
    // 数据的反序列化 json字符串 -> 反序列化 数据对象
    // 看作容器，方便访问
    // json jsBuf = json::parse(recvBuf);
    // cout<<jsBuf["msg_type"]<<endl;
    // cout<<jsBuf["from"]<<endl;
    // cout<<jsBuf["to"]<<endl;
    // cout<<jsBuf["msg"]<<endl;

    // json jsBuf = json::parse(recvBuf);
    // cout << jsBuf["id"] << endl;
    // auto arr = jsBuf["id"];
    // cout << arr[2] << endl;

    // auto msgjs = jsBuf["msg"];
    // cout << msgjs["zhang san"] << endl;
    // cout << msgjs["liu shuo"] << endl;

    json jsBuf = json::parse(recvBuf);
    vector<int> vec = jsBuf["list"];
    // auto vec = jsBuf["list"];
    // cout << vec << endl;
    for(int &v:vec){
        cout<<v<<" ";
    }
    cout<<""<<endl;

    // auto mymap = jsBuf["path"];
    // cout << mymap << endl;
    map<int,string> mymap = jsBuf["path"];
    for(auto &p:mymap){
        cout<<p.first<<" "<<p.second<<endl;
    }
    return 0;
}