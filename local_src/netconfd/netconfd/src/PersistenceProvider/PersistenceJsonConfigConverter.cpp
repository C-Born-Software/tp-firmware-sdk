// SPDX-License-Identifier: GPL-2.0-or-later


#include <PersistenceJsonConfigConverter.hpp>
#include "Logger.hpp"
#include <nlohmann/json.hpp>
#include <iostream>

namespace netconfd {

using nlohmann::json;

Status PersistenceJsonConfigConverter::ToJson(const BridgeConfig& bridge_config,
                                              const IPConfigs& ip_configs, bool pretty,
                                              ::std::string& json_out) const {

  json_out.clear();

  json bridge_config_json = BridgeConfigToNJson(bridge_config);

  // explicit empty object
  json ip_configs_json = IPConfigsToNJson(ip_configs);

  // Concatenate/Merge the different config ptrees
  json whole_config;
  whole_config["bridge-config"] = bridge_config_json;
  whole_config["ip-config"] = ip_configs_json;

  json_out = whole_config.dump(pretty ? 2 : -1);

  return Status{};
}

Status PersistenceJsonConfigConverter::ToConfigs(const ::std::string& json_str,
                                                 BridgeConfig& bridge_config,
                                                 IPConfigs& ip_configs) const {
  json config_json;
  Status status = JsonToNJson(json_str, config_json);

  json bridge_config_json;
  json ip_configs_json;

  if (status.Ok()) {
    try {

      bridge_config_json = config_json.at("bridge-config");
      ip_configs_json = config_json.at("ip-config");
    } catch (std::exception const& e) {
      status.Prepend(StatusCode::JSON_CONVERT_ERROR, "Convert json to configs error " + std::string(e.what()));
    } catch (...) {
      status.Prepend(StatusCode::JSON_CONVERT_ERROR, "Convert json to configs error, unexpected exception");
    }
  }

  if (status.Ok()) {
    status = NJsonToBridgeConfig(bridge_config_json, bridge_config);
  }

  if (status.Ok()) {
    status = NJsonToIPConfigs(ip_configs_json, ip_configs);
  }

  return status;

}


::std::string PersistenceJsonConfigConverter::ToJsonString(const InterfaceConfigs& obj) {
  return InterfaceConfigToJson(obj);
}

Status PersistenceJsonConfigConverter::FromJsonString(const ::std::string& str, InterfaceConfigs& out_obj) {
  return InterfaceConfigFromJson(str, out_obj);

}

} /* namespace netconfd */
