// SPDX-License-Identifier: GPL-2.0-or-later

#include "JsonConfigConverter.hpp"

#include <boost/algorithm/string.hpp>
#include <vector>
#include <map>

#include "TypeUtils.hpp"
#include "TypesHelper.hpp"
#include "Status.hpp"
#include "Types.hpp"
#include "Logger.hpp"
#include "Helper.hpp"

#include <nlohmann/json.hpp>

#include <iostream>
namespace netconfd {

using namespace ::std::literals;
using nlohmann::json;

constexpr auto PORT_CFG_KEY_DEVICE = "device";
constexpr auto PORT_CFG_KEY_STATE = "state";
constexpr auto PORT_CFG_KEY_AUTONEG = "autonegotiation";
constexpr auto PORT_CFG_KEY_SPEED = "speed";
constexpr auto PORT_CFG_KEY_DUPLEX = "duplex";

NLOHMANN_JSON_SERIALIZE_ENUM(Duplex, { {Duplex::HALF, "half"}, {Duplex::FULL, "full"}, {Duplex::UNKNOWN, nullptr} })

NLOHMANN_JSON_SERIALIZE_ENUM(Autonegotiation, { {Autonegotiation::OFF, false}, {Autonegotiation::ON, true},
                             {Autonegotiation::UNKNOWN, nullptr} })

NLOHMANN_JSON_SERIALIZE_ENUM(InterfaceState, { {InterfaceState::DOWN, "down"}, {InterfaceState::UP, "up"},
                             {InterfaceState::UNKNOWN, nullptr} })

NLOHMANN_JSON_SERIALIZE_ENUM(IPSource, { {IPSource::UNKNOWN, nullptr }, {IPSource::NONE, "none" }, {IPSource::STATIC,
                             "static" }, {IPSource::DHCP, "dhcp" }, {IPSource::BOOTP, "bootp" }, {IPSource::TEMPORARY,
                             "temporary"}, {IPSource::EXTERNAL, "external"}});

JsonConfigConverter::JsonConfigConverter()
    : interface_config_keys_ { PORT_CFG_KEY_DEVICE, PORT_CFG_KEY_STATE, PORT_CFG_KEY_AUTONEG, PORT_CFG_KEY_SPEED,
        PORT_CFG_KEY_DUPLEX } {
}

json JsonConfigConverter::IPConfigsToNJson(const IPConfigs& ip_configs) const {
  json json_out( { });
  for (auto ip_config : ip_configs) {
    json inner = json { { "ipaddr", ip_config.address_ }, { "netmask", ip_config.netmask_ }, { "bcast", ip_config
        .broadcast_ } };
    to_json(inner["source"], ip_config.source_);
    json_out[ip_config.interface_] = inner;
  }
  return json_out;
}

json JsonConfigConverter::BridgeConfigToNJson(const BridgeConfig& bridge_config) const {
  return json(bridge_config);
}

Status JsonConfigConverter::BridgeConfigToJson(const BridgeConfig& bridge_config, ::std::string& json_out) const {
  json bridge_config_json(bridge_config);
  json_out = bridge_config_json.dump();
  return Status { };
}

::std::string JsonConfigConverter::IPConfigsToJson(const IPConfigs& ip_configs) const {
  return IPConfigsToNJson(ip_configs).dump();
}

Status JsonConfigConverter::JsonToNJson(::std::string const& json_str, json& json_object) const {

  Status status;

  try {

    class throwing_sax : public nlohmann::detail::json_sax_dom_parser<nlohmann::json> {
     public:
      explicit throwing_sax(nlohmann::json& j)
          : nlohmann::detail::json_sax_dom_parser<nlohmann::json>(j) {
      }
      ;

      bool key(json::string_t& val) {
        if (ref_stack.back()->contains(val)) {
          throw std::invalid_argument("key " + val + " was already stored");
        }
        return nlohmann::detail::json_sax_dom_parser<nlohmann::json>::key(val);
      }
    };
    throwing_sax sax_consumer(json_object);
    json::sax_parse(json_str, &sax_consumer);

  } catch (std::exception const& e) {
    status.Prepend(StatusCode::JSON_CONVERT_ERROR, "Json parse error " + std::string(e.what()));
  } catch (...) {
    status.Prepend(StatusCode::JSON_CONVERT_ERROR, "Json parse error unexpected exception");
  }

  return status;

}

::std::string JsonConfigConverter::InterfacesToJson(const Interfaces& interfaces) const {

  ::std::string json_string = "["s;
  if (!interfaces.empty()) {

    for (uint32_t i = 0; i < interfaces.size() - 1; i++) {
      json_string += "\"" + interfaces[i] + "\",";
    }
    json_string += "\"" + interfaces[interfaces.size() - 1] + "\"";

  }

  json_string += "]";
  return json_string;
}

Status JsonConfigConverter::NJsonToBridgeConfig(const json& json_object, BridgeConfig& bridge_config) const {
  Status status;

  for (const auto& bridge_cfg_json : json_object.items()) {
    auto bridge_cfg_pair = std::make_pair<Bridge, Interfaces>(bridge_cfg_json.key(), bridge_cfg_json.value()); // NOLINT: Need to express types in make_pair to get it working.

    if (bridge_cfg_pair.first.empty()) {
      status.Append(StatusCode::JSON_CONVERT_ERROR, "Bridge Name is empty");
    } else if (bridge_config.count(bridge_cfg_pair.first) > 0) {
      status.Append(StatusCode::JSON_CONVERT_ERROR, "Bridge Name duplicate!");
    }

    if (status.NotOk()) {
      bridge_config.clear();
      break;
    }
    bridge_config.insert(bridge_cfg_pair);
  }

  return status;
}

Status JsonConfigConverter::NJsonToIPConfigs(const json& json_object, IPConfigs& ip_configs) const {

  Status status;

  try {
    auto get_to_if_exists = [](const ::std::string& key, const nlohmann::json& inner_json, auto& to) {
      auto iter = inner_json.find(key);
      if (iter != inner_json.end()) {
        iter.value().get_to(to);
      }
    };

    for (const auto& ip_config : json_object.items()) {
      IPConfig c;
      c.interface_ = ip_config.key();
      auto inner = ip_config.value();

      get_to_if_exists("ipaddr", inner, c.address_);
      get_to_if_exists("netmask", inner, c.netmask_);
      get_to_if_exists("bcast", inner, c.broadcast_);
      get_to_if_exists("source", inner, c.source_);

      ip_configs.push_back(::std::move(c));
    }

  } catch (std::exception const& e) {
    ip_configs.clear();
    status.Prepend(StatusCode::JSON_CONVERT_ERROR, "Convert json to Ip config error: " + std::string(e.what()));
  } catch (...) {
    ip_configs.clear();
    status.Prepend(StatusCode::JSON_CONVERT_ERROR, "Convert json to Ip config error unexpected exception");
  }

  return status;
}

Status JsonConfigConverter::NJsonToInterfaceConfigs(const nlohmann::json& jo,
                                                    InterfaceConfigs& interface_configs) const {
  if (jo.empty()) {
    return Status { StatusCode::JSON_CONVERT_ERROR, "Empty JSON object" };
  }
  if (jo.is_array()) {
    for (auto& jentry : jo) {
      if (!interface_config_keys_.matchesJsonObject(jentry)) {
        return Status { StatusCode::JSON_CONVERT_ERROR, "Invalid JSON object" };
      }
      interface_configs.push_back(PortConfigFromJson(jentry));
    }
  } else {
    if (!interface_config_keys_.matchesJsonObject(jo)) {
      return Status { StatusCode::JSON_CONVERT_ERROR, "Invalid JSON object" };
    }
    interface_configs.push_back(PortConfigFromJson(jo));
  }
  return Status { StatusCode::OK };
}

Status JsonConfigConverter::JsonToBridgeConfig(const ::std::string& json_str, BridgeConfig& bridge_config) const {
  Status status;
  try {
    json whole_bridge_json;
    status = JsonToNJson(json_str, whole_bridge_json);
    if (status.Ok()) {
      status = NJsonToBridgeConfig(whole_bridge_json, bridge_config);
    }
  } catch (std::exception& e) {
    bridge_config.clear();
    status.Append(StatusCode::JSON_CONVERT_ERROR, "Parse BridgeConfig failed! "s + e.what());
  }

  return status;
}

Status JsonConfigConverter::JsonToIPConfigs(const ::std::string& json_string, IPConfigs& ip_configs) const {
  Status status;
  try {
    nlohmann::json j = json::parse(json_string);
    status = NJsonToIPConfigs(j, ip_configs);
  } catch (std::exception& e) {
    ip_configs.clear();
    status.Append(StatusCode::JSON_CONVERT_ERROR, "Parse IpConfig failed! "s + e.what());

  }
  return status;
}

::std::string JsonConfigConverter::InterfaceConfigToJson(const InterfaceConfigs& port_configs) const {

  if (port_configs.empty()) {
    throw std::runtime_error("Interface config empty. This should not happen.");
  }

  return InterfaceConfigsToNJson(port_configs).dump();
}

json JsonConfigConverter::InterfaceConfigsToNJson(const InterfaceConfigs& interface_configs) const {

  if (interface_configs.size() > 1) {
    nlohmann::json jarray { };
    for (const auto& config : interface_configs) {
      jarray.push_back(PortConfigToNJson(config));
    }
    return jarray;
  }

  return interface_configs.size() == 1 ? PortConfigToNJson(interface_configs.at(0)) : nlohmann::json( { });
}

json JsonConfigConverter::PortConfigToNJson(const InterfaceConfig& interface_config) const {
  json j;

  j[PORT_CFG_KEY_DEVICE] = interface_config.device_name_;

  if (interface_config.state_ != InterfaceState::UNKNOWN) {
    j[PORT_CFG_KEY_STATE] = interface_config.state_;
  }

  if (interface_config.autoneg_ != Autonegotiation::UNKNOWN) {
    j[PORT_CFG_KEY_AUTONEG] = interface_config.autoneg_;
  }

  if (interface_config.speed_ > 0) {
    j[PORT_CFG_KEY_SPEED] = interface_config.speed_;
  }

  if (interface_config.duplex_ != Duplex::UNKNOWN) {
    j[PORT_CFG_KEY_DUPLEX] = interface_config.duplex_;
  }
  return j;
}

InterfaceConfig JsonConfigConverter::PortConfigFromJson(const json& config) const {

  InterfaceConfig p { config.at(PORT_CFG_KEY_DEVICE).get<::std::string>() };

  auto has_angeg = config.find(PORT_CFG_KEY_AUTONEG) != config.end();
  if (has_angeg) {
    p.autoneg_ = config.at(PORT_CFG_KEY_AUTONEG).get<Autonegotiation>();
  }

  auto duplex_opt = config.find(PORT_CFG_KEY_DUPLEX) != config.end();
  if (duplex_opt) {
    p.duplex_ = config.at(PORT_CFG_KEY_DUPLEX).get<Duplex>();
  }

  auto state_opt = config.find(PORT_CFG_KEY_STATE) != config.end();
  if (state_opt) {
    p.state_ = config.at(PORT_CFG_KEY_STATE).get<InterfaceState>();
  }

  auto speed_opt = config.find(PORT_CFG_KEY_SPEED) != config.end();
  if (speed_opt) {
    p.speed_ = config.at(PORT_CFG_KEY_SPEED).get<int>();
  }
  return p;
}

Status JsonConfigConverter::InterfaceConfigFromJson(const ::std::string& json_string,
                                                    InterfaceConfigs& interface_configs) const {
  Status status;
  try {

    json jo;
    status = JsonToNJson(json_string, jo);
    if (status.NotOk()) {
      return status;
    }

    status = NJsonToInterfaceConfigs(jo, interface_configs);

  } catch (std::exception& e) {
    return {StatusCode::JSON_CONVERT_ERROR, e.what()};
  }
  return status;
}

}  // namespace netconfd
