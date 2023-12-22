#include <iostream>
#include "server.h"
#include "client.h"
#include <thread>
#include <string>

using namespace std;

#define TEST 1


//Тесты
void test(short port)
{
    try {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        //std::list<std::string> list {"cmd1\n", "cmd2\n", "cmd3\n", "cmd4\n", "cmd5\n"};
        std::list<std::string> list2 {"second1\n", "second2\n", "second3\n", "second4\n", "second5\n", "second6\n", "second7\n", "second8\n", "second9\n", "second10\n","second11\n", "second12\n"};
        Client cl1(std::to_string(port));
        Client cl2(std::to_string(port));
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::list<std::string> list3 {"cmd1\n", "cmd2\n", "cmd3\n", "cmd4\n","{\n","nested1\n", "nested2\n", "nested3\n","}\n", "cmd6\n", "cmd7\n","cmd8\n"};
        std::list<std::string> list4 {"ccc1\n",  "ccc4\n","{\n","nested4\n", "nested5\n","}\n", "ccc6\n", "ccc7\n","ccc8\n","ccc2\n", "ccc3\n",};
        auto it3=list2.begin();
        for(auto it=list3.begin(); it!=list3.end(); it++)
        {
            cl1.send(*it);
            cl2.send(*it3);
            it3++;
        }
        auto it4=list2.begin();
        for(auto it=list4.begin(); it!=list4.end(); it++)
        {
            cl1.send(*it);
            cl2.send(*it4);
            it4++;
        }

        cl1.disconnect();
        cl2.disconnect();
    }  catch (std::exception e) { cout<<"Exception into test:  "<<e.what()<<endl;

    }


}



int main(int argc, char *argv[])
{

    short port=(argc==3)? std::stoi(std::string(argv[1])) : 9000;
    int n=(argc==3)? std::stoi(std::string(argv[2])) : 3;


    if (TEST)
    {
        //Запускаем двух клиентов с тестовыми данными
        auto res=std::thread(test, port);
        res.detach();
    }


    try {
        //Создаем очередь сообщений на логирование
        MyQueue<std::string>& logger_queue=MyQueue<std::string>::get_instance();

        //Создаем очередь сообщений для файловых потоков
        MyQueue<std::pair<std::string,std::string>>& file_queue=MyQueue<std::pair<std::string,std::string>>::get_instance();

        //Создаем потоки loger, file1, file2
        ServiceTreads::startServiceThreads( logger_queue, file_queue);

        //Создаем и запускаем сервер
        io_context context;
        Server s(context, port, n);

        //context.run();
    }  catch (std::exception e) { cout<<"Wery bad Exception from Server:  "<<e.what()<<endl;

                                }


    int i;
    cin>>i;



    return 0;
}
