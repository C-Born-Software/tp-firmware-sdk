//------------------------------------------------------------------------------
// Copyright (c) WAGO Kontakttechnik GmbH & Co. KG
//
// PROPRIETARY RIGHTS are involved in the subject matter of this material. All
// manufacturing, reproduction, use and sales rights pertaining to this
// subject matter are governed by the license agreement. The recipient of this
// software implicitly accepts the terms of the license.
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file     process_params.hpp
///
///  \brief    Function processing params configuration file, defining network device name mappings, e.g. ethX1 <-> br0.
///
///  \author   WAGO Kontakttechnik GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef PROCESS_PARAMS_HPP_
#define PROCESS_PARAMS_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#include "xmlhlp.hpp"

namespace wago
{

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Update params_gen.xml file, defining network device name mappings, e.g. ethX1 <-> br0.
///
/// The params_gen.xml file bases on the params.xml file. In addition, the ipcmn.xml file
/// is checked for ip masquerading and port forwarding rules concerning network interfaces
/// which are not part of the params_gen.xml file yet. If such network interfaces are found,
/// they are added to params_gen.xml file.
///
//------------------------------------------------------------------------------
void update_network_interface_name_mapping();

//------------------------------------------------------------------------------
// macros
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

}

#endif /* PROCESS_PARAMS_HPP_ */
//---- End of source file ------------------------------------------------------

