#ifndef INPUT_SWITCH_POWEROFF_H
#define INPUT_SWITCH_POWEROFF_H
/* Nintendo Switch Pro, Windows Bluetooth HID. No pairing or flash writes. */
#include <stdint.h>
#include <string.h>

#define SWOS_SWITCH_SLEEP_REPORT_SIZE 49u

static inline unsigned swos_switch_ascii_lower(unsigned c)
{
    return c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c;
}

/* Accept only the Windows classic Bluetooth HID service and exact VID/PID.
 * SDL HIDAPI joystick GUIDs alone do not identify the transport reliably. */
static inline int swos_switch_sleep_eligible(unsigned vendor, unsigned product,
                                             const char *path)
{
    static const char prefix[] = "\\\\?\\hid#{00001124-0000-1000-8000-00805f9b34fb}_vid&0002057e_pid&2009#";
    if (vendor != 0x057eu || product != 0x2009u || !path)
        return 0;
    for (size_t i = 0; i < sizeof prefix - 1u; ++i)
        if (!path[i] || swos_switch_ascii_lower((unsigned char)path[i]) != (unsigned char)prefix[i])
            return 0;
    return path[sizeof prefix - 1u] != '\0';
}

/* Output report 01, counter 0, neutral mandatory protocol fields, HCI
 * subcommand 06/00 (disconnect/sleep). Not shipment mode or unpairing. */
static inline void swos_switch_sleep_report(uint8_t out[SWOS_SWITCH_SLEEP_REPORT_SIZE])
{
    static const uint8_t header[] = {0x01,0x00,0x00,0x01,0x40,0x40,0x00,0x01,0x40,0x40,0x06,0x00};
    memset(out,0,SWOS_SWITCH_SLEEP_REPORT_SIZE);
    memcpy(out,header,sizeof header);
}
#endif
