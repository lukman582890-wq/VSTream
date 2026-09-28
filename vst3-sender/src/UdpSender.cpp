#include "vstream/UdpSender.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif
#include <cstring>
namespace vstream {
struct UdpSender::Impl {
#ifdef _WIN32
    SOCKET socket=INVALID_SOCKET;
#else
    int socket=-1;
#endif
    sockaddr_storage address{};
    int addressLength=0;
    bool open=false;
};
UdpSender::UdpSender():impl_(new Impl){}
UdpSender::~UdpSender(){close();delete impl_;}
bool UdpSender::open(const std::string& host,std::uint16_t port){
#ifdef _WIN32
    WSADATA wsa{}; if(WSAStartup(MAKEWORD(2,2),&wsa)!=0)return false;
    impl_->socket=::socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(impl_->socket==INVALID_SOCKET)return false;
#else
    impl_->socket=::socket(AF_INET,SOCK_DGRAM,0); if(impl_->socket<0)return false;
#endif
    sockaddr_in addr{}; addr.sin_family=AF_INET; addr.sin_port=htons(port);
    if(::inet_pton(AF_INET,host.c_str(),&addr.sin_addr)!=1){close();return false;}
    std::memcpy(&impl_->address,&addr,sizeof(addr)); impl_->addressLength=sizeof(addr); impl_->open=true; return true;
}
void UdpSender::close(){
    if(!impl_||!impl_->open)return;
#ifdef _WIN32
    closesocket(impl_->socket); WSACleanup();
#else
    ::close(impl_->socket);
#endif
    impl_->open=false;
}
bool UdpSender::isOpen()const noexcept{return impl_&&impl_->open;}
bool UdpSender::send(const std::uint8_t* data,std::size_t size){
    if(!isOpen()||size>65507)return false;
#ifdef _WIN32
    return ::sendto(impl_->socket,reinterpret_cast<const char*>(data),(int)size,0,reinterpret_cast<const sockaddr*>(&impl_->address),impl_->addressLength)==(int)size;
#else
    return ::sendto(impl_->socket,data,size,0,reinterpret_cast<const sockaddr*>(&impl_->address),impl_->addressLength)==(ssize_t)size;
#endif
}
}
