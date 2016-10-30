#include "ev3Server.hpp"
#include <unistd.h>
#include <iostream>
#include <random>
#include <mutex>
#include <queue>
#include <thread>
#include <ncurses.h>

namespace Ev3Controller {

std::mutex global_stream_lock;
std::mutex global_rng_lock;

void ServerRunThread( std::shared_ptr< Hive > hive)
{
    global_stream_lock.lock();
    // std::cout << "thread started" << std::endl;
    global_stream_lock.unlock();

    hive->Run();

    global_stream_lock.lock();
    // std::cout << "thread ended" << std::endl;
    global_stream_lock.unlock();
}



std::string genIdentifier() {
    static const std::string alphanums =
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static const unsigned int idLen = 32;
    static std::mt19937 gen;
    static std::uniform_int_distribution<int> pick(0,alphanums.length()-1);
    global_rng_lock.lock();
    std::string id;
    for (unsigned int i = 0; i < idLen; i++) {
        id += alphanums[pick(gen)];
    }
    global_rng_lock.unlock();
    return id;
}

void Ev3Server::processCommand(const std::vector<uint8_t> & buffer,
        const std::shared_ptr<Ev3ServerConnection> & connection) {

    Ev3Command command;
    command.ParseFromString(std::string(buffer.begin(),buffer.end()));

    global_stream_lock.lock();
    // std::cout << "[" << __FUNCTION__ << "]" << std::endl;
    global_stream_lock.unlock();


    if (command.type() == Ev3Command_Type_INIT) {
        const std::string newId = genIdentifier();
        Ev3Command response;
        response.set_type(Ev3Command_Type_INIT);
        response.set_id(newId);
        std::string responseStr;
        response.SerializeToString(&responseStr);
        connection->Send(
                std::vector<uint8_t>(responseStr.begin(),responseStr.end()));

        // add to records
        id.insert(newId);
        idToConn[newId] = connection;
        connToId[connection] = newId;
    }
}

void Ev3Server::sendCommand(const std::string & recipient,
        const Ev3Command & command) {

    const std::shared_ptr<Ev3ServerConnection> & connection =
        idToConn.at(recipient);

    std::string commandStr;
    command.SerializeToString(&commandStr);
    connection->Send(
            std::vector<uint8_t>(commandStr.begin(),commandStr.end()));
}


void Ev3Server::quit() {
    Ev3Command command;
    command.set_type(Ev3Command_Type_QUIT);

    std::set<std::string>::const_iterator it,end;
    end = id.end();
    for (it = id.begin(); it != end; ++it) {
        command.set_id(*it);
        std::string commandStr;
        command.SerializeToString(&commandStr);

        const std::shared_ptr<Ev3ServerConnection> & connection =
            idToConn.at(*it);
        connection->Send(
                std::vector<uint8_t>(commandStr.begin(),commandStr.end()));

        drop(connection);
    }
}


const std::set<std::string> & Ev3Server::getId() const {
    return id;
}

void Ev3Server::drop(const std::shared_ptr<Ev3ServerConnection> & connection) {
    if(connToId.find(connection) != connToId.end()) {
        const std::string connId = connToId.at(connection);
        connToId.erase(connection);
        idToConn.erase(connId);
        id.erase(connId);
    }
}

void Ev3ServerConnection::OnAccept( const std::string & host, uint16_t port )
{
    global_stream_lock.lock();
    // std::cout << "[" << __PRETTY_FUNCTION__ << "] "
    // 					<< host << ":" << port << std::endl;
    global_stream_lock.unlock();

    // Start the next receive
    Recv();
}


void Ev3ServerConnection::OnConnect( const std::string & host, uint16_t port )
{
    global_stream_lock.lock();
    // std::cout << "[" << __PRETTY_FUNCTION__ << "] "
    // 					<< host << ":" << port << std::endl;
    global_stream_lock.unlock();

    // Start the next receive
    Recv();
}

void Ev3ServerConnection::OnSend( const std::vector< uint8_t > & buffer )
{
    // global_stream_lock.lock();
    // std::cout << "[" << __PRETTY_FUNCTION__ << "] "
    // 					<< buffer.size() << " bytes" << std::endl;
    // std::cout << std::endl;
    // global_stream_lock.unlock();
}

void Ev3ServerConnection::OnRecv( std::vector< uint8_t > & buffer )
{

    // global_stream_lock.lock();
    // std::cout << "[" << __FUNCTION__ << "]" << std::endl;
    // global_stream_lock.unlock();

    ev3Server->processCommand(buffer,
            std::dynamic_pointer_cast<Ev3ServerConnection>(shared_from_this()));

    // Start the next receive
    Recv();
}

void Ev3ServerConnection::OnTimer( const std::chrono::milliseconds & delta )
{
    // global_stream_lock.lock();
    // std::cout << "[" << __PRETTY_FUNCTION__ << "] " << delta << std::endl;
    // global_stream_lock.unlock();
}

void Ev3ServerConnection::OnError( const asio::error_code & error )
{
    global_stream_lock.lock();
    // std::cout << "[" << __PRETTY_FUNCTION__ << "] " << error
    // 					<< ": " << error.message() << std::endl;
    global_stream_lock.unlock();

    if (error != asio::error::operation_aborted)
        this->ev3Server->drop(
                std::dynamic_pointer_cast<Ev3ServerConnection>(
                        shared_from_this()));
}

Ev3ServerConnection::Ev3ServerConnection(
        std::shared_ptr<Ev3Server> ev3Server,
        std::shared_ptr< Hive > hive )
    : Connection( hive ), ev3Server(ev3Server)
{
}

Ev3ServerConnection::~Ev3ServerConnection()
{
}

std::shared_ptr<Connection> Ev3ServerConnection::NewConnection(){
    return std::shared_ptr<Ev3ServerConnection>(
            new Ev3ServerConnection(this->ev3Server,this->GetHive()));
}

bool Ev3Acceptor::OnAccept( std::shared_ptr< Connection > connection,
        const std::string & host, uint16_t port )
{
    global_stream_lock.lock();
    // std::cout << "[" << __PRETTY_FUNCTION__ << "] "
    // 					<< host << ":" << port << std::endl;
    global_stream_lock.unlock();

    return true;
}

void Ev3Acceptor::OnTimer( const std::chrono::milliseconds & delta )
{
    // global_stream_lock.lock();
    // std::cout << "[" << __PRETTY_FUNCTION__ << "] " << delta << std::endl;
    // std::map<std::shared_ptr<Ev3ServerConnection>,std::string >
    // 	::const_iterator
    // 	it,end;
    // end = ballSrv->connToId.end();
    // for (it = ballSrv->connToId.begin(); it != end; ++it) {
    // 	std::cout << it->first << " : " << it->second << std::endl;
    // }
    // global_stream_lock.unlock();
}

void Ev3Acceptor::OnError( const asio::error_code & error )
{
    global_stream_lock.lock();
    // std::cout << "[" << __PRETTY_FUNCTION__ << "] " << error << std::endl;
    global_stream_lock.unlock();
}
Ev3Acceptor::Ev3Acceptor( std::shared_ptr<Ev3Server> ev3Server,
        std::shared_ptr< Hive > hive )
    : Acceptor( hive ), ev3Server(ev3Server)
{
}

Ev3Acceptor::~Ev3Acceptor()
{
}


InputPoller::InputPoller(std::shared_ptr< Ev3Server > & server)
    : server(server), mode(ROBOT_SELECT),
      width(60), height(10),
      startx((80 - width) / 2), starty((24 - height) / 2),
      xVel(0), zRot(0),
      choice(0) {
}

void InputPoller::poll() {
    initscr();
    clear();
    noecho();
    cbreak();	/* Line buffering disabled. pass on everything */

    menu_win = newwin(height,width,starty,startx);

    // nodelay(menu_win, TRUE);

    keypad(menu_win, TRUE);
    refresh();

    refresh_id();
    print_menu();
    while(mode != ROBOT_QUIT) {
        // const int ch = wgetch(menu_win);

        // if(ch == 114) {
        //     // r for refresh
        // } else if (ch == 113) {
        //     // q for quit
        //     mode = ROBOT_QUIT;
        // }else {
        switch (mode) {
        case ROBOT_SELECT: {
            poll_select();
            break;
        }
        case ROBOT_DRIVE: {
            poll_drive();
            break;
        }
        default:
            break;
        }

        // refresh data and show the menu
        refresh_id();
        print_menu();
    }
    clrtoeol();
    refresh();
    endwin();
}

void InputPoller::refresh_id() {
    id.clear();

    const std::set<std::string> & idSet = this->server->getId();
    std::set<std::string>::const_iterator it, end;
    end = idSet.end();
    for (it = idSet.begin(); it != end; ++it) {
        id.push_back(*it);
    }
}

void InputPoller::poll_select() {
    while(mode == ROBOT_SELECT) {
        const int ch = wgetch(menu_win);
        switch(ch) {
        case KEY_UP:
            --choice;
            if(choice < 0)
                choice = id.size() - 1;
            break;
        case KEY_DOWN:
            ++choice;
            if(choice >= id.size())
                choice = 0;
            break;
        case 10:
            // return
            idChoice = id.at(choice);
            mode = ROBOT_DRIVE;
            break;
        case 113:
            // q
            mode = ROBOT_QUIT;
            server->quit();
            break;
        default:
            break;
        }
        refresh_id();
        print_menu();
    }
}


void InputPoller::throttle_speeds(const bool & x_vel_changed) {
    const double w = 2.25;
    if (x_vel_changed) {
        xVel = std::max(std::min(xVel,100.0),-100.0);


        const double zRotUpr = std::min((xVel + 100.0) / w, (100 - xVel) / w);
        const double zRotLwr = std::max((xVel - 100.0) / w, (-xVel - 100)/ w);

        zRot = std::max(std::min(zRot,zRotUpr),zRotLwr);
    } else {
        zRot = std::max(std::min(zRot,100.0/w),-100.0/w);

        const double xVelUpr = std::min(100 + zRot*w, 100 - zRot*w);
        const double xVelLwr = std::max(-100 + zRot*w, -100 - zRot*w);
        xVel = std::max(std::min(xVel,xVelUpr),xVelLwr);
    }
}


void InputPoller::poll_drive() {
    while(mode == ROBOT_DRIVE) {
        const int ch = wgetch(menu_win);
        switch (ch) {
        case KEY_UP:
            xVel += 1.0;
            throttle_speeds(true);
            break;
        case KEY_DOWN:
            xVel -= 1.0;
            throttle_speeds(true);
            break;
        case KEY_LEFT:
            zRot += 1.0;
            throttle_speeds(false);
            break;
        case KEY_RIGHT:
            zRot -= 1.0;
            throttle_speeds(false);
            break;
        case 32:
            // SPACEBAR
            xVel = 0.0;
            zRot = 0.0;
            break;
        case 113:
            mode = ROBOT_SELECT;
            xVel = 0.0;
            zRot = 0.0;
            break;
        default:
            break;
        }

        Ev3Command command;
        command.set_type(Ev3Command_Type_DRIVE);
        command.set_id(idChoice);
        command.set_xvel(xVel);
        command.set_zrot(zRot);
        server->sendCommand(idChoice,command);

        print_menu();
    }
}

void InputPoller::print_menu() {
    wclear(menu_win);
    switch (mode) {
    case ROBOT_SELECT: {
        print_menu_select();
        break;
    }
    case ROBOT_DRIVE: {
        print_menu_drive();
        break;
    }
    default:
        break;
    }
}


void InputPoller::print_menu_select() {
    int x, y, i;

    mvwprintw(menu_win,1,1,"%s", "Registered Robots");

    x = 2;
    y = 2;
    box(menu_win, 0, 0);
    for(i = 0; i < id.size(); ++i)
    {
        if(choice == i) /* High light the present choice */
        {
            wattron(menu_win, A_REVERSE);
            mvwprintw(menu_win, y, x, "%s", id.at(i).c_str());
            wattroff(menu_win, A_REVERSE);
        }
        else
            mvwprintw(menu_win, y, x, "%s", id.at(i).c_str());
        ++y;
    }
    wrefresh(menu_win);
}


void InputPoller::print_menu_drive() {
    box(menu_win, 0, 0);
    mvwprintw(menu_win,1,1,"%s: %s","Robot",idChoice.c_str());
    mvwprintw(menu_win,3,1,"vel: % 6.3f",xVel);
    mvwprintw(menu_win,4,1,"rot: % 6.3f",zRot);
    wrefresh(menu_win);
}


} // Ev3Controller namespace

int main( int argc, char * argv[] )
{
    std::shared_ptr<Ev3Controller::Ev3Server> ballSrv(
            new Ev3Controller::Ev3Server() );

    std::shared_ptr< Hive > hive( new Hive() );

    std::shared_ptr< Ev3Controller::Ev3Acceptor > acceptor(
      new Ev3Controller::Ev3Acceptor( ballSrv, hive ) );
    acceptor->Listen( "0.0.0.0", 7777 );

    std::shared_ptr< Ev3Controller::Ev3ServerConnection > connection(
      new Ev3Controller::Ev3ServerConnection(ballSrv,hive));
    acceptor->Accept( connection );

    std::thread worker_thread(
      std::bind(&Ev3Controller::ServerRunThread, hive));

    Ev3Controller::InputPoller inputPoller(ballSrv);

    inputPoller.poll();

    std::this_thread::sleep_for(
            std::chrono::seconds(3));

    hive->Stop();

    worker_thread.join();

    return 0;
}
