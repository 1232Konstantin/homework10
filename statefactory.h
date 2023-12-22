#ifndef STATEFACTORY_H
#define STATEFACTORY_H
#include <memory>
#include <string>
#include <map>
#include <functional>

//Библиотека фабрики состояний.
//Не требует кастомизации при условии, что объект состояния создается из одного POD аргумента

//Интерфейс для хранения объектов в нутри фабрики состояний
class BaseState
{
public:
    virtual void execute(std::string, std::string timepoint)=0;
    virtual ~BaseState()=default;
};

//Абстрактный класс создателя состояний
class Abstract_BaseState_Creator
{
public:
    virtual ~Abstract_BaseState_Creator() {}
    virtual std::unique_ptr<BaseState> create() const = 0;
};

//Шаблонный креатор для создания объектов состояний
template <typename  State, typename U>       //Класс ChildObject наследник класса BaseState
class State_Creator: public Abstract_BaseState_Creator
{
    U m_arg;
public:
    State_Creator(const U arg) : m_arg(arg){}
    virtual std::unique_ptr<BaseState> create() const override {return std::unique_ptr<BaseState>(new State(m_arg));}
};

//Фабрика состояний
class StateFactory
{
public:
 StateFactory () = default;

 using map_type= std::map<std::string, std::shared_ptr<Abstract_BaseState_Creator>> ;

 template<typename StateCreator,typename Arg>
    void registrate (const std::string& type, Arg arg) //добавление креатора объектов в map
    {
            map_type::iterator it = m_map.find(type);
            if (it == m_map.end())
                m_map[type] = std::shared_ptr<Abstract_BaseState_Creator>(new StateCreator(arg));
    }

    std::unique_ptr<BaseState> create(const std::string& type) //создание объекта
    {
            map_type::iterator it = m_map.find(type);
            if (it != m_map.end())   return it->second->create();
            return 0;
    }

private:
    //inline static map_type m_map;
    map_type m_map;

};


#endif // STATEFACTORY_H
