#pragma once

template<typename T>
class Singleton
{
public:
	static T& instance();

	Singleton(const Singleton&) = delete;

	Singleton& operator=(const Singleton) = delete;

protected:
	struct token
	{
	};

	Singleton()
	{}
};

#include <memory>

template<typename T>
T& Singleton<T>::instance()
{
	static T instance{token{}};
	return instance;
}

