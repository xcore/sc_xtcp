
void set_ip_config(chanend config_svr, unsigned int val);
void set_ip_addr(chanend config_svr, unsigned int val);
unsigned int get_ip_config(chanend config_svr);
unsigned int get_ip_addr(chanend config_svr);

void set_default_ip_config(chanend config_svr, unsigned int val);
void set_default_ip_addr(chanend config_svr, unsigned int val);
unsigned int get_default_ip_config(chanend config_svr);
unsigned int get_default_ip_addr(chanend config_svr);
void set_default_netmask(chanend config_svr, unsigned int val);
unsigned int get_default_netmask(chanend config_svr);
void enable_ipconfig_from_ipserver(chanend config_svr);

void commit_config(chanend config_svr);
void xc2_firmware_config(chanend config_ch[],
                         int num_config,
                         chanend xtcp_svr,
                         chanend led_svr);
