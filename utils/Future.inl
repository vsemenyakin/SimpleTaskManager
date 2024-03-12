#pragma once

template<typename ValueType>
struct PromiseState
{
    std::function<void(const ValueType&)> Continuation;
    ValueType Value;
};

//----

template<typename ValueType>
void Future<ValueType>::Next(
	const std::function<void(const ValueType&)>& Continuation)
{
	State->Continuation = Continuation;
}

template<typename ValueType>
Future<ValueType>::Future(std::shared_ptr<StateType> InState)
  : State(InState) { }

//----

template<typename ValueType>
Promise<ValueType>::Promise()
  : State(std::make_shared<StateType>()) { }

template<typename ValueType>
void Promise<ValueType>::Set(ValueType&& Value)
{
  State->Value = std::move(Value);
  
  if (State->Continuation)
  {
	 State->Continuation(State->Value);   
  }
}

template<typename ValueType>
Future<ValueType> Promise<ValueType>::GetFuture()
{
   return { State };
}
