#ifndef EV3_SERVER_HPP
#define EV3_SERVER_HPP

#include "networkWrapper.hpp"
#include "command.pb.h"
#include <queue>
#include <set>
#include <map>
#include <asio/error.hpp>
#include <ncurses.h>

namespace Ev3Controller {

// random identifier
std::string genIdentifier();

class Ev3Server;
class Ev3ServerConnection;
class Ev3Acceptor;

class Ev3Server {
public:
    void processCommand(const std::vector<uint8_t> & buffer,
            const std::shared_ptr<Ev3ServerConnection> & connection);

    const std::set<std::string> & getId() const;

    void drop(const std::shared_ptr<Ev3ServerConnection> & connection);

    void sendCommand(const std::string & recipient, const Ev3Command & command);

    void quit();

private:
    // connection containers
    std::set<std::string> id;
    std::map<std::shared_ptr<Ev3ServerConnection>,
             std::string> connToId;
    std::map<std::string,
             std::shared_ptr<Ev3ServerConnection> > idToConn;
};


class Ev3ServerConnection : public Connection
{
private:
    std::shared_ptr<Ev3Server> ev3Server;

    void OnAccept( const std::string & host, uint16_t port );

    void OnConnect( const std::string & host, uint16_t port );

    void OnSend( const std::vector< uint8_t > & buffer );

    void OnRecv( std::vector< uint8_t > & buffer );

    void OnTimer( const std::chrono::milliseconds & delta );

    void OnError( const asio::error_code & error );

public:
    Ev3ServerConnection( std::shared_ptr<Ev3Server> ev3Server,
            std::shared_ptr< Hive > hive );


    ~Ev3ServerConnection();

    std::shared_ptr<Connection> NewConnection();
};

class Ev3Acceptor : public Acceptor
{
private:
    std::shared_ptr<Ev3Server> ev3Server;

    bool OnAccept( std::shared_ptr< Connection > connection,
            const std::string & host, uint16_t port );

    void OnTimer( const std::chrono::milliseconds & delta );

    void OnError( const asio::error_code & error );

public:
    Ev3Acceptor( std::shared_ptr<Ev3Server> ev3Server,
            std::shared_ptr< Hive > hive );

    ~Ev3Acceptor();
};

enum ControllerMode {ROBOT_SELECT, ROBOT_DRIVE, ROBOT_QUIT};

class InputPoller {
private:
    std::shared_ptr< Ev3Server> server;

    ControllerMode mode;

    const int width;
    const int height;

    const int startx;
    const int starty;

    double xVel;
    double zRot;

    WINDOW * menu_win;

    std::vector<std::string> id;
    std::string idChoice;
    int choice;

    void refresh_id();

    void poll_select();

    void poll_drive();

    void print_menu();

    void print_menu_select();

    void print_menu_drive();

public:
    InputPoller(std::shared_ptr< Ev3Server > & server );

    void poll();
};

} // Ev3Controller namespace

#endif
