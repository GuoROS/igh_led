#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <unistd.h>
#include <ecrt.h>
#include <iostream>
#include <cstdint>
#include <thread>
#include <chrono>

// 从站配置
constexpr uint32_t EL1809_VENDOR_ID = 0x00000002;
constexpr uint32_t EL1809_PRODUCT_CODE = 0x07113052;
constexpr uint32_t EL2809_VENDOR_ID = 0x00000002;
constexpr uint32_t EL2809_PRODUCT_CODE = 0x0af93052;

// 过程数据指针
static uint8_t* domain1_pd = nullptr;


// 通道偏移量和位位置
static unsigned int in_offs[16] = {0};
static unsigned int in_bit_pos[16] = {0};
static unsigned int out_offs[16] = {0};
static unsigned int out_bit_pos[16] = {0};



// EtherCAT对象
static ec_master_t* master = nullptr;
static ec_domain_t* domain1 = nullptr;
static volatile sig_atomic_t run = 1;

// 信号处理
void signal_handler(int sig) { run = 0; }
// 资源清理函数
void cleanup() {
    if (domain1) {
        ecrt_domain_queue(domain1);
        ecrt_master_send(master);
    }
    if (master) {
        ecrt_release_master(master);
    }
}
int main() {
    std::signal(SIGINT, signal_handler);

    // 初始化EtherCAT
    master = ecrt_request_master(0);
    if (!master) {
        std::cerr << "Failed to get master!" << std::endl;
        return 1;
    }

    domain1 = ecrt_master_create_domain(master);
    if (!domain1) {
        std::cerr << "Failed to create domain!" << std::endl;
        ecrt_release_master(master);
        return 1;
    }

    // =============== 配置EL1809输入从站 ===============
    ec_slave_config_t* sc_el1809 = ecrt_master_slave_config(
        master, 0, 1, EL1809_VENDOR_ID, EL1809_PRODUCT_CODE);
    if (!sc_el1809) {
        std::cerr << "Failed to configure EL1809!" << std::endl;
        cleanup();
        return 1;
    }

    // EL1809 PDO条目配置
    const ec_pdo_entry_info_t slave_1_pdo_entries[] = {
        {0x6000, 0x01, 1}, {0x6010, 0x01, 1}, {0x6020, 0x01, 1}, {0x6030, 0x01, 1},
        {0x6040, 0x01, 1}, {0x6050, 0x01, 1}, {0x6060, 0x01, 1}, {0x6070, 0x01, 1},
        {0x6080, 0x01, 1}, {0x6090, 0x01, 1}, {0x60a0, 0x01, 1}, {0x60b0, 0x01, 1},
        {0x60c0, 0x01, 1}, {0x60d0, 0x01, 1}, {0x60e0, 0x01, 1}, {0x60f0, 0x01, 1}
    };

    // EL1809 PDO配置
    const ec_pdo_info_t slave_1_pdos[] = {
        {0x1a00, 1, &slave_1_pdo_entries[0]},  {0x1a01, 1, &slave_1_pdo_entries[1]},
        {0x1a02, 1, &slave_1_pdo_entries[2]},  {0x1a03, 1, &slave_1_pdo_entries[3]},
        {0x1a04, 1, &slave_1_pdo_entries[4]},  {0x1a05, 1, &slave_1_pdo_entries[5]},
        {0x1a06, 1, &slave_1_pdo_entries[6]},  {0x1a07, 1, &slave_1_pdo_entries[7]},
        {0x1a08, 1, &slave_1_pdo_entries[8]},  {0x1a09, 1, &slave_1_pdo_entries[9]},
        {0x1a0a, 1, &slave_1_pdo_entries[10]}, {0x1a0b, 1, &slave_1_pdo_entries[11]},
        {0x1a0c, 1, &slave_1_pdo_entries[12]}, {0x1a0d, 1, &slave_1_pdo_entries[13]},
        {0x1a0e, 1, &slave_1_pdo_entries[14]}, {0x1a0f, 1, &slave_1_pdo_entries[15]}
    };

    // EL1809同步管理器配置
    const ec_sync_info_t slave_1_syncs[] = {
        {0, EC_DIR_INPUT, 16, slave_1_pdos, EC_WD_DISABLE},
        {0xff}
    };

    if (ecrt_slave_config_pdos(sc_el1809, EC_END, slave_1_syncs)) {
        std::cerr << "Failed to configure EL1809 PDOs!" << std::endl;
        cleanup();
        return 1;
    }

    // =============== 配置EL2809输出从站 ===============
    ec_slave_config_t* sc_el2809 = ecrt_master_slave_config(
        master, 0, 2, EL2809_VENDOR_ID, EL2809_PRODUCT_CODE);
    if (!sc_el2809) {
        std::cerr << "Failed to configure EL2809!" << std::endl;
        cleanup();
        return 1;
    }

    // EL2809 PDO条目配置
    const ec_pdo_entry_info_t slave_2_pdo_entries[] = {
        {0x7000, 0x01, 1}, {0x7010, 0x01, 1}, {0x7020, 0x01, 1}, {0x7030, 0x01, 1},
        {0x7040, 0x01, 1}, {0x7050, 0x01, 1}, {0x7060, 0x01, 1}, {0x7070, 0x01, 1},
        {0x7080, 0x01, 1}, {0x7090, 0x01, 1}, {0x70a0, 0x01, 1}, {0x70b0, 0x01, 1},
        {0x70c0, 0x01, 1}, {0x70d0, 0x01, 1}, {0x70e0, 0x01, 1}, {0x70f0, 0x01, 1}
    };

    // EL2809 PDO配置
    const ec_pdo_info_t slave_2_pdos[] = {
        {0x1600, 1, &slave_2_pdo_entries[0]},  {0x1601, 1, &slave_2_pdo_entries[1]},
        {0x1602, 1, &slave_2_pdo_entries[2]},  {0x1603, 1, &slave_2_pdo_entries[3]},
        {0x1604, 1, &slave_2_pdo_entries[4]},  {0x1605, 1, &slave_2_pdo_entries[5]},
        {0x1606, 1, &slave_2_pdo_entries[6]},  {0x1607, 1, &slave_2_pdo_entries[7]},
        {0x1608, 1, &slave_2_pdo_entries[8]},  {0x1609, 1, &slave_2_pdo_entries[9]},
        {0x160a, 1, &slave_2_pdo_entries[10]}, {0x160b, 1, &slave_2_pdo_entries[11]},
        {0x160c, 1, &slave_2_pdo_entries[12]}, {0x160d, 1, &slave_2_pdo_entries[13]},
        {0x160e, 1, &slave_2_pdo_entries[14]}, {0x160f, 1, &slave_2_pdo_entries[15]}
    };

    // EL2809同步管理器配置
    const ec_sync_info_t slave_2_syncs[] = {
        {0, EC_DIR_OUTPUT, 8, slave_2_pdos, EC_WD_ENABLE},
        {1, EC_DIR_OUTPUT, 8, slave_2_pdos + 8, EC_WD_ENABLE},
        {0xff}
    };

    if (ecrt_slave_config_pdos(sc_el2809, EC_END, slave_2_syncs)) {
        std::cerr << "Failed to configure EL2809 PDOs!" << std::endl;
        cleanup();
        return 1;
    }

    // =============== 注册PDO条目 ===============
    ec_pdo_entry_reg_t regs[33] = {};

    // 注册输入通道
    for (int i = 0; i < 16; ++i) {
        regs[i] = {0, 1, EL1809_VENDOR_ID, EL1809_PRODUCT_CODE,
                  slave_1_pdo_entries[i].index, slave_1_pdo_entries[i].subindex,
                  &in_offs[i], &in_bit_pos[i]};
    }

    // 注册输出通道
    for (int i = 0; i < 16; ++i) {
        regs[16 + i] = {0, 2, EL2809_VENDOR_ID, EL2809_PRODUCT_CODE,
                       slave_2_pdo_entries[i].index, slave_2_pdo_entries[i].subindex,
                       &out_offs[i], &out_bit_pos[i]};
    }

    // 结束标记
    regs[32].index = 0;

    if (ecrt_domain_reg_pdo_entry_list(domain1, regs)) {
        std::cerr << "Failed to register PDO entries!" << std::endl;
        cleanup();
        return 1;
    }

    // =============== 激活主站 ===============
    if (ecrt_master_activate(master)) {
        std::cerr << "Failed to activate master!" << std::endl;
        cleanup();
        return 1;
    }

    if (!(domain1_pd = ecrt_domain_data(domain1))) {
        std::cerr << "Failed to get domain data pointer!" << std::endl;
        cleanup();
        return 1;
    }

    // =============== 主循环 ===============
    constexpr int CYCLE_FREQ = 100;       // 100Hz
    constexpr int LED_UPDATE_HZ = 10;     // 10Hz
    constexpr int LED_UPDATE_INTERVAL = CYCLE_FREQ / LED_UPDATE_HZ;
    
    int cycle = 0;
    int light_pos = 0;
    auto cycle_time = std::chrono::microseconds(1000000 / CYCLE_FREQ);

    std::cout << "EtherCAT running. Press Ctrl-C to exit." << std::endl;

    while (run) {
        ecrt_master_receive(master);
        ecrt_domain_process(domain1);

        // 更新LED位置
        if (cycle % LED_UPDATE_INTERVAL == 0) {
            light_pos = (light_pos + 1) % 16;
        }

        // 设置输出
        for (int i = 0; i < 16; ++i) {
            EC_WRITE_BIT(domain1_pd + out_offs[i], out_bit_pos[i], 1);
        }

        ecrt_domain_queue(domain1);
        ecrt_master_send(master);

        std::this_thread::sleep_for(cycle_time);
        cycle = (cycle + 1) % CYCLE_FREQ;
    }

    // =============== 清理 ===============
    // 关闭所有输出
    for (int i = 0; i < 16; ++i) {
        EC_WRITE_BIT(domain1_pd + out_offs[i], out_bit_pos[i], 0);
    }
    ecrt_domain_queue(domain1);
    ecrt_master_send(master);

    cleanup();
    std::cout << "EtherCAT master released. All outputs turned off." << std::endl;
    return 0;
}

