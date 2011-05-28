#include "Raw_Ethernet.h++"

#include <iostream>

#include <boost/spirit/include/karma.hpp>
namespace karma = boost::spirit::karma;

int main() {

  Raw_Ethernet::Socket socket("eth1", 0xf1bb);
  socket.set_receive_timeout(1e-3);

  std::cerr << "Sending ..." << std::endl;
  socket.send_packet({
    0x00, 0x23, 0x08, 0x15, 0xaa, 0xc4, // DST
    0x00, 0x23, 0x08, 0x15, 0xaa, 0xc4, // SRC
    0xf1, 0xbb,                         // PROTO
    0xef, 0xbe, 0xad, 0xde              // DATA
  });

  for (int i=0; i<4; ++i) {
    std::cerr << "Receiving ..." << std::endl;
    std::cerr
      << karma::format (
        -("[" << karma::right_align(2,'0')[karma::hex] % " " << "]₁₆\n"),
        socket.receive_packet()
      )
    ;
  }

}
