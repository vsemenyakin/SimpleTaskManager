#pragma once

#include <functional>
#include <memory>

template<typename ValueType>
struct PromiseState;

//-----

template<typename ValueType>
class Future
{
public:    
    void Next(const std::function<void(const ValueType&)>& Continuation);
    
private:
   using StateType = PromiseState<ValueType>;

   template<typename _ValueType>
   friend class Promise;

   Future(std::shared_ptr<StateType> InState);

   std::shared_ptr<StateType> State;
};

//-----

template<typename ValueType>
class Promise
{   
public:

   Promise();

   void Set(ValueType&& Value);

   Future<ValueType> GetFuture();

private:
   using StateType = PromiseState<ValueType>;

   std::shared_ptr<StateType> State;
};

//-----

#include "Future.inl"
