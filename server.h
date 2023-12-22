#ifndef SERVER_H
#define SERVER_H
#include <iostream>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <vector>

#include "servicethreads.h"
#include "states.h"

using namespace boost::asio;


class Session: public std::enable_shared_from_this<Session>
{
    ip::tcp::socket socket_m;
    const static size_t max_len=1025;
    char data_m[max_len];
    std::shared_ptr<Executor> executor_m;


public:
    Session(ip::tcp::socket socket, size_t bulk) :socket_m(std::move(socket))
    {
        //очередь сообщений на логирование
        MyQueue<std::string>& logger_queue=MyQueue<std::string>::get_instance();

        //очередь сообщений для файловых потоков
        MyQueue<std::pair<std::string,std::string>>& file_queue=MyQueue<std::pair<std::string,std::string>>::get_instance();

        SharedExecutorData data;
        data.size=bulk;
        data.logQueue_ptr=&logger_queue;
        data.fileQueue_ptr=&file_queue;



        executor_m=std::shared_ptr<Executor>(new Executor(data));
        std::shared_ptr<StateFactory> factory(new StateFactory);

        factory->registrate<State_Creator<Simple_Commamd_Queue_State,Executor_ptr>, Executor_ptr> (Simple_Commamd_Queue_State::id, Executor_ptr(executor_m));
        factory->registrate<State_Creator<Dynamic_Commamd_Queue_State,Executor_ptr>, Executor_ptr> (Dynamic_Commamd_Queue_State::id, Executor_ptr(executor_m));
        factory->registrate<State_Creator<Dynamic_Commamd_Queue_Nested_Block_State,Executor_ptr>, Executor_ptr> (Dynamic_Commamd_Queue_Nested_Block_State::id, Executor_ptr(executor_m));

        executor_m->setFactory(factory);
    }

    void start() { doRead();}

  private:
    void doRead()
    {
        auto self(shared_from_this());
        socket_m.async_read_some(buffer(data_m, max_len),
                   [this, self](boost::system::error_code error, std::size_t lenght) {

                        if (!error)
                        {
                            data_m[lenght]='\0';
                            std::string com(data_m);
                            std::vector<std::string> vector;
                            boost::algorithm::split(vector, com, [](char ch){return (ch=='\n');});
                            for (auto x: vector)
                            {
                                if (x=="") break;
                                //std::string s="@"+x+'\n';
                                //std::cout<<s;
                                executor_m->execute(x);
                            }

                        }
                        //else std::cout<<"Error:  "<<error.message()<<std::endl;
                        doRead();
        });

    }


};

class Server
{
    ip::tcp::acceptor acceptor_m;
    size_t bulk_m;
    std::vector<std::thread> m_threads;
    inline const auto static nThreads = std::thread::hardware_concurrency();

public:
    Server(io_context& io_context, short port, size_t bulk) : acceptor_m(io_context, ip::tcp::endpoint(ip::tcp::v4(), port)), bulk_m(bulk)
    {
        doAccept();

        //Use signal_set for terminate context
        signal_set signals{io_context, SIGINT, SIGTERM};
        //signal SIGINT will be send when user press Ctrl+C (terminate)
        //signal SIGTERM will be send when system terminate program
        signals.async_wait([&](auto, auto) { io_context.stop(); });

          m_threads.reserve(nThreads);
          for (unsigned int i = 0; i < nThreads; ++i) {
            m_threads.emplace_back([&io_context]() { io_context.run(); });
          }
          for (auto &th : m_threads) {
            th.join(); //this is io_context run procedure into every thread from threads
            //that will allow us to use multiple threads to work on client's messages
          }
    }
private:
    void doAccept()
    {
        acceptor_m.async_accept([this](boost::system::error_code error, ip::tcp::socket socket) {
           if (!error)
           {
               //std::cout<<"create session on:"<<socket.remote_endpoint().address().to_string()<<":"<<socket.remote_endpoint().port()<<"\n";
               std::make_shared<Session>(std::move(socket), bulk_m)->start();
           }
           else std::cout<<"Unfortunately an error is occured: "<<error.message()<<std::endl;
           doAccept();
        });
    }
};


#endif // SERVER_H
