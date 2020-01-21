#ifndef NETCONFD_TEST_EXTERN_MOCKS_MOCKIDEVICEPROPERTIESPROVIDER_HPP_
#define NETCONFD_TEST_EXTERN_MOCKS_MOCKIDEVICEPROPERTIESPROVIDER_HPP_

#include <gmock/gmock.h>
#include "IDevicePropertiesProvider.hpp"

namespace netconfd {

class MockIDevicePropertiesProvider : public IDevicePropertiesProvider {
 public:
  MOCK_CONST_METHOD0(GetProductInterfaces, Interfaces() );
  MOCK_CONST_METHOD0(GetOSInterfaces, Interfaces() );
  MOCK_CONST_METHOD0(GetInterfacesNameMapping, InterfaceNameMapping() );
  MOCK_CONST_METHOD1(HasInterface, bool(const ::std::string&) );
  MOCK_CONST_METHOD0(GetMac, ::std::string() );
  MOCK_CONST_METHOD0(GetOrderNumber, ::std::string() );
  MOCK_CONST_METHOD1(GetIncrementedMac, ::std::string(uint32_t) );
  MOCK_CONST_METHOD0(GetHostname, ::std::string() );
};

}
  // namespace netconfd
#endif 
