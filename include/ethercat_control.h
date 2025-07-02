#ifndef IGH_LED_H
#define IGH_LED_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <ecrt.h>

// 定义从站配置
#define EL1809_VENDOR_ID    0x00000002
#define EL1809_PRODUCT_CODE 0x07113052
#define EL2809_VENDOR_ID    0x00000002
#define EL2809_PRODUCT_CODE 0x0af93052

// 过程数据域指针
extern uint8_t *domain1_pd;

// EL1809输入通道偏移量和位位置数组
extern unsigned int in_offs[16];
extern unsigned int in_bit_pos[16];

// EL2809输出通道偏移量和位位置数组
extern unsigned int out_offs[16];
extern unsigned int out_bit_pos[16];

// EL1809同步管理器配置
extern ec_pdo_entry_info_t slave_1_pdo_entries[];
extern ec_pdo_info_t slave_1_pdos[];
extern ec_sync_info_t slave_1_syncs[];

// EL2809同步管理器配置
extern ec_pdo_entry_info_t slave_2_pdo_entries[];
extern ec_pdo_info_t slave_2_pdos[];
extern ec_sync_info_t slave_2_syncs[];

// EtherCAT主站句柄
extern ec_master_t *master;
extern ec_master_state_t master_state;

// 域和从站配置
extern ec_domain_t *domain1;
extern ec_domain_state_t domain_state;
extern ec_slave_config_t *sc_el1809;
extern ec_slave_config_t *sc_el2809;
extern ec_slave_config_state_t sc_el1809_state;
extern ec_slave_config_state_t sc_el2809_state;

// 中断处理
extern volatile sig_atomic_t run;
void signal_handler(int sig);

// 检查从站状态
void check_domain_state();
void check_master_state();
void check_slave_config_states();

#endif // IGH_LED_H
