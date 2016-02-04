/* Copyright(C)
* For free
* All right reserved
* 
*/
/**
* @file TcpServerImpl.cpp
* @brief tcp服务器实现
* @author highway-9, 787280310@qq.com
* @version 1.1.0
* @date 2016-01-31
*/

#include "TcpServerImpl.h"
#include "Message.h"
#include "PeopleInfoMessage.h"
#include <iostream>

TcpServerImpl::TcpServerImpl(unsigned short port)
    : m_acceptor(m_ioService,
        boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
    m_ioServiceThread.reset();
    accept();
}

TcpServerImpl::~TcpServerImpl()
{
    stop();
}

bool TcpServerImpl::start()
{
    if (m_ioServiceThread == NULL)
    {
        try
        {
            m_ioServiceThread = boost::make_shared<boost::thread>
                    (boost::bind(&boost::asio::io_service::run, &m_ioService));
        }
        catch (std::exception& e)
        {
            std::cout << "Error: " << e.what() << std::endl;
            return false;
        }
    }

    return true;
}

bool TcpServerImpl::stop()
{
    closeAllTcpSession();
    m_ioService.stop();
    joinIOServiceThread();

    return true;
}

std::vector<std::string> TcpServerImpl::allRemoteAddress()
{
    std::vector<std::string> vecAllRemoteAddress;
    for (auto& iter : m_tcpSessionMap)
    {
        vecAllRemoteAddress.push_back(iter.first);
    }

    return vecAllRemoteAddress;
}

void TcpServerImpl::accept()
{
    std::cout << __FUNCTION__ << " " << __LINE__ << std::endl;
    TcpSessionPtr tcpSession(new TcpSession(m_ioService));
    m_acceptor.async_accept(tcpSession->socket(),
        boost::bind(&TcpServerImpl::handleAccept, this, tcpSession,
                    boost::asio::placeholders::error));
}

void TcpServerImpl::handleAccept(TcpSessionPtr tcpSession,
                                 const boost::system::error_code &error)
{
    std::cout << __FUNCTION__ << " " << __LINE__ << std::endl;
    if (!error)
    {
        //Message message;
        //tcpSession->asyncRead(message);
//        PeopleInfoMessage *peopleInfoMessage = new PeopleInfoMessage;
//        peopleInfoMessage->m_messageType = 1000;
//        peopleInfoMessage->m_name = "Jack";
//        peopleInfoMessage->m_age = 20;
//        tcpSession->asyncWrite(peopleInfoMessage);
        std::cout << "remote address: " << tcpSession->remoteAddress() << std::endl;
        m_tcpSessionMap.insert(std::make_pair(tcpSession->remoteAddress(), tcpSession));
    }
    else
    {
        std::cout << "Tcp server accept failed: " << error.message() << std::endl;
    }

    accept();
}

void TcpServerImpl::closeAllTcpSession()
{
    auto begin = m_tcpSessionMap.begin();
    auto end = m_tcpSessionMap.end();
    while (begin != end)
    {
        TcpSessionPtr session = begin->second;
        try
        {
            session->socket().close();
        }
        catch (std::exception& e)
        {
            std::cout << "Error: " << e.what() << std::endl;
        }

        ++begin;
    }

    m_tcpSessionMap.clear();
}

void TcpServerImpl::joinIOServiceThread()
{
    if (m_ioServiceThread != NULL)
    {
        if (m_ioServiceThread->joinable())
        {
            m_ioServiceThread->join();
        }
    }
}

bool TcpServerImpl::isTcpSessionExists(const std::string &remoteAddress)
{
    auto iter = m_tcpSessionMap.find(remoteAddress);
    if (iter != m_tcpSessionMap.end())
    {
        return true;
    }

    return false;
}

TcpSessionPtr TcpServerImpl::tcpSession(const std::string& remoteAddress)
{
    auto iter = m_tcpSessionMap.find(remoteAddress);
    if (iter != m_tcpSessionMap.end())
    {
        return iter->second;
    }

    return TcpSessionPtr();
}