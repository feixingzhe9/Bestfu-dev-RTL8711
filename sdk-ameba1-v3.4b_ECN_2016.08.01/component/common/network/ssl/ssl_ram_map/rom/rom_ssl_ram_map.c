#include <rom_ssl_ram_map.h>
#include <section_config.h>

#ifndef SSL_RAM_MAP_SECTION
#define SSL_RAM_MAP_SECTION		//SECTION(".ssl_ram_map") //will define in section_config.h
#endif

/* RAM table referred by SSL ROM */
SSL_RAM_MAP_SECTION
struct _rom_ssl_ram_map rom_ssl_ram_map;
