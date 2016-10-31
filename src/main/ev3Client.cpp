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
#include <glog/logging.h>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

namespace Ev3Controller {

std::mutex global_stream_lock;


void ClientRunThread( const std::shared_ptr< Hive > & hive)
{
    global_stream_lock.lock();
    LOG(INFO) << "Starting thread.";
    global_stream_lock.unlock();

    hive->Run();

    global_stream_lock.lock();
    LOG(INFO) << "Quitting thread.";
    global_stream_lock.unlock();
}


#ifdef BUILD_FOR_EV3
Ev3Client::Ev3Client()
    : isInit_(false), alive_(true),
      robot(Ev3Robot(ev3dev::OUTPUT_A,ev3dev::OUTPUT_D,2.25)) {
}
#else
Ev3Client::Ev3Client()
    : isInit_(false), alive_(true),
      robot(Ev3Robot()) {
}
#endif

void Ev3Client::processCommand(const std::vector<uint8_t> & buffer,
        const std::shared_ptr<Ev3ClientConnection> & connection) {

    Ev3Command command;
    command.ParseFromString(std::string(buffer.begin(),buffer.end()));

    global_stream_lock.lock();
    LOG(INFO) << "Processing command.";
    global_stream_lock.unlock();

    if(command.type() == Ev3Command_Type_INIT) {
        init(command.id(),connection);
    } else if (command.type() == Ev3Command_Type_QUIT) {
        this->kill();
    } else if(command.type() == Ev3Command_Type_DRIVE) {
        this->robot.setVelocity(command);
    }

}

void Ev3Client::init(std::string id,
        const std::shared_ptr<Ev3ClientConnection> connection) {
    global_stream_lock.lock();
    LOG(INFO) << "Initializing robot.";
    global_stream_lock.unlock();

    this->connection = connection;
    this->id = id;

    this->isInit_ = true;
}

bool Ev3Client::isInit() const {
    return isInit_;
}

bool Ev3Client::alive() const {
    return alive_;
}

void Ev3Client::kill() {
    alive_ = false;
}

void Ev3ClientConnection::OnAccept( const std::string & host, uint16_t port )
{
    global_stream_lock.lock();
    LOG(INFO) << "Accepted.";
    global_stream_lock.unlock();

    // Start the next receive
    Recv();
}

void Ev3ClientConnection::OnConnect( const std::string & host, uint16_t port )
{
    global_stream_lock.lock();
    LOG(INFO) << "Connected.";
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
    global_stream_lock.lock();
    LOG(INFO) << "Sending.";
    global_stream_lock.unlock();
}

void Ev3ClientConnection::OnRecv( std::vector< uint8_t > & buffer )
{

    global_stream_lock.lock();
    LOG(INFO) << "Receiving.";
    global_stream_lock.unlock();

    ev3Client->processCommand(buffer,
            std::dynamic_pointer_cast<Ev3ClientConnection>(shared_from_this()));


    // Start the next receive
    Recv();
}

void Ev3ClientConnection::OnTimer( const std::chrono::milliseconds & delta )
{
    global_stream_lock.lock();
    LOG(INFO) << "Timer.";
    global_stream_lock.unlock();
}

void Ev3ClientConnection::OnError( const asio::error_code & error )
{
    global_stream_lock.lock();
    LOG(ERROR) << "Error: " << error.message() << ".";
    global_stream_lock.unlock();
    if (error == asio::error::eof || error == asio::error::connection_refused) {
        this->ev3Client->kill();
    }
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
    google::InitGoogleLogging(argv[0]);

    std::string hostname = TOSTRING(EV3_SERVER_HOSTNAME);
    LOG(INFO) << "Hostname: " << hostname << ".";

    std::shared_ptr<Ev3Controller::Ev3Client> ev3Client(
            new Ev3Controller::Ev3Client() );

    std::shared_ptr< Hive > hive( new Hive() );

    std::shared_ptr< Ev3Controller::Ev3ClientConnection > connection(
            new Ev3Controller::Ev3ClientConnection( ev3Client,hive ) );
    connection->Connect( hostname , 7777 );

    std::thread worker_thread(
            std::bind(&Ev3Controller::ClientRunThread, hive));

    while(ev3Client->alive()) {
        std::this_thread::sleep_for(
                std::chrono::seconds(1));
    }

    hive->Stop();

    worker_thread.join();

    return 0;
}
