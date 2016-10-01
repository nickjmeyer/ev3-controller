#include "networkWrapper.hpp"
#include "command.pb.h"
#include "ev3Client.hpp"
#include <iostream>
#include <thread>
#include <mutex>
#include <google/protobuf/text_format.h>
#include <iomanip>
#include <sstream>
#include <functional>
#include <chrono>

namespace Ev3Controller {

std::mutex global_stream_lock;


Ev3Client::Ev3Client()
	: isInit_(false) {
}

void Ev3Client::processCommand(const std::vector<uint8_t> & buffer,
	const std::shared_ptr<Ev3ClientConnection> & connection) {

	Ev3Command command;
	command.ParseFromString(std::string(buffer.begin(),buffer.end()));

	global_stream_lock.lock();
	std::cout << "[" << __FUNCTION__ << "]" << std::endl;
	global_stream_lock.unlock();

	if(command.type() == Ev3Command_Type_INIT) {
		init(command.id(),connection);
	} else if (command.type() == Ev3Command_Type_QUIT) {
		this->connection->GetHive()->Stop();
	} else if(command.type() == Ev3Command_Type_VELOCITY) {
		global_stream_lock.lock();
		std::cout << "VELOCITY MESSAGE" << std::endl;
		global_stream_lock.unlock();
	}

}

void Ev3Client::init(std::string id,
	const std::shared_ptr<Ev3ClientConnection> connection) {
	global_stream_lock.lock();
	std::cout << "[" << __FUNCTION__ << "]: " << id << std::endl;
	global_stream_lock.unlock();

	this->connection = connection;
	this->id = id;

	this->isInit_ = true;
}

bool Ev3Client::isInit() const {
	return isInit_;
}

void Ev3ClientConnection::OnAccept( const std::string & host, uint16_t port )
{
	global_stream_lock.lock();
	std::cout << "[" << __PRETTY_FUNCTION__ << "] "
						<< host << ":" << port << std::endl;
	global_stream_lock.unlock();

	// Start the next receive
	Recv();
}

void Ev3ClientConnection::OnConnect( const std::string & host, uint16_t port )
{
	global_stream_lock.lock();
	std::cout << "[" << __PRETTY_FUNCTION__ << "] "
						<< host << ":" << port << std::endl;
	global_stream_lock.unlock();

	// create message
	Ev3Command command;
	command.set_type(Ev3Command_Type_INIT);

	// serialize to byte buffer
	std::string commandStr;
	command.SerializeToString(&commandStr);
	std::vector<uint8_t> buffer(commandStr.begin(),commandStr.end());

	// send byte buffer
	Send(buffer);

	// Start the next receive
	Recv();
}

void Ev3ClientConnection::OnSend( const std::vector< uint8_t > & buffer )
{
	// global_stream_lock.lock();
	// std::cout << "[" << __PRETTY_FUNCTION__ << "] "
	// 					<< buffer.size() << " bytes" << std::endl;
	// global_stream_lock.unlock();
}

void Ev3ClientConnection::OnRecv( std::vector< uint8_t > & buffer )
{

	// global_stream_lock.lock();
	// std::cout << "[" << __PRETTY_FUNCTION__ << "] "
	// 					<< buffer.size() << " bytes" << std::endl;
	// global_stream_lock.unlock();

	ev3Client->processCommand(buffer,
		std::dynamic_pointer_cast<Ev3ClientConnection>(shared_from_this()));


	// Start the next receive
	Recv();
}

void Ev3ClientConnection::OnTimer( const std::chrono::milliseconds & delta )
{
	// global_stream_lock.lock();
	// std::cout << "[" << __PRETTY_FUNCTION__ << "] " << delta << std::endl;
	// global_stream_lock.unlock();
}

void Ev3ClientConnection::OnError( const asio::error_code & error )
{
	global_stream_lock.lock();
	std::cout << "[" << __PRETTY_FUNCTION__ << "] " << error
						<< ": " << error.message() << std::endl;
	global_stream_lock.unlock();
}

Ev3ClientConnection::Ev3ClientConnection( std::shared_ptr<Ev3Client> ev3Client,
	std::shared_ptr< Hive > hive )
	: Connection( hive ), ev3Client(ev3Client)
{
}

Ev3ClientConnection::~Ev3ClientConnection()
{
}

std::shared_ptr<Connection> Ev3ClientConnection::NewConnection () {
	return std::shared_ptr<Ev3ClientConnection>(new Ev3ClientConnection(
			this->ev3Client,this->GetHive()));
}


} // Ev3Controller namespace



int main( int argc, char * argv[] )
{
	std::string hostname = "localhost";
	if(argc == 2) {
		hostname = argv[1];
	} else if (argc > 2) {
		std::cout << "Too many command line arguments." << std::endl;
		return 1;
	}


	std::shared_ptr<Ev3Controller::Ev3Client> ev3Client(
		new Ev3Controller::Ev3Client() );

	std::shared_ptr< Hive > hive( new Hive() );

	std::shared_ptr< Ev3Controller::Ev3ClientConnection > connection(
		new Ev3Controller::Ev3ClientConnection( ev3Client,hive ) );
	connection->Connect( hostname , 7777 );

	hive->Run();

	return 0;
}