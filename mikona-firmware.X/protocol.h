#ifndef PROTOCOL_H
#define	PROTOCOL_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

union reg_u16
{
    uint8_t u8[2];
    uint16_t u16;
};
    
typedef enum
{
    REG_ADDR_DEV_ID = 0x01,
    REG_ADDR_STATUS = 0x02,
    REG_ADDR_V_OUT  = 0x03,
    REG_ADDR_KICK_A = 0x04,
    REG_ADDR_KICK_B = 0x05,
} reg_addr_t;

#define DEV_ID 0x54

struct registers_t
{
    uint8_t status;       // read-write @ REG_ADDR_STATUS
    uint8_t v_out;        // read-only  @ REG_ADDR_V_OUT
    union reg_u16 kick_a; // write-only @ REG_ADDR_KICK_A
    union reg_u16 kick_b; // write-only @ REG_ADDR_KICK_B
};
extern struct registers_t g_registers;

// status bits:
// [7|6|5|4|  3  |  2 |    1    |   0  ]
// [X|X|X|X|FAULT|DONE|DISCHARGE|CHARGE]

#define REG_STATUS_CHARGE_BIT    0
#define REG_STATUS_DISCHARGE_BIT 1
#define REG_STATUS_DONE_BIT      2
#define REG_STATUS_FAULT_BIT     3

#define REG_STATUS_READ_MASK  0b00001111
#define REG_STATUS_WRITE_MASK 0b00001011

// bit helpers (operate on g_registers.status)
#define REG_GET_BIT(bit)        ((g_registers.status >> (bit)) & 1u)
#define REG_SET_BIT(bit, value) (g_registers.status = (uint8_t)((g_registers.status & ~(uint8_t)(1u << (bit))) | ((uint8_t)(value) << (bit))))

// sets our i2c interrupt handlers
void set_i2c_callbacks(void);

#ifdef	__cplusplus
}
#endif

#endif	/* PROTOCOL_H */
