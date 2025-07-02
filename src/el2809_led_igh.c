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
static uint8_t *domain1_pd = NULL;

// EL1809输入通道偏移量和位位置数组
static unsigned int in_offs[16];
static unsigned int in_bit_pos[16];

// EL2809输出通道偏移量和位位置数组
static unsigned int out_offs[16];
static unsigned int out_bit_pos[16];

// EL1809同步管理器配置
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
    {0x60f0, 0x01, 1}, /* Channel 16 */
};

ec_pdo_info_t slave_1_pdos[] = {
    {0x1a00, 16, slave_1_pdo_entries}, /* 单个PDO包含所有16个通道 */
};

ec_sync_info_t slave_1_syncs[] = {
    {0, EC_DIR_INPUT, 1, slave_1_pdos, EC_WD_DISABLE}, /* 仅一个同步管理器 */
    {0xff} /* 结束标记 */
};

// EL2809同步管理器配置
ec_pdo_entry_info_t slave_2_pdo_entries[] = {
    {0x7000, 0x01, 1}, /* Output */
    {0x7010, 0x01, 1}, /* Output */
    {0x7020, 0x01, 1}, /* Output */
    {0x7030, 0x01, 1}, /* Output */
    {0x7040, 0x01, 1}, /* Output */
    {0x7050, 0x01, 1}, /* Output */
    {0x7060, 0x01, 1}, /* Output */
    {0x7070, 0x01, 1}, /* Output */
    {0x7080, 0x01, 1}, /* Output */
    {0x7090, 0x01, 1}, /* Output */
    {0x70a0, 0x01, 1}, /* Output */
    {0x70b0, 0x01, 1}, /* Output */
    {0x70c0, 0x01, 1}, /* Output */
    {0x70d0, 0x01, 1}, /* Output */
    {0x70e0, 0x01, 1}, /* Output */
    {0x70f0, 0x01, 1}, /* Output */
};

ec_pdo_info_t slave_2_pdos[] = {
    {0x1600, 1, slave_2_pdo_entries + 0}, /* Channel 1 */
    {0x1601, 1, slave_2_pdo_entries + 1}, /* Channel 2 */
    {0x1602, 1, slave_2_pdo_entries + 2}, /* Channel 3 */
    {0x1603, 1, slave_2_pdo_entries + 3}, /* Channel 4 */
    {0x1604, 1, slave_2_pdo_entries + 4}, /* Channel 5 */
    {0x1605, 1, slave_2_pdo_entries + 5}, /* Channel 6 */
    {0x1606, 1, slave_2_pdo_entries + 6}, /* Channel 7 */
    {0x1607, 1, slave_2_pdo_entries + 7}, /* Channel 8 */
    {0x1608, 1, slave_2_pdo_entries + 8}, /* Channel 9 */
    {0x1609, 1, slave_2_pdo_entries + 9}, /* Channel 10 */
    {0x160a, 1, slave_2_pdo_entries + 10}, /* Channel 11 */
    {0x160b, 1, slave_2_pdo_entries + 11}, /* Channel 12 */
    {0x160c, 1, slave_2_pdo_entries + 12}, /* Channel 13 */
    {0x160d, 1, slave_2_pdo_entries + 13}, /* Channel 14 */
    {0x160e, 1, slave_2_pdo_entries + 14}, /* Channel 15 */
    {0x160f, 1, slave_2_pdo_entries + 15}, /* Channel 16 */
};

ec_sync_info_t slave_2_syncs[] = {
    {0, EC_DIR_OUTPUT, 8, slave_2_pdos + 0, EC_WD_ENABLE},
    {1, EC_DIR_OUTPUT, 8, slave_2_pdos + 8, EC_WD_ENABLE},
    {0xff}
};

// EtherCAT主站句柄
static ec_master_t *master = NULL;
static ec_master_state_t master_state = {};

// 域和从站配置
static ec_domain_t *domain1 = NULL;
static ec_domain_state_t domain_state = {};
static ec_slave_config_t *sc_el1809 = NULL;
static ec_slave_config_t *sc_el2809 = NULL;
static ec_slave_config_state_t sc_el1809_state = {};
static ec_slave_config_state_t sc_el2809_state = {};

