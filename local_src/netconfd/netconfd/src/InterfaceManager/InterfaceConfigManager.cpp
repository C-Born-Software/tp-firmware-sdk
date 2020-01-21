// SPDX-License-Identifier: GPL-2.0-or-later

#include "InterfaceConfigManager.hpp"
#include <algorithm>
#include <boost/format.hpp>

#include "Logger.hpp"

namespace netconfd {

using namespace std::literals;

using ::std::copy_if;
using ::std::find_if;
using ::std::inserter;
using ::std::back_inserter;
using ::std::transform;

auto generate_default_port_config = [] (const ::std::string& linux_interface_name) noexcept { // NOLINT(cert-err58-cpp): There will nothing be thrown here
    // Create the PortConfig object without the 'eth' prefix.
    return InterfaceConfig {linux_interface_name.substr(3), InterfaceState::UP, Autonegotiation::ON};
  };

InterfaceConfigManager::InterfaceConfigManager(IInterfaceInformation& itf_info,
                                               IPersistence<InterfaceConfigs>& persistence_provider,
                                               IJsonConfigConverter& json_config_converter,
                                               IEthernetInterfaceFactory& eth_factory)
    : itf_info_ { itf_info },
      persistence_provider_ { persistence_provider },
      json_config_converter_ { json_config_converter },
      ethernet_interface_factory_ { eth_factory } {

  InterfaceConfigs peristet_configs;
  auto read_persistence_data_status = persistence_provider.Read(peristet_configs);
  if (read_persistence_data_status.NotOk()) {
    LogWarning("InterfaceConfigManager: Failed to read persistence data, use default data set");
  }

  auto interfaces = itf_info_.GetInterfaces();
  decltype(interfaces) considered_interfaces;
  copy_if(interfaces.begin(), interfaces.end(), back_inserter(considered_interfaces),
          [](auto itf_name) {return itf_name.rfind("ethX", 0) == 0;});

  InitializeEthernetInterfaceMap(considered_interfaces);

  InitializePortConfigs(considered_interfaces, peristet_configs);

  persistence_provider_.Write(current_config_);
}

void InterfaceConfigManager::InitializeEthernetInterfaceMap(const Interfaces& considered_interfaces) {
  auto create_ethernet_interface = [&](auto interface_name) {
    auto intf_short_name = interface_name.substr(3);  // remove eth prefix
      return ::std::make_pair(intf_short_name, ethernet_interface_factory_.getEthernetInterface(interface_name));
    };
  transform(considered_interfaces.begin(), considered_interfaces.end(),
            inserter(ethernet_interfaces_, ethernet_interfaces_.end()), create_ethernet_interface);
}

void InterfaceConfigManager::InitializePortConfigs(const Interfaces& considered_interfaces,
                                                   const InterfaceConfigs& persistet_configs) {

  auto persistet_entry_is_found_in_considered_interfaces =
      [&considered_interfaces](auto& entry) noexcept {
        return find_if(considered_interfaces.begin(),
            considered_interfaces.end() ,
            [&entry](const auto& linux_itf_name) {return linux_itf_name.substr(3) == entry.device_name_;}) != considered_interfaces.end();
      };
  copy_if(persistet_configs.begin(), persistet_configs.end(), back_inserter(current_config_),
          persistet_entry_is_found_in_considered_interfaces);

  auto is_missing_in_current_config =
      [&curr_config = current_config_](const auto& linux_itf_name) noexcept {
        return find_if(curr_config.begin(),
            curr_config.end() ,
            [&linux_itf_name](const auto& entry) {return linux_itf_name.substr(3) == entry.device_name_;}) == curr_config.end();
      };
  Interfaces missing_interfaces;
  copy_if(considered_interfaces.begin(), considered_interfaces.end(), back_inserter(missing_interfaces),
          is_missing_in_current_config);

  if (not missing_interfaces.empty()) {
    transform(missing_interfaces.begin(), missing_interfaces.end(), back_inserter(current_config_),
              generate_default_port_config);
  }

  auto applyAllResult = ApplyAllConfigs(current_config_);
  if (applyAllResult.NotOk()) {
    LogError("InterfaceConfigManager: Failed to apply initial config! " + applyAllResult.GetMessage());
  }

}

Status InterfaceConfigManager::IsPortConfigValid(const InterfaceConfigs& port_configs) {
  auto device_does_not_exist = [this](const auto& port_config) {
    return ethernet_interfaces_.count(port_config.device_name_) == 0;
  };

  auto none_existing_entry = find_if(port_configs.begin(), port_configs.end(), device_does_not_exist);
  if (none_existing_entry != port_configs.end()) {
    auto error_message = (::boost::format("Unknown device '%s'") % none_existing_entry->device_name_).str();
    return Status { StatusCode::INVALID_CONFIG, error_message };
  }

  auto data_is_insufficient = [](const InterfaceConfig& port_config) {
    return not ((port_config.state_ != InterfaceState::UNKNOWN) ||
        (port_config.autoneg_ != Autonegotiation::UNKNOWN) ||
        (port_config.speed_ > 0 && port_config.duplex_ != Duplex::UNKNOWN));
  };

  auto insufficient_entry = find_if(port_configs.begin(), port_configs.end(), data_is_insufficient);
  if (insufficient_entry != port_configs.end()) {
    auto error_message = (::boost::format("Port data for device '%s' insufficient") % insufficient_entry->device_name_)
        .str();
    return Status { StatusCode::INVALID_CONFIG, error_message };
  }

  return Status { };
}

Status InterfaceConfigManager::ApplyPortConfig(InterfaceConfig const& cfg) {

  auto& eif = ethernet_interfaces_.at(cfg.device_name_);
  if (cfg.autoneg_ != Autonegotiation::UNKNOWN) {
    eif->SetAutoneg((cfg.autoneg_ == Autonegotiation::ON) ? eth::Autoneg::On : eth::Autoneg::Off);
  }
  if (cfg.state_ != InterfaceState::UNKNOWN) {
    eif->SetState((cfg.state_ == InterfaceState::UP) ? eth::DeviceState::Up : eth::DeviceState::Down);
  }
  if (cfg.duplex_ != Duplex::UNKNOWN) {
    eif->SetDuplex((cfg.duplex_ == Duplex::FULL) ? eth::Duplex::Full : eth::Duplex::Half);
  }
  if (cfg.speed_ > 0) {
    eif->SetSpeed(cfg.speed_);
  }

  try {
    eif->Commit();
  } catch (std::exception& e) {
    auto error_message = (::boost::format("InterfaceConfigManager: Failed to apply port config for %s: %s")
        % cfg.device_name_ % e.what()).str();
    return Status { StatusCode::ERROR, error_message };
  }

  return Status { StatusCode::OK };

}

Status InterfaceConfigManager::ApplyAllConfigs(InterfaceConfigs& port_configs) {
  ::std::vector<Status> applyResults;
  std::transform(port_configs.begin(), port_configs.end(), ::std::back_inserter(applyResults),
                 [this] (auto& port_config) {return this->ApplyPortConfig(port_config);});

  auto first_negative_status = std::find_if(applyResults.begin(), applyResults.end(),
                                            [](auto& status) {return status.NotOk();});

  if (first_negative_status != applyResults.end()) {
    return *first_negative_status;
  }

  return Status { StatusCode::OK };
}

Status InterfaceConfigManager::Configure(InterfaceConfigs& port_configs) {
  auto status = IsPortConfigValid(port_configs);
  if (status.NotOk()) {
    return status;
  }

  status = ApplyAllConfigs(port_configs);

  if (status.NotOk()) {
    //persistence_provider_.Read();
    InterfaceConfigs old_port_configs;
    ApplyAllConfigs(old_port_configs);
  } else {
    UpdateCurrentConfig(port_configs);
    persistence_provider_.Write(current_config_);
  }

  return status;
}

void InterfaceConfigManager::UpdateCurrentConfig(const InterfaceConfigs& port_configs) {
  for (auto& config : port_configs) {
    const auto device_name_matches = [&](auto& c) {
      return c.device_name_ == config.device_name_;
    };
    ::std::replace_if(current_config_.begin(), current_config_.end(), device_name_matches, config);
  }
}

[[gnu::const]]
const InterfaceConfigs& InterfaceConfigManager::GetPortConfigs() {
  return current_config_;
}

}

/* namespace netconfd */

