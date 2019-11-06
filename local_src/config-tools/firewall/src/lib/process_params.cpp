//------------------------------------------------------------------------------
// Copyright (c) WAGO Kontakttechnik GmbH & Co. KG
//
// PROPRIETARY RIGHTS are involved in the subject matter of this material. All
// manufacturing, reproduction, use and sales rights pertaining to this
// subject matter are governed by the license agreement. The recipient of this
// software implicitly accepts the terms of the license.
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file     process_params.cpp
///
///  \brief    Function processing params configuration file, defining network device name mappings, e.g. ethX1 <-> br0.
///
///  \author   WAGO Kontakttechnik GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "process_params.hpp"

#include "process.hpp"
#include "file_access.hpp"
#include "error.hpp"

#include <iostream>
#include <fstream>
#include <sys/stat.h>


//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

namespace wago
{

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// macros
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

void update_network_interface_name_mapping()
{
    /* Update params_gen.xml file based on params.xml file.
     * - for each interface name in forward section of ipcmn.xml
     *    - check if interface is part of params_gen.xml
     *    - if not, add interface to params_gen.xml
     */

    xmldoc ipcmn;
    xmlctx ctx_ipcmn;

    xmldoc params(read_configuration("params", false));

    if (!params.is_empty())
    {
        try
        {
          ipcmn = read_configuration("iptables", false);
        }
        catch (std::runtime_error e)
        {
          // Just copy params.xml to params_gen.xml
          std::ofstream dst(get_config_fname("params_gen"), std::ios::binary);
          std::ifstream src(get_config_fname("params"), std::ios::binary);
          dst << src.rdbuf();
        }
    }

    if (!ipcmn.is_empty())
    {
        ctx_ipcmn = get_ctx(ipcmn);
    }

    // Iterate over interfaces of forward section
    if(!ctx_ipcmn.is_empty())
    {
        // Get list of interfaces used in masquerading and port-forwarding rules.
        std::vector<std::string> required_itf_list;
        get_attribute_value_list(ctx_ipcmn, "/f:firewall/f:ipv4/f:forward", "if", required_itf_list);

        xmlctx ctx_params(get_ctx(params));

        for(std::vector<std::string>::iterator it = required_itf_list.begin(); it != required_itf_list.end(); ++it)
        {
            // Check if interface name is part of params_gen.xml already.
            std::string const xpath_available_itfs("/f:firewall/f:parameters/f:interfaces/f:interface[@name=\"" + *it + "\"]");
            int const count = get_node_count(ctx_params, xpath_available_itfs);
            if(0 == count)
            {
                // Get parent of new interface node.
                xmlnode interfaces(get_node(ctx_params, "/f:firewall/f:parameters/f:interfaces", false));

                if (!interfaces.is_empty())
                {
                    // Create new interface node and add required attributes.
                    xmlnode interface_new(append_node(interfaces, "interface"));
                    append_attribute(interface_new, "name", *it);
                    append_attribute(interface_new, "rname", *it);
                    append_attribute(interface_new, "ethernet", "yes");
                }
            }
        }

        // Save xml changes to file.
        store_configuration("params_gen", false, params);
    }
}

}

//---- End of source file ------------------------------------------------------

