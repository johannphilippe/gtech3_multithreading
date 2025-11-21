#ifndef QUEUE_H
#define QUEUE_H
#include "Thread.h"
#include<optional>

template <typename T>
class SafeQueue
{

    struct QueueNode
    {
        QueueNode* pNextNode = nullptr;
        QueueNode* pPreviousNode = nullptr;
        T Data;
    };

public:
    SafeQueue();
    ~SafeQueue();

    void Push(T data);
    std::optional<T> Pop();
    std::optional<T> Back();

    bool IsEmpty();

private:
    QueueNode* m_pLastNode = nullptr;
    CRITICAL_SECTION* m_criticalSection;
    //LPCRITICAL_SECTION m_criticalSection;
};

template <typename T>
SafeQueue<T>::SafeQueue()
{
    InitializeCriticalSection(&m_criticalSection);
}

template <typename T>
SafeQueue<T>::~SafeQueue()
{
    QueueNode* currentNode = m_pLastNode;
    while (currentNode != nullptr)
    {
        QueueNode* nextNode = currentNode->pNextNode;
        delete currentNode;
        currentNode = nextNode;
    }
    DeleteCriticalSection(&m_criticalSection);
}

template <typename T>
void SafeQueue<T>::Push(T data)
{
    EnterCriticalSection(&m_criticalSection);

    QueueNode* newNode = new QueueNode();
    newNode->Data = data;
    newNode->pPreviousNode = m_pLastNode;

    if (m_pLastNode == nullptr)
    {
        m_pLastNode = newNode;
        return;
    }

    m_pLastNode->pNextNode = newNode;
    m_pLastNode = newNode;

    LeaveCriticalSection(&m_criticalSection);
}

template <typename T>
std::optional<T> SafeQueue<T>::Pop()
{
    EnterCriticalSection(&m_criticalSection);

    QueueNode* temp = m_pLastNode;
    T tempData = temp->Data;
    m_pLastNode = m_pLastNode->pPreviousNode;
    delete temp;

    LeaveCriticalSection(&m_criticalSection);
    return tempData;
}

template <typename T>
std::optional<T> SafeQueue<T>::Back()
{
    EnterCriticalSection(&m_criticalSection);
    if (m_pLastNode == nullptr)
    {
        LeaveCriticalSection(&m_criticalSection);
        return {};
    }

    LeaveCriticalSection(&m_criticalSection);
    return m_pLastNode->Data;
}

template <typename T>
bool SafeQueue<T>::IsEmpty()
{
    EnterCriticalSection(&m_criticalSection);
    bool isEmpty = m_pLastNode == nullptr;
    LeaveCriticalSection(&m_criticalSection);
    return isEmpty;
}


#endif
