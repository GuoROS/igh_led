#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <ecrt.h>

// 从站配置
#define EL1809_VENDOR_ID    0x00000002
#define EL1809_PRODUCT_CODE 0x07113052
#define EL2809_VENDOR_ID    0x00000002
#define EL2809_PRODUCT_CODE 0x0af93052

// 过程数据指针
static uint8_t *domain1_pd = NULL;

// 通道偏移量和位位置
static unsigned int in_offs[16], out_offs[16];
static unsigned int in_bit_pos[16], out_bit_pos[16];

// EtherCAT对象
static ec_master_t *master = NULL;
static ec_domain_t *domain1 = NULL;
static volatile sig_atomic_t run = 1;

// 信号处理
void signal_handler(int sig) { run = 0; }

int main() {
    signal(SIGINT, signal_handler);
    
    // 初始化EtherCAT
    master = ecrt_request_master(0);
    domain1 = ecrt_master_create_domain(master);
    
    // 配置EL1809
    ec_slave_config_t *sc_el1809 = ecrt_master_slave_config(
        master, 0, 1, EL1809_VENDOR_ID, EL1809_PRODUCT_CODE);
    
    // EL1809 PDO配置
    ec_pdo_entry_info_t slave1_entries[16];
    for (int i = 0; i < 16; i++) 
        slave1_entries[i] = (ec_pdo_entry_info_t){0x6000 + i*0x10, 0x01, 1};
    
    ec_pdo_info_t slave1_pdos[16];
    for (int i = 0; i < 16; i++) 
        slave1_pdos[i] = (ec_pdo_info_t){0x1a00 + i, 1, &slave1_entries[i]};
    
    ec_sync_info_t slave1_syncs[] = {
        {0, EC_DIR_INPUT, 16, slave1_pdos, EC_WD_DISABLE},
        {0xff}
    };
    ecrt_slave_config_pdos(sc_el1809, EC_END, slave1_syncs);

    // 配置EL2809
    ec_slave_config_t *sc_el2809 = ecrt_master_slave_config(
        master, 0, 2, EL2809_VENDOR_ID, EL2809_PRODUCT_CODE);
    
    // EL2809 PDO配置
    ec_pdo_entry_info_t slave2_entries[16];
    for (int i = 0; i < 16; i++) 
        slave2_entries[i] = (ec_pdo_entry_info_t){0x7000 + i*0x10, 0x01, 1};
    
    ec_pdo_info_t slave2_pdos[16];
    for (int i = 0; i < 8; i++) 
        slave2_pdos[i] = (ec_pdo_info_t){0x1600 + i, 1, &slave2_entries[i]};
    for (int i = 8; i < 16; i++) 
        slave2_pdos[i] = (ec_pdo_info_t){0x1600 + i, 1, &slave2_entries[i]};
    
    ec_sync_info_t slave2_syncs[] = {
        {0, EC_DIR_OUTPUT, 8, slave2_pdos, EC_WD_ENABLE},
        {1, EC_DIR_OUTPUT, 8, slave2_pdos + 8, EC_WD_ENABLE},
        {0xff}
    };
    ecrt_slave_config_pdos(sc_el2809, EC_END, slave2_syncs);

    // 注册PDO条目
    ec_pdo_entry_reg_t regs[33] = {0}; // 32通道 + 结束标记
    for (int i = 0; i < 16; i++) {
        regs[i] = (ec_pdo_entry_reg_t){0, 1, EL1809_VENDOR_ID, EL1809_PRODUCT_CODE, 
                                      0x6000 + i*0x10, 0x01, &in_offs[i], &in_bit_pos[i]};
        regs[16+i] = (ec_pdo_entry_reg_t){0, 2, EL2809_VENDOR_ID, EL2809_PRODUCT_CODE, 
                                       0x7000 + i*0x10, 0x01, &out_offs[i], &out_bit_pos[i]};
    }
    ecrt_domain_reg_pdo_entry_list(domain1, regs);

    // 激活主站
    ecrt_master_activate(master);
    domain1_pd = ecrt_domain_data(domain1);

    // 主循环
    int cycle = 0, light_pos = 0;
    const int CYCLE_HZ = 100;          // 100Hz循环
    const int LED_UPDATE_INTERVAL = 10; // 10Hz LED更新
    
    while (run) {
        ecrt_master_receive(master);
        ecrt_domain_process(domain1);

        // 更新LED位置
        if (cycle % (CYCLE_HZ / LED_UPDATE_INTERVAL) == 0) 
            light_pos = (light_pos + 1) % 16;

        // 设置输出
        for (int i = 0; i < 16; i++) 
            EC_WRITE_BIT(domain1_pd + out_offs[i], out_bit_pos[i], (i == light_pos));

        ecrt_domain_queue(domain1);
        ecrt_master_send(master);
        usleep(1000000 / CYCLE_HZ);
        cycle++;
    }

    // 清理
    for (int i = 0; i < 16; i++) 
        EC_WRITE_BIT(domain1_pd + out_offs[i], out_bit_pos[i], 0);
    
    ecrt_domain_queue(domain1);
    ecrt_master_send(master);
    ecrt_release_master(master);
    return 0;
}