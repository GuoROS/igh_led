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
static unsigned int in_offs[16];
static unsigned int in_bit_pos[16];
static unsigned int out_offs[16];
static unsigned int out_bit_pos[16];

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
    if (!master) {
        fprintf(stderr, "Failed to get master!\n");
        return 1;
    }
    
    domain1 = ecrt_master_create_domain(master);
    if (!domain1) {
        fprintf(stderr, "Failed to create domain!\n");
        return 1;
    }
    
    // =============== 配置EL1809输入从站 ===============
    ec_slave_config_t *sc_el1809 = ecrt_master_slave_config(
        master, 0, 1, EL1809_VENDOR_ID, EL1809_PRODUCT_CODE);
    if (!sc_el1809) {
        fprintf(stderr, "Failed to configure EL1809!\n");
        return 1;
    }
    
    // EL1809 PDO条目配置 (16个输入通道)
    ec_pdo_entry_info_t slave_1_pdo_entries[] = {
        {0x6000, 0x01, 1}, /* Channel 1 */
        {0x6010, 0x01, 1}, /* Channel 2 */
        {0x6020, 0x01, 1}, /* Channel 3 */
        {0x6030, 0x01, 1}, /* Channel 4 */
        {0x6040, 0x01, 1}, /* Channel 5 */
        {0x6050, 0x01, 1}, /* Channel 6 */
        {0x6060, 0x01, 1}, /* Channel 7 */
        {0x6070, 0x01, 1}, /* Channel 8 */
        {0x6080, 0x01, 1}, /* Channel 9 */
        {0x6090, 0x01, 1}, /* Channel 10 */
        {0x60a0, 0x01, 1}, /* Channel 11 */
        {0x60b0, 0x01, 1}, /* Channel 12 */
        {0x60c0, 0x01, 1}, /* Channel 13 */
        {0x60d0, 0x01, 1}, /* Channel 14 */
        {0x60e0, 0x01, 1}, /* Channel 15 */
        {0x60f0, 0x01, 1}  /* Channel 16 */
    };
    
    // EL1809 PDO配置 (每个通道一个PDO)
    ec_pdo_info_t slave_1_pdos[] = {
        {0x1a00, 1, slave_1_pdo_entries + 0},  /* Channel 1 */
        {0x1a01, 1, slave_1_pdo_entries + 1},  /* Channel 2 */
        {0x1a02, 1, slave_1_pdo_entries + 2},  /* Channel 3 */
        {0x1a03, 1, slave_1_pdo_entries + 3},  /* Channel 4 */
        {0x1a04, 1, slave_1_pdo_entries + 4},  /* Channel 5 */
        {0x1a05, 1, slave_1_pdo_entries + 5},  /* Channel 6 */
        {0x1a06, 1, slave_1_pdo_entries + 6},  /* Channel 7 */
        {0x1a07, 1, slave_1_pdo_entries + 7},  /* Channel 8 */
        {0x1a08, 1, slave_1_pdo_entries + 8},  /* Channel 9 */
        {0x1a09, 1, slave_1_pdo_entries + 9},  /* Channel 10 */
        {0x1a0a, 1, slave_1_pdo_entries + 10}, /* Channel 11 */
        {0x1a0b, 1, slave_1_pdo_entries + 11}, /* Channel 12 */
        {0x1a0c, 1, slave_1_pdo_entries + 12}, /* Channel 13 */
        {0x1a0d, 1, slave_1_pdo_entries + 13}, /* Channel 14 */
        {0x1a0e, 1, slave_1_pdo_entries + 14}, /* Channel 15 */
        {0x1a0f, 1, slave_1_pdo_entries + 15}  /* Channel 16 */
    };
    
    // EL1809同步管理器配置
    ec_sync_info_t slave_1_syncs[] = {
        {0, EC_DIR_INPUT, 16, slave_1_pdos, EC_WD_DISABLE},
        {0xff} /* 结束标记 */
    };
    
    if (ecrt_slave_config_pdos(sc_el1809, EC_END, slave_1_syncs)) {
        fprintf(stderr, "Failed to configure EL1809 PDOs!\n");
        return 1;
    }
    
    // =============== 配置EL2809输出从站 ===============
    ec_slave_config_t *sc_el2809 = ecrt_master_slave_config(
        master, 0, 2, EL2809_VENDOR_ID, EL2809_PRODUCT_CODE);
    if (!sc_el2809) {
        fprintf(stderr, "Failed to configure EL2809!\n");
        return 1;
    }
    
    // EL2809 PDO条目配置 (16个输出通道)
    ec_pdo_entry_info_t slave_2_pdo_entries[] = {
        {0x7000, 0x01, 1}, /* Channel 1 */
        {0x7010, 0x01, 1}, /* Channel 2 */
        {0x7020, 0x01, 1}, /* Channel 3 */
        {0x7030, 0x01, 1}, /* Channel 4 */
        {0x7040, 0x01, 1}, /* Channel 5 */
        {0x7050, 0x01, 1}, /* Channel 6 */
        {0x7060, 0x01, 1}, /* Channel 7 */
        {0x7070, 0x01, 1}, /* Channel 8 */
        {0x7080, 0x01, 1}, /* Channel 9 */
        {0x7090, 0x01, 1}, /* Channel 10 */
        {0x70a0, 0x01, 1}, /* Channel 11 */
        {0x70b0, 0x01, 1}, /* Channel 12 */
        {0x70c0, 0x01, 1}, /* Channel 13 */
        {0x70d0, 0x01, 1}, /* Channel 14 */
        {0x70e0, 0x01, 1}, /* Channel 15 */
        {0x70f0, 0x01, 1}  /* Channel 16 */
    };
    
    // EL2809 PDO配置 (分为两个同步管理器)
    ec_pdo_info_t slave_2_pdos[] = {
        {0x1600, 1, slave_2_pdo_entries + 0},  /* Channel 1 */
        {0x1601, 1, slave_2_pdo_entries + 1},  /* Channel 2 */
        {0x1602, 1, slave_2_pdo_entries + 2},  /* Channel 3 */
        {0x1603, 1, slave_2_pdo_entries + 3},  /* Channel 4 */
        {0x1604, 1, slave_2_pdo_entries + 4},  /* Channel 5 */
        {0x1605, 1, slave_2_pdo_entries + 5},  /* Channel 6 */
        {0x1606, 1, slave_2_pdo_entries + 6},  /* Channel 7 */
        {0x1607, 1, slave_2_pdo_entries + 7},  /* Channel 8 */
        {0x1608, 1, slave_2_pdo_entries + 8},  /* Channel 9 */
        {0x1609, 1, slave_2_pdo_entries + 9},  /* Channel 10 */
        {0x160a, 1, slave_2_pdo_entries + 10}, /* Channel 11 */
        {0x160b, 1, slave_2_pdo_entries + 11}, /* Channel 12 */
        {0x160c, 1, slave_2_pdo_entries + 12}, /* Channel 13 */
        {0x160d, 1, slave_2_pdo_entries + 13}, /* Channel 14 */
        {0x160e, 1, slave_2_pdo_entries + 14}, /* Channel 15 */
        {0x160f, 1, slave_2_pdo_entries + 15}  /* Channel 16 */
    };
    
    // EL2809同步管理器配置
    ec_sync_info_t slave_2_syncs[] = {
        {0, EC_DIR_OUTPUT, 8, slave_2_pdos + 0, EC_WD_ENABLE},  // 前8个通道
        {1, EC_DIR_OUTPUT, 8, slave_2_pdos + 8, EC_WD_ENABLE}, // 后8个通道
        {0xff} /* 结束标记 */
    };
    
    if (ecrt_slave_config_pdos(sc_el2809, EC_END, slave_2_syncs)) {
        fprintf(stderr, "Failed to configure EL2809 PDOs!\n");
        return 1;
    }
    
    // =============== 注册PDO条目 ===============
    ec_pdo_entry_reg_t regs[33] = {0}; // 32个通道 + 结束标记
    
    // 注册EL1809输入通道
    for (int i = 0; i < 16; i++) {
        regs[i] = (ec_pdo_entry_reg_t){
            .alias = 0,
            .position = 1,
            .vendor_id = EL1809_VENDOR_ID,
            .product_code = EL1809_PRODUCT_CODE,
            .index = slave_1_pdo_entries[i].index,
            .subindex = slave_1_pdo_entries[i].subindex,
            .offset = &in_offs[i],
            .bit_position = &in_bit_pos[i]
        };
    }
    
    // 注册EL2809输出通道
    for (int i = 0; i < 16; i++) {
        regs[16 + i] = (ec_pdo_entry_reg_t){
            .alias = 0,
            .position = 2,
            .vendor_id = EL2809_VENDOR_ID,
            .product_code = EL2809_PRODUCT_CODE,
            .index = slave_2_pdo_entries[i].index,
            .subindex = slave_2_pdo_entries[i].subindex,
            .offset = &out_offs[i],
            .bit_position = &out_bit_pos[i]
        };
    }
    
    // 结束标记
    regs[32].index = 0;
    
    if (ecrt_domain_reg_pdo_entry_list(domain1, regs)) {
        fprintf(stderr, "Failed to register PDO entries!\n");
        return 1;
    }
    
    // =============== 激活主站 ===============
    if (ecrt_master_activate(master)) {
        fprintf(stderr, "Failed to activate master!\n");
        return 1;
    }
    
    domain1_pd = ecrt_domain_data(domain1);
    if (!domain1_pd) {
        fprintf(stderr, "Failed to get domain data pointer!\n");
        return 1;
    }
    
    // =============== 主循环 ===============
    int cycle = 0;
    int light_pos = 0; // 当前点亮的LED位置
    const int CYCLE_FREQ = 100; // 100Hz循环频率
    const int LED_UPDATE_HZ = 10; // 10Hz LED更新频率
    const int LED_UPDATE_INTERVAL = CYCLE_FREQ / LED_UPDATE_HZ;
    
    printf("EtherCAT running. Press Ctrl-C to exit.\n");
    
    while (run) {
        // EtherCAT通信处理
        ecrt_master_receive(master);
        ecrt_domain_process(domain1);
        
        // 更新LED位置
        if (cycle % LED_UPDATE_INTERVAL == 0) {
            light_pos = (light_pos + 1) % 16;
        }
        
        // 设置输出 - 只点亮当前LED
        for (int i = 0; i < 16; i++) {
            EC_WRITE_BIT(domain1_pd + out_offs[i], out_bit_pos[i], (i == light_pos));
        }
        
        // 发送过程数据
        ecrt_domain_queue(domain1);
        ecrt_master_send(master);
        
        // 控制循环频率
        usleep(1000000 / CYCLE_FREQ);
        cycle++;
    }
    
    // =============== 清理 ===============
    // 关闭所有输出
    for (int i = 0; i < 16; i++) {
        EC_WRITE_BIT(domain1_pd + out_offs[i], out_bit_pos[i], 0);
    }
    
    ecrt_domain_queue(domain1);
    ecrt_master_send(master);
    ecrt_release_master(master);
    
    printf("EtherCAT master released. All outputs turned off.\n");
    return 0;
}