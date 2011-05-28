#include "Raw_Ethernet.h++"

#include <algorithm>
#include <cassert>
#include <cmath>

#include <boost/lexical_cast.hpp>

#include <arpa/inet.h>
#include <errno.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

namespace Raw_Ethernet {

  static void throw_with_errno(const std::string & what) {
    std::string msg(strerror(errno));
    throw Exception (
      what + ": (" + boost::lexical_cast<std::string>(errno) + ") " + msg
    );
  }

  Exception::Exception(const std::string & what) : runtime_error(what) {}

  struct Socket::Detail {
    int fd;
  };

  Socket::Socket(const std::string & if_name, uint16_t protocol) 
    : detail(new Socket::Detail)
  {
    // Open raw packet socket
    detail->fd = socket(AF_PACKET, SOCK_RAW, htons(protocol));
    if (detail->fd < 0) throw_with_errno("Could not open socket");

    // Get interface index from name
    if (if_name.size() >= IFNAMSIZ) {
      throw Exception("Interface name too long");
    }
    ifreq ifr;
    std::copy_n(if_name.c_str(), if_name.size() + 1, ifr.ifr_name);
    {
      int err = ioctl(detail->fd, SIOCGIFINDEX, &ifr);
      if (err) {
        throw_with_errno("Could not get interface index");
      }
    }

    // Bind socket to requested interface
    struct sockaddr_ll addr;
    memset(&addr, 0, sizeof(addr));
    addr.sll_family = AF_PACKET;
    addr.sll_protocol = htons(protocol);
    addr.sll_ifindex = ifr.ifr_ifindex;
    {
      int err = bind(detail->fd, (struct sockaddr *)(&addr), sizeof(addr));
      if (err) {
        throw_with_errno("Could not bind socket");
      }
    }

  }

  Socket::~Socket() {
    close(detail->fd);
  }

  void Socket::send_packet(const std::vector<uint8_t> & packet) {
    ssize_t err_or_size =
      send(detail->fd, packet.data(), packet.size(), 0);
    if (err_or_size < 0) {
      throw_with_errno("Error while sending packet");
    } else if (size_t(err_or_size) != packet.size()) {
      throw_with_errno("Packet truncated while sending");
    }
  }

  boost::optional<std::vector<uint8_t>> Socket::receive_packet() {
    std::vector<uint8_t> packet(ETH_FRAME_LEN);
    ssize_t err_or_size =
      recv(detail->fd, packet.data(), packet.size(), MSG_TRUNC);
    if (err_or_size < 0) {
      switch (errno) {
        case EAGAIN: return boost::none;
        default: throw_with_errno("Error while receiving packet");
      }
    } else {
      assert(err_or_size <= ETH_FRAME_LEN);
      packet.resize(err_or_size);
    }
    return packet;
  }

  void Socket::set_receive_timeout(double seconds) {
    if (seconds < 0) seconds = 0;
    timeval tv;
    tv.tv_sec  = floor(seconds);
    tv.tv_usec = floor((seconds - tv.tv_sec) / 1e-6);
    if (seconds > 0 && tv.tv_sec == 0 && tv.tv_usec == 0) {
      tv.tv_usec = 1;
    }
    int err =
      setsockopt(detail->fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    if (err != 0) {
      throw_with_errno("Could not set socket receive timeout");
    }
  }

}