// 中断处理
static volatile sig_atomic_t run = 1;
void signal_handler(int sig) { run = 0; }

// 检查从站状态
void check_domain_state() {
    ec_domain_state_t ds;
    ecrt_domain_state(domain1, &ds);

    if (ds.working_counter != domain_state.working_counter) {
        printf("Domain: WC %u\n", ds.working_counter);
    }
    domain_state = ds;
}

void check_master_state() {
    ec_master_state_t ms;
    ecrt_master_state(master, &ms);

    if (ms.slaves_responding != master_state.slaves_responding) {
        printf("%u slave(s).\n", ms.slaves_responding);
    }
    master_state = ms;
}

void check_slave_config_states() {
    ec_slave_config_state_t s;
    
    // 检查EL1809状态
    ecrt_slave_config_state(sc_el1809, &s);
    if (s.al_state != sc_el1809_state.al_state) {
        printf("EL1809: State 0x%02X\n", s.al_state);
    }
    sc_el1809_state = s;
    
    // 检查EL2809状态
    ecrt_slave_config_state(sc_el2809, &s);
    if (s.al_state != sc_el2809_state.al_state) {
        printf("EL2809: State 0x%02X\n", s.al_state);
    }
    sc_el2809_state = s;
}

int main(int argc, char *argv[]) {
    // 注册信号处理
    signal(SIGINT, signal_handler);

    printf("Initializing EtherCAT...\n");

    // 1. 获取主站实例
    master = ecrt_request_master(0);
    if (!master) {
        fprintf(stderr, "Failed to get master!\n");
        return 1;
    }

    // 2. 创建域
    domain1 = ecrt_master_create_domain(master);
    if (!domain1) {
        fprintf(stderr, "Failed to create domain!\n");
        return 1;
    }

    // 3. 配置EL1809输入从站
    sc_el1809 = ecrt_master_slave_config(
        master, 0, 1,  // 主站0, 从站1
        EL1809_VENDOR_ID, EL1809_PRODUCT_CODE);
    if (!sc_el1809) {
        fprintf(stderr, "Failed to configure EL1809!\n");
        return 1;
    }

    // 配置EL1809 PDO
    if (ecrt_slave_config_pdos(sc_el1809, EC_END, slave_1_syncs)) {
        fprintf(stderr, "Failed to configure EL1809 PDOs!\n");
        return 1;
    }

    // 4. 配置EL2809输出从站
    sc_el2809 = ecrt_master_slave_config(
        master, 0, 2,  // 主站0, 从站2
        EL2809_VENDOR_ID, EL2809_PRODUCT_CODE);
    if (!sc_el2809) {
        fprintf(stderr, "Failed to configure EL2809!\n");
        return 1;
    }

    // 配置EL2809 PDO
    if (ecrt_slave_config_pdos(sc_el2809, EC_END, slave_2_syncs)) {
        fprintf(stderr, "Failed to configure EL2809 PDOs!\n");
        return 1;
    }

    // 5. 注册PDO条目
    // EL1809输入注册
    ec_pdo_entry_reg_t in_regs[17] = {}; // 16通道 + 结束标记
    for (int i = 0; i < 16; i++) {
        in_regs[i].alias = 0;
        in_regs[i].position = 1;      // 从站位置1
        in_regs[i].vendor_id = EL1809_VENDOR_ID;
        in_regs[i].product_code = EL1809_PRODUCT_CODE;
        in_regs[i].index = slave_1_pdo_entries[i].index;
        in_regs[i].subindex = slave_1_pdo_entries[i].subindex;
        in_regs[i].offset = &in_offs[i];
        in_regs[i].bit_position = &in_bit_pos[i];
    }
    in_regs[16].index = 0; // 结束标记

    // EL2809输出注册
    ec_pdo_entry_reg_t out_regs[17] = {}; // 16通道 + 结束标记
    for (int i = 0; i < 16; i++) {
        out_regs[i].alias = 0;
        out_regs[i].position = 2;      // 从站位置2
        out_regs[i].vendor_id = EL2809_VENDOR_ID;
        out_regs[i].product_code = EL2809_PRODUCT_CODE;
        out_regs[i].index = slave_2_pdo_entries[i].index;
        out_regs[i].subindex = slave_2_pdo_entries[i].subindex;
        out_regs[i].offset = &out_offs[i];
        out_regs[i].bit_position = &out_bit_pos[i];
    }
    out_regs[16].index = 0; // 结束标记

    // 注册所有PDO条目
    if (ecrt_domain_reg_pdo_entry_list(domain1, in_regs)) {
        fprintf(stderr, "Failed to register EL1809 PDO entries!\n");
        return 1;
    }
    if (ecrt_domain_reg_pdo_entry_list(domain1, out_regs)) {
        fprintf(stderr, "Failed to register EL2809 PDO entries!\n");
        return 1;
    }

    // 6. 激活主站
    if (ecrt_master_activate(master)) {
        fprintf(stderr, "Failed to activate master!\n");
        return 1;
    }

    // 7. 获取过程数据域指针
    domain1_pd = ecrt_domain_data(domain1);
    if (!domain1_pd) {
        fprintf(stderr, "Failed to get domain data pointer!\n");
        return 1;
    }

    printf("EtherCAT initialization complete. Starting cycle...\n");
    printf("EL1809 Input Offsets and bit positions:\n");
    for (int i = 0; i < 16; i++) {
        printf("Channel %2d: offset=%3u, bit=%u\n", i+1, in_offs[i], in_bit_pos[i]);
    }
    printf("EL2809 Output Offsets and bit positions:\n");
    for (int i = 0; i < 16; i++) {
        printf("Channel %2d: offset=%3u, bit=%u\n", i+1, out_offs[i], out_bit_pos[i]);
    }

    // 主循环
    int cycle = 0;
    int light_pos = 0; // 当前点亮的LED位置
    while (run) {
        // 接收过程数据
        ecrt_master_receive(master);
        ecrt_domain_process(domain1);

        // 检查状态
        check_domain_state();
        check_master_state();
        check_slave_config_states();

        // 流水灯控制 - 每10个循环（约100ms）移动一次
        if (cycle % 10 == 0) {
            light_pos = (light_pos + 1) % 16;
        }

        // 设置EL2809输出 - 只点亮当前LED
        for (int i = 0; i < 16; i++) {
            if (i == light_pos) {
                EC_WRITE_BIT(domain1_pd + out_offs[i], out_bit_pos[i], 1);
            } else {
                EC_WRITE_BIT(domain1_pd + out_offs[i], out_bit_pos[i], 0);
            }
        }

        // 打印输入状态 (EL1809)
        printf("\nCycle %d 输入状态 (EL1809):\n", cycle);
        printf("+---------+---------+---------+---------+---------+---------+---------+---------+\n");
        for (int i = 0; i < 16; i++) {
            uint8_t val = EC_READ_BIT(domain1_pd + in_offs[i], in_bit_pos[i]);
            printf("| 通道 %2d: %d ", i + 1, val);
            if ((i + 1) % 4 == 0)
                printf("|\n");
        }
        printf("+---------+---------+---------+---------+---------+---------+---------+---------+\n");

        // 打印输出状态 (EL2809)
        printf("Cycle %d 输出状态 (EL2809):\n", cycle);
        printf("+---------+---------+---------+---------+---------+---------+---------+---------+\n");
        for (int i = 0; i < 16; i++) {
            uint8_t val = EC_READ_BIT(domain1_pd + out_offs[i], out_bit_pos[i]);
            printf("| 通道 %2d: %d ", i + 1, val);
            if ((i + 1) % 4 == 0)
                printf("|\n");
        }
        printf("+---------+---------+---------+---------+---------+---------+---------+---------+\n");
        printf("当前点亮: 通道 %d\n", light_pos + 1);

        // 发送过程数据
        ecrt_domain_queue(domain1);
        ecrt_master_send(master);

        // 控制循环频率 (约100Hz)
        usleep(10000);
        cycle++;
    }

    // 清理 - 关闭所有输出
    for (int i = 0; i < 16; i++) {
        EC_WRITE_BIT(domain1_pd + out_offs[i], out_bit_pos[i], 0);
    }
    ecrt_domain_queue(domain1);
    ecrt_master_send(master);
    
    ecrt_release_master(master);
    printf("EtherCAT master released. All outputs turned off.\n");
    return 0;
}