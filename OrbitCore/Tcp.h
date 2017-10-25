//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_set>
#include <asio.hpp>

#include "Message.h"
#include "TcpEntity.h"
#include "OrbitAsio.h"

//-----------------------------------------------------------------------------
class TcpConnection : public std::enable_shared_from_this < TcpConnection >
{
public:
    typedef std::shared_ptr<TcpConnection> pointer;

    ~TcpConnection();

    static pointer create( asio::io_service& io_service )
    {
        return pointer( new TcpConnection( io_service ) );
    }

    TcpSocket& GetSocket()
    {
        return m_WrappedSocket;
    }

    void start()
    {
        ReadMessage();
    }

    void ReadMessage();
    void ReadPayload();
	void ReadFooter();
    void DecodeMessage( const Message & a_Message );

    bool IsWebsocket() { return m_WebSocketKey != ""; }
    void ReadWebsocketHandshake();
    void ReadWebsocketMessage();
    void ReadWebsocketMask();
    void ReadWebsocketPayload();
    void DecodeWebsocketPayload();
    ULONG64 GetNumBytesReceived(){ return m_NumBytesReceived; }

    void ResetStats();
    std::vector<std::string> GetStats();

private:
    TcpConnection( asio::io_service& io_service )
        : m_Socket( io_service )
        , m_WrappedSocket( &m_Socket )
    {
        m_NumBytesReceived = 0;
    }
    // handle_write() is responsible for any further actions 
    // for this client connection.
    void handle_write( const asio::error_code& /*error*/,
        size_t /*bytes_transferred*/ )
    {
    }

    void handle_request_line( asio::error_code ec, std::size_t bytes_transferred );
    void SendWebsocketResponse();

    asio::ip::tcp::socket         m_Socket;
    TcpSocket           m_WrappedSocket;
    Message             m_Message;
    std::vector<char>   m_Payload;
    asio::streambuf     m_StreamBuf;
    std::string         m_WebSocketKey;
    char                m_WebSocketBuffer[MAX_WS_HEADER_LENGTH];
    unsigned int        m_WebSocketPayloadLength;
    unsigned int        m_WebSocketMask;
    ULONG64             m_NumBytesReceived;
};

//-----------------------------------------------------------------------------
class tcp_server : public std::enable_shared_from_this < tcp_server >
{
public:
    tcp_server( asio::io_service & io_service, unsigned short port );
    ~tcp_server();

    void Disconnect();
    bool HasConnection(){ return m_Connection != nullptr; }
    TcpSocket* GetSocket(){ return m_Connection ? &m_Connection->GetSocket() : nullptr; }
    void RegisterConnection( std::shared_ptr<TcpConnection> a_Connection );
    ULONG64 GetNumBytesReceived(){ return m_Connection ? m_Connection->GetNumBytesReceived() : 0; }
    void ResetStats(){ if( m_Connection ) m_Connection->ResetStats(); }

private:
    void start_accept();
    void handle_accept( TcpConnection::pointer new_connection, const asio::error_code& error );

    asio::ip::tcp::acceptor m_Acceptor;
    std::shared_ptr<TcpConnection> m_Connection;
    std::unordered_set< std::shared_ptr<TcpConnection> > m_ConnectionsSet;
};
