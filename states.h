#ifndef STATES_H
#define STATES_H

#include "executer.h"
#include <ostream>
#include <sstream>
#include <memory>





//Интерфейс состояний
class IState: public BaseState
{

public:
    IState(const Executor_ptr exec) : m_executor(exec) {}
    virtual void execute(std::string, std::string) override {};
    Executor_ptr get_executor() {return m_executor;}
    size_t getSize() {return m_executor.lock()->getExecutorData().size;}
    MyQueue<std::string>* getLogQueue() //очередь на логгирование
    {
        return reinterpret_cast<MyQueue<std::string>*>(m_executor.lock()->getExecutorData().logQueue_ptr);
    }
    MyQueue<std::pair<std::string, std::string>>* getFileQueue() //очередь записи в файл
    {
        return reinterpret_cast<MyQueue<std::pair<std::string, std::string>>*>(m_executor.lock()->getExecutorData().fileQueue_ptr);
    }


    virtual ~IState()=default;
protected:

    //многопоточная версия print_block(). Все остальное из предыдущей домашки без изменений
    void print_block(bool common=false)
    {
        //std::cout<<"print_block\n";
        std::string commandBlock;
        std::string commandBlockTime;
        auto command_list=(common)?  m_executor.lock()->getCommonCommandList() : m_executor.lock()->getCommandList();
        if (!command_list->empty())
        {
            commandBlock+="bulk: " + command_list->front().first;
            commandBlockTime=command_list->front().second;
            command_list->pop_front();

            for (auto x : *command_list) commandBlock+=", " + x.first;
            commandBlock+="\n";
            command_list->clear();

            //помещаем в очередь логов
            std::shared_ptr<std::string> str1(new std::string);
            *str1=commandBlock;
            getLogQueue()->add(str1);
            getLogQueue()->cv.notify_one(); //уведомляем логер

            //помещаем в очередь записи в файл
            std::shared_ptr<std::pair<std::string,std::string>> pair_ptr(new std::pair<std::string,std::string>);
            *pair_ptr=std::make_pair(commandBlock, commandBlockTime);
            getFileQueue()->add(pair_ptr);
            getFileQueue()->cv.notify_one(); //уведомляем потоки file1 и file2
        }
        //else std::cout<<"EMPTY\n";
    }


private:
    Executor_ptr m_executor;


};





//Состояние 1
//Для сохранения неизменности executor исходное состояние должно называться "FIRST_STATE"
class Simple_Commamd_Queue_State: public IState
{

public:
    Simple_Commamd_Queue_State(const Executor_ptr& exec ) : IState(exec)  {}
    inline const static std::string id="FIRST_STATE";
    virtual void execute(std::string command, std::string time) final override
    {
        if(command=="EOF")
        {
            print_block(true) ;
            return;
        }
        if (command=="{")
        {
            //print_block(true);
            get_executor().lock()->setState("DYNAMIC_COMMAND_QUEUE");
            return;
        }
        auto list=get_executor().lock()->getCommonCommandList();
        list->push_back(std::make_pair(command,time));
        if (list->size()==getSize()) print_block(true);
        //else std::cout<<"#"<<list->size();
    };
    ~Simple_Commamd_Queue_State()=default;
};

//Состояние 2
class Dynamic_Commamd_Queue_State: public IState
{

public:
    Dynamic_Commamd_Queue_State(const Executor_ptr& exec ) : IState(exec)  {}
    inline const static std::string id="DYNAMIC_COMMAND_QUEUE";
    virtual void execute(std::string command, std::string time) final override
    {
        if(command=="EOF")
        {
            get_executor().lock()->getCommandList()->clear();
            get_executor().lock()->setState("FIRST_STATE");
            return;
        }

        if(command=="}")
        {
            print_block();
            get_executor().lock()->setState("FIRST_STATE");
            return;
        }
        if(command=="{")
        {
            get_executor().lock()->setState("DYNAMIC_COMMAND_QUEUE_NESTED_BLOCK");
            return;
        }
        auto list=get_executor().lock()->getCommandList();
        list->push_back(std::make_pair(command,time));
    };
    ~Dynamic_Commamd_Queue_State()=default;
};

//Состояние 3
class Dynamic_Commamd_Queue_Nested_Block_State: public IState
{
    size_t m_count=1;
public:
    Dynamic_Commamd_Queue_Nested_Block_State(const Executor_ptr& exec ) : IState(exec)  { }
    inline const static std::string id="DYNAMIC_COMMAND_QUEUE_NESTED_BLOCK";
    virtual void execute(std::string command, std::string time) final override
    {
        if(command=="EOF")
        {
            get_executor().lock()->getCommandList()->clear();
            return;
        }

        if(command=="}")
        {
            m_count--;
            if (m_count==0) get_executor().lock()->setState("DYNAMIC_COMMAND_QUEUE");
            return;
        }
        if(command=="{")
        {
            m_count++;
            return;
        }
        auto list=get_executor().lock()->getCommandList();
        list->push_back(std::make_pair(command,time));
    };
    ~Dynamic_Commamd_Queue_Nested_Block_State()=default;
};

#endif // STATES_H
