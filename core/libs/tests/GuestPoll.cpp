#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif
#include "prx/libc/include/general/VabiMacros.hpp"
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
struct PollDescriptor {
    int descriptor;
    short events;
    short revents;
};
extern "C" {
int APS5_VABI socket_nid_postfix(int, int, int);
int APS5_VABI bind_nid_postfix(int, const void*, std::uint32_t);
int APS5_VABI listen_nid_postfix(int, int);
int APS5_VABI accept_nid_postfix(int, void*, std::uint32_t*);
int APS5_VABI connect_nid_postfix(int, const void*, std::uint32_t);
int APS5_VABI getsockname_nid_postfix(int, void*, std::uint32_t*);
std::int64_t APS5_VABI send_nid_postfix(int, const void*, std::uint64_t, int);
std::int64_t APS5_VABI recv_nid_postfix(int, void*, std::uint64_t, int);
int APS5_VABI close_nid_postfix(int);
int APS5_VABI poll_nid_postfix(PollDescriptor*, std::uint32_t, int);
int APS5_VABI fcntl_nid_postfix(int, int, ...);
int* APS5_VABI __error_nid_postfix();
}
#ifdef _WIN32
using NativeSocket = SOCKET;
constexpr NativeSocket NativeInvalid = INVALID_SOCKET;
static void CloseNative(NativeSocket socket) { closesocket(socket); }
#else
using NativeSocket = int;
constexpr NativeSocket NativeInvalid = -1;
static void CloseNative(NativeSocket socket) { close(socket); }
#endif
static void Require(bool value) { if (!value) std::abort(); }
constexpr short In = 0x1;
constexpr short Priority = 0x2;
constexpr short ReadBand = 0x80;
constexpr short Out = 0x4;
constexpr short Hup = 0x10;
constexpr short Invalid = 0x20;
static void PollsUrgentData() {
    const int listener = socket_nid_postfix(2, 1, 0);
    Require(listener >= 0);
    std::array<unsigned char, 16> address{16, 2, 0, 0, 127, 0, 0, 1};
    Require(bind_nid_postfix(listener, address.data(), address.size()) == 0);
    std::uint32_t size = address.size();
    Require(getsockname_nid_postfix(listener, address.data(), &size) == 0);
    Require(listen_nid_postfix(listener, 4) == 0);
    const int client = socket_nid_postfix(2, 1, 0);
    Require(client >= 0 && fcntl_nid_postfix(client, 4, 4) == 0);
    if (connect_nid_postfix(client, address.data(), address.size()) != 0)
        Require(*__error_nid_postfix() == 35 || *__error_nid_postfix() == 36);
    PollDescriptor connecting{client, Priority | Out, -1};
    Require(poll_nid_postfix(&connecting, 1, 1000) == 1 && connecting.revents == Out);
    const int server = accept_nid_postfix(listener, nullptr, nullptr);
    Require(server >= 0);
    PollDescriptor pair[2]{{server, Priority | Out, -1}, {client, In | Priority | Out | ReadBand, -1}};
    Require(poll_nid_postfix(pair, 2, 0) == 2 && pair[0].revents == Out && pair[1].revents == Out);
    char byte = 'u';
    Require(send_nid_postfix(client, &byte, 1, 1) == -1 && *__error_nid_postfix() == 45);
    Require(recv_nid_postfix(server, &byte, 1, 1) == -1 && *__error_nid_postfix() == 45);
    const NativeSocket peer = ::socket(AF_INET, SOCK_STREAM, 0);
    Require(peer != NativeInvalid);
    sockaddr_in native{};
    native.sin_family = AF_INET;
    std::memcpy(&native.sin_port, address.data() + 2, 2);
    std::memcpy(&native.sin_addr, address.data() + 4, 4);
    Require(::connect(peer, reinterpret_cast<const sockaddr*>(&native), sizeof(native)) == 0);
    const int urgent = accept_nid_postfix(listener, nullptr, nullptr);
    Require(urgent >= 0);
    PollDescriptor waiting{urgent, Priority, -1};
    Require(poll_nid_postfix(&waiting, 1, 0) == 0 && waiting.revents == 0);
    Require(::send(peer, "u", 1, MSG_OOB) == 1);
    Require(poll_nid_postfix(&waiting, 1, 1000) == 1 && waiting.revents == Priority);
    PollDescriptor bands{urgent, In | Priority | Out, -1};
    Require(poll_nid_postfix(&bands, 1, 0) == 1 && bands.revents == (Priority | Out));
#ifdef _WIN32
    PollDescriptor band{urgent, ReadBand, -1};
    Require(poll_nid_postfix(&band, 1, 0) == 1 && band.revents == ReadBand);
#endif
    CloseNative(peer);
    Require(close_nid_postfix(urgent) == 0);
    Require(close_nid_postfix(server) == 0);
    Require(close_nid_postfix(client) == 0);
    Require(close_nid_postfix(listener) == 0);
}
int main() {
    PollsUrgentData();
    const int listener = socket_nid_postfix(2, 1, 0);
    Require(listener >= 0);
    std::array<unsigned char, 16> address{16, 2, 0, 0, 127, 0, 0, 1};
    Require(bind_nid_postfix(listener, address.data(), address.size()) == 0);
    std::uint32_t size = address.size();
    Require(getsockname_nid_postfix(listener, address.data(), &size) == 0);
    Require(listen_nid_postfix(listener, 4) == 0);
    PollDescriptor single{listener, In, -1};
    Require(poll_nid_postfix(&single, 1, 0) == 0 && single.revents == 0);
    const auto start = std::chrono::steady_clock::now();
    Require(poll_nid_postfix(&single, 1, 50) == 0);
    Require(std::chrono::steady_clock::now() - start >= std::chrono::milliseconds(40));
    const int client = socket_nid_postfix(2, 1, 0);
    Require(client >= 0 && connect_nid_postfix(client, address.data(), address.size()) == 0);
    Require(poll_nid_postfix(&single, 1, 1000) == 1 && single.revents == In);
    const int server = accept_nid_postfix(listener, nullptr, nullptr);
    Require(server >= 0);
    PollDescriptor pair[3]{{server, In, 0}, {-1, In, 7}, {client, Out, 0}};
    Require(poll_nid_postfix(pair, 3, 1000) == 1);
    Require(pair[0].revents == 0 && pair[1].revents == 0 && pair[2].revents == Out);
    const char message[] = "poll";
    Require(send_nid_postfix(client, message, sizeof(message), 0) == sizeof(message));
    pair[2].events = In;
    Require(poll_nid_postfix(pair, 3, 1000) == 1 && pair[0].revents == In && pair[2].revents == 0);
    char buffer[8]{};
    Require(recv_nid_postfix(server, buffer, sizeof(buffer), 0) == sizeof(message));
    Require(close_nid_postfix(client) == 0);
    Require(poll_nid_postfix(pair, 1, 1000) == 1 && (pair[0].revents & (In | Hup)) != 0);
    Require(recv_nid_postfix(server, buffer, sizeof(buffer), 0) == 0);
    PollDescriptor closed[2]{{client, In, 0}, {listener, In, 0}};
    Require(poll_nid_postfix(closed, 2, -1) == 1 && closed[0].revents == Invalid && closed[1].revents == 0);
    Require(poll_nid_postfix(nullptr, 0, 10) == 0);
    Require(poll_nid_postfix(nullptr, 1, 0) == -1 && *__error_nid_postfix() == 14);
    Require(poll_nid_postfix(&single, 1, -2) == -1 && *__error_nid_postfix() == 22);
    Require(close_nid_postfix(server) == 0);
    Require(close_nid_postfix(listener) == 0);
}
