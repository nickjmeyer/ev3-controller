#ifndef EV3_CLIENT_HPP
#define EV3_CLIENT_HPP

#include "networkWrapper.hpp"
#include "command.pb.h"
#include "ev3Robot.hpp"
#include <queue>

namespace Ev3Controller {

class Ev3Client;
class Ev3ClientConnection;

class Ev3Client {
public:

    Ev3Client();

    void processCommand(const std::vector<uint8_t> & buffer,
            const std::shared_ptr<Ev3ClientConnection> & connection);

    void init(const std::string id,
            const std::shared_ptr<Ev3ClientConnection> connection);

    bool isInit() const;

    bool alive() const;

    void kill();

private:
    std::shared_ptr<Ev3ClientConnection> connection;
    bool isInit_;
    std::string id;
    bool alive_;

    Ev3Robot robot;
};


class Ev3ClientConnection : public Connection
{
private:
    std::shared_ptr<Ev3Client> ev3Client;

    void OnAccept( const std::string & host, uint16_t port );

    void OnConnect( const std::string & host, uint16_t port );

    void OnSend( const std::vector< uint8_t > & buffer );

    void OnRecv( std::vector< uint8_t > & buffer );

    void OnTimer( const std::chrono::milliseconds & delta );

    void OnError( const asio::error_code & error );

public:
    Ev3ClientConnection( std::shared_ptr<Ev3Client> ev3Client,
            std::shared_ptr< Hive > hive );

    ~Ev3ClientConnection();

    std::shared_ptr<Connection> NewConnection();
};

} // Ev3Controller namespace

#endif
