#ifndef EV3_SERVER_HPP
#define EV3_SERVER_HPP

#include "networkWrapper.hpp"
#include "command.pb.h"
#include <queue>
#include <asio/error.hpp>

namespace Ev3Controller {

// random identifier
std::string genIdentifier();

class Ev3Server;
class Ev3ServerConnection;
class Ev3Acceptor;

class Ev3Server {
public:
	void processCommand(const std::vector<uint8_t> & buffer,
		const std::shared_ptr<Ev3ServerConnection> connection);

	void quit();

private:
	// connection containers
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

} // Ev3Controller namespace

#endif
