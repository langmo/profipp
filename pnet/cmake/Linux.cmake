#********************************************************************
#        _       _         _
#  _ __ | |_  _ | |  __ _ | |__   ___
# | '__|| __|(_)| | / _` || '_ \ / __|
# | |   | |_  _ | || (_| || |_) |\__ \
# |_|    \__|(_)|_| \__,_||_.__/ |___/
#
# www.rt-labs.com
# Copyright 2018 rt-labs AB, Sweden.
#
# This software is dual-licensed under GPLv3 and a commercial
# license. See the file LICENSE.md distributed with this software for
# full license information.
#*******************************************************************/

if (PNET_OPTION_SNMP)
  find_package(NetSNMP REQUIRED)
  find_package(NetSNMPAgent REQUIRED)
endif()

target_include_directories(pnet
  PRIVATE
  src/ports/linux
  )

target_sources(pnet
  PRIVATE
  src/ports/linux/pnal.c
  src/ports/linux/pnal_eth.c
  src/ports/linux/pnal_udp.c
  src/ports/linux/pnal_filetools.c
  $<$<BOOL:${PNET_OPTION_SNMP}>:src/ports/linux/pnal_snmp.c>
  $<$<BOOL:${PNET_OPTION_SNMP}>:src/ports/linux/mib/system_mib.c>
  $<$<BOOL:${PNET_OPTION_SNMP}>:src/ports/linux/mib/lldpLocalSystemData.c>
  $<$<BOOL:${PNET_OPTION_SNMP}>:src/ports/linux/mib/lldpLocPortTable.c>
  $<$<BOOL:${PNET_OPTION_SNMP}>:src/ports/linux/mib/lldpConfigManAddrTable.c>
  $<$<BOOL:${PNET_OPTION_SNMP}>:src/ports/linux/mib/lldpLocManAddrTable.c>
  $<$<BOOL:${PNET_OPTION_SNMP}>:src/ports/linux/mib/lldpRemTable.c>
  $<$<BOOL:${PNET_OPTION_SNMP}>:src/ports/linux/mib/lldpRemManAddrTable.c>
  $<$<BOOL:${PNET_OPTION_SNMP}>:src/ports/linux/mib/lldpXdot3LocPortTable.c>
  $<$<BOOL:${PNET_OPTION_SNMP}>:src/ports/linux/mib/lldpXdot3RemPortTable.c>
  $<$<BOOL:${PNET_OPTION_SNMP}>:src/ports/linux/mib/lldpXPnoLocTable.c>
  $<$<BOOL:${PNET_OPTION_SNMP}>:src/ports/linux/mib/lldpXPnoRemTable.c>
  )

target_compile_options(pnet
  PRIVATE
  -Wall
  -Wextra
  -Werror
  -Wno-unused-parameter
  -ffunction-sections
  -fdata-sections
  INTERFACE
  $<$<CONFIG:Coverage>:--coverage>
  )

target_link_libraries(pnet
  PUBLIC
  $<$<BOOL:${PNET_OPTION_SNMP}>:NetSNMP::NetSNMPAgent>
  $<$<BOOL:${PNET_OPTION_SNMP}>:NetSNMP::NetSNMP>
  INTERFACE
  $<$<CONFIG:Coverage>:--coverage>
  )

install (FILES
  include/pnal_config.h
  DESTINATION include
  )
