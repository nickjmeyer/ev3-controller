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
	std::cout << "thread started" << std::endl;
	global_stream_lock.unlock();

	hive->Run();

	global_stream_lock.lock();
	std::cout << "thread ended" << std::endl;
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
	std::cout << "[" << __FUNCTION__ << "]" << std::endl;
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
    connToId[connection] = newId;
    idToConn[newId] = connection;
	}
}

const std::set<std::string> & Ev3Server::getId() const {
    return id;
}

void Ev3Server::drop(std::shared_ptr<Ev3ServerConnection> connection) {
    std::string connId = connToId.at(connection);
    connToId.erase(connection);
    idToConn.erase(connId);
    id.erase(connId);
}

void Ev3ServerConnection::OnAccept( const std::string & host, uint16_t port )
{
	global_stream_lock.lock();
	std::cout << "[" << __PRETTY_FUNCTION__ << "] "
						<< host << ":" << port << std::endl;
	global_stream_lock.unlock();

	// Start the next receive
	Recv();
}


void Ev3ServerConnection::OnConnect( const std::string & host, uint16_t port )
{
	global_stream_lock.lock();
	std::cout << "[" << __PRETTY_FUNCTION__ << "] "
						<< host << ":" << port << std::endl;
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
	std::cout << "[" << __PRETTY_FUNCTION__ << "] " << error
						<< ": " << error.message() << std::endl;
  global_stream_lock.unlock();

  this->ev3Server->drop(
          std::dynamic_pointer_cast<Ev3ServerConnection>(shared_from_this()));
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
	std::cout << "[" << __PRETTY_FUNCTION__ << "] "
						<< host << ":" << port << std::endl;
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
	std::cout << "[" << __PRETTY_FUNCTION__ << "] " << error << std::endl;
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
      width(30), height(10),
      xVel(0), zRot(0) {

    initscr();
    clear();
    noecho();
    cbreak();	/* Line buffering disabled. pass on everything */

    int startx = (80 - WIDTH) / 2;
    int starty = (24 - HEIGHT) / 2;

    menu_win = newwin(height,width,startx,starty);
    keypad(menu_win, TRUE);
}

void InputPoller::poll() {
    while(mode != ROBOT_QUIT) {
        refresh();
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
            mode = ROBOT_QUIT;
            break;
        }
    }
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
    int c;
    int choice = 0;

    print_menu();
    while(1)
    {
        c = wgetch(menu_win);
        refresh_id();
        choice %= id.size();

        switch(c) {
        case KEY_UP:
            --choice;
            choice %= id.size();
            break;
        case KEY_DOWN:
            ++choice;
            choice %= id.size();
        case 10:
            idChoice = id.at(choice);
            break;
        default:
            refresh();
            break;
        }
        print_menu();
        if(choice != 0)	/* User did a choice come out of the infinite loop */
            break;
    }
    clrtoeol();
    refresh();
    endwin();
}

void InputPoller::print_menu() {
    switch (mode) {
    case ROBOT_SELECT: {
        print_select();
        break;
    }
    case ROBOT_DRIVE: {
        print_drive();
        break;
    }
    default:
        break;
    }
}


void InputPoller::print_select() {
    int x, y, i;

    x = 2;
    y = 2;
    box(menu_win, 0, 0);
    for(i = 0; i < choices.size(); ++i)
    {	if(highlight == i) /* High light the present choice */
        {	wattron(menu_win, A_REVERSE);
            mvwprintw(menu_win, y, x, "%s", choices[i].c_str());
            wattroff(menu_win, A_REVERSE);
        }
        else
            mvwprintw(menu_win, y, x, "%s", choices[i].c_str());
        ++y;
    }
    wrefresh(menu_win);
}

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

  // Ev3Controller::poll_input( hive );

  hive->Stop();

	worker_thread.join();

	return 0;
}
