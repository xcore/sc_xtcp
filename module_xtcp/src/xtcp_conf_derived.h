#ifndef __xtcp_conf_derived_h__
#define __xtcp_conf_derived_h__

#ifdef __xtcp_conf_h_exists__
#include "xtcp_conf.h"
#endif

#ifdef __xtcp_client_conf_h_exists__
#include "xtcp_client_conf.h"
#endif

#ifndef XTCP_SEPARATE_MAC
#define XTCP_SEPARATE_MAC 0
#endif

#if XTCP_SEPARATE_MAC
#define ETHERNET_DEFAULT_IMPLEMENTATION full
#endif

#ifndef ETHERNET_USE_XTCP_FILTER
#define ETHERNET_USE_XTCP_FILTER 1
#endif

#if ETHERNET_USE_XTCP_FILTER
#define ETHERNET_CUSTOM_FILTER_HEADER "xtcp_mac_filter.h"
#endif

#ifndef XTCP_ENABLE_PUSH_FLAG_NOTIFICATION
#define XTCP_ENABLE_PUSH_FLAG_NOTIFICATION 0
#endif

#endif // __xtcp_conf_derived_h__
