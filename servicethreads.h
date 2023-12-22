#ifndef SERVICETHREADS_H
#define SERVICETHREADS_H
#include "myqueue.h"
#include <fstream>
#include <sstream>
#include <thread>
#include  <functional>
#include <iostream>

namespace ServiceTreads {     //тут описан запуск потоков loger, file1 и file2


    inline void file_start(MyQueue<std::pair<std::string,std::string>>& file_queue)
    {
        //поток будет ждать работы на condition_variable из очереди file_queue
        try {
          //  std::cout<<"\nstart file thread"<<std::this_thread::get_id()<<std::endl;
            auto func=[&](std::unique_lock<std::mutex>& lock) //функция которая будет вызываться при наличии данных в очереди
            {
                auto pair=file_queue.get();
                bool need_notify=(file_queue.size()>0);
                lock.unlock(); //отпускаем мьютекс, так как запись в файл длительный процесс
                if (need_notify) file_queue.cv.notify_one(); //если очередь оказывается не пуста пробуем уведомить второй поток, что для него тоже есть работа
                //переходим к записи данных в файл. Не забываем добавить уникальный идентификатор потока к имени файла
                std::fstream stream;
                std::thread::id id=std::this_thread::get_id();
                std::stringstream namestream;
                namestream<<"D:/garbage/"<<pair->second<<"_thread"<<id<<".log";
                std::string fileName=namestream.str();
                //std::cout<<"file: "<<fileName<<std::endl;
                stream.open(fileName,std::ios_base::out);
                stream<<pair->first;
                stream.close();
            };

        //подключаемся к очереди
        for(;;)
        {
            std::unique_lock<std::mutex> lock1(file_queue.mutex);
            if (file_queue.size()>0) func(std::ref(lock1)); //если очередь не пуста, рано вставать на ожидание. работаем!
            else //очередь пуста, встаем на ожидание
            {
                file_queue.cv.wait(lock1, [&file_queue]{return (file_queue.size()>0);});
                func(std::ref(lock1));
            }
        }

        }  catch (...) {
            std::cout<<"There is a big trouble: Unexpected exception in file tread..."<<std::endl;
        }


    }


    inline void logger_start(MyQueue<std::string>& logger_queue)
    {
        //поток будет ждать работы на condition_variable из очереди logger_queue
        try {
           // std::cout<<"\nstart logger thread"<<std::this_thread::get_id()<<std::endl;
            auto func=[&](std::unique_lock<std::mutex>& lock) //функция которая будет вызываться при наличии данных в очереди
            {
                auto str=logger_queue.get();
                lock.unlock();
                std::string temp=(*str);
                std::cout<<temp;
            };

        //подключаемся к очереди
        for(;;)
        {
            std::unique_lock<std::mutex> lock1(logger_queue.mutex);
            if (logger_queue.size()>0) func(std::ref(lock1)); //если очередь не пуста, рано вставать на ожидание. работаем!
            else //очередь пуста, встаем на ожидание
            {
                logger_queue.cv.wait(lock1, [&logger_queue]{return (logger_queue.size()>0);});
                func(std::ref(lock1));
            }
        }

        }  catch (...) {
            std::cout<<"There is a big trouble: Unexpected exception in logger tread..."<<std::endl;
        }
    }



    inline void startServiceThreads(MyQueue<std::string>& logger_queue, MyQueue<std::pair<std::string,std::string>>& file_queue)
    {
        try {
            //Запускаем в работу потоки loger, file1 и file2
            std::thread logger;
            logger= std::thread(logger_start, std::ref(logger_queue));
            std::thread file1tread;
            file1tread= std::thread(file_start, std::ref(file_queue));
            std::thread file2tread;
            file2tread= std::thread(file_start, std::ref(file_queue));
            logger.detach();
            file1tread.detach();
            file2tread.detach();

        }  catch (...) {
            std::cout<<"There is a big trouble: Unexpected exception in startServiceThreads function..."<<std::endl;
        }

    }


}


#endif // SERVICETHREADS_H
