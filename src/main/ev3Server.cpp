#include "ev3Server.hpp"
#include <unistd.h>
#include <iostream>
#include <random>
#include <mutex>
#include <queue>
#include <thread>

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
	std::shared_ptr<Ev3ServerConnection> connection) {

	Ev3Command command;
	command.ParseFromString(std::string(buffer.begin(),buffer.end()));

	global_stream_lock.lock();
	std::cout << "[" << __FUNCTION__ << "]" << std::endl;
	global_stream_lock.unlock();


	if (command.type() == Ev3Command_Type_INIT) {
		const std::string id = genIdentifier();
		Ev3Command response;
		response.set_type(Ev3Command_Type_INIT);
		response.set_id(id);
		std::string responseStr;
		response.SerializeToString(&responseStr);
		connection->Send(
			std::vector<uint8_t>(responseStr.begin(),responseStr.end()));
	}
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

	std::cin.clear();
	std::cin.ignore();



	hive->Stop();

	worker_thread.join();

	return 0;
}
