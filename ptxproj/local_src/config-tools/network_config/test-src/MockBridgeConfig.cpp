// SPDX-License-Identifier: GPL-2.0-or-later

#include "MockBridgeConfig.hpp"
#include "Status.hpp"
#include <memory>

namespace netconf {
namespace api {

::std::unique_ptr<MockBridgeConfig, void (*)(MockBridgeConfig* mock)> activeMock { nullptr, []([[maybe_unused]]MockBridgeConfig* mock) {
} };

Status GetBridgeConfig(BridgeConfig& config) {
  if (activeMock){
    return activeMock->GetBridgeConfig(config);
  }
  return Status{};
}

Status SetBridgeConfig(const BridgeConfig &config) {
  if (activeMock){
    return activeMock->SetBridgeConfig(config);
  }
  return Status { StatusCode::BRIDGE_NAME_INVALID };
}

MockBridgeConfig::MockBridgeConfig() {
  activeMock.reset(this);
}

MockBridgeConfig::~MockBridgeConfig() {
  activeMock.reset();
}

}
}
