#ifndef __Raw_Ethernet__Header__
#define __Raw_Ethernet__Header__

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/optional.hpp>

namespace Raw_Ethernet {

  struct Exception : std::runtime_error {
    explicit Exception(const std::string & what);
  };

  struct Socket {
    Socket(const std::string & if_name, uint16_t protocol);
    ~Socket();

    void send_packet(const std::vector<uint8_t> & packet);
    boost::optional<std::vector<uint8_t>> receive_packet();

    void set_receive_timeout(double seconds);

    private:
    class Detail;
    std::unique_ptr<Detail> detail;
  };

};

#endif
