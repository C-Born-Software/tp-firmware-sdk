#include "Status.hpp"

#include <map>

namespace wago::snmp_config_lib {

namespace {

std::string GetTextForCode(StatusCode c) {
  // @formatter:off
  static const ::std::map<StatusCode, ::std::string> status_text = {
    { StatusCode::OK, ::std::string { } },
    { StatusCode::WRONG_PARAMETER_PATTERN, "" }
  };
  // @formatter:on
  return status_text.at(c);
}

}  //namespace

Status::Status(StatusCode code)
    : status_code_ { code } {
}

Status::Status(StatusCode code, ::std::string &&additional_information)
    : status_code_ { code },
      additional_information_ { additional_information } {
  ;
}

::std::string Status::ToString() const {
  return GetTextForCode(status_code_).append(additional_information_);
}

bool Status::IsOk() const {
  return status_code_ == StatusCode::OK;
}

void Statuses::AddErrorStatus(Status &&status) {
  if (not status.IsOk()) {
    statuses_.emplace_back(status);
  }
}

void Statuses::AddErrorStatuses(Statuses &&statuses) {
  statuses_.insert(statuses_.end(), statuses.statuses_.begin(), statuses.statuses_.end());
}

bool Statuses::IsOk() const{
  return statuses_.empty();
}

const ::std::vector<Status>& Statuses::GetStatuses() const{
  return statuses_;
}

}  // namespace wago::snmp_config_lib
