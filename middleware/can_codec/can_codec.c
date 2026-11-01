#include "can_codec.h"
#include "can_matrix.h"
#include <string.h>

/* Hàm phụ trợ đóng gói uint16 theo chuẩn Little-Endian (Mục 9.4 Customer Spec) */
static inline void PackU16LE(uint8_t *buf, uint16_t val)
{
    buf[0] = (uint8_t)(val & 0xFFU);
    buf[1] = (uint8_t)((val >> 8U) & 0xFFU);
}

/* Hàm tính Simple CRC-8 (XOR hoặc Modulo sum) cho frame 0x400 */
static uint8_t CalcSimpleChecksum(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0U;
    for (uint8_t i = 0U; i < len; i++) {
        crc ^= data[i];
    }
    return crc;
}

/* -------------------------------------------------------------------------
 * Frame 0x100: VehicleDynamic (DLC = 8, Cycle = 20 ms)
 * - Bytes 0..1: VehicleSpeed (uint16 LE, 0.01 km/h, 0xFFFF = invalid)
 * - Bytes 2..3: EngineRPM    (uint16 LE, 0.25 rpm,  0xFFFF = invalid)
 * - Byte 4    : Gear         (uint8, 0=P,1=R,2=N,3=D, 0xFF = invalid)
 * - Byte 5    : Validity bitmap (Bit 0: Speed valid, Bit 1: RPM valid)
 * - Byte 7[3:0]: DynCounter  (uint4, 0..15 modulo 16)
 * ------------------------------------------------------------------------- */
void CanCodec_EncodeVehicleDynamic(const VehicleMotionSnapshot_t *motion,
                                   uint8_t rolling_counter,
                                   uint8_t out_payload[8])
{
    memset(out_payload, 0, 8U);
    if (motion == NULL) {
        return;
    }

    /* Yêu cầu VEH-005: Nếu tín hiệu không hợp lệ, phát giá trị 0xFFFF */
    uint16_t raw_speed = (motion->speed_quality == SIGNAL_VALID)
                         ? motion->speed_centi_kmh
                         : CAN_RAW_U16_INVALID;

    uint16_t raw_rpm   = (motion->rpm_quality == SIGNAL_VALID)
                         ? motion->rpm_quarter
                         : CAN_RAW_U16_INVALID;

    PackU16LE(&out_payload[0], raw_speed);
    PackU16LE(&out_payload[2], raw_rpm);

    out_payload[4] = (motion->gear <= 3U) ? motion->gear : CAN_RAW_U8_INVALID;

    /* Byte 5: Cờ báo tính hợp lệ (Validity indication) */
    uint8_t validity_flags = 0U;
    if (motion->speed_quality == SIGNAL_VALID) { validity_flags |= 0x01U; }
    if (motion->rpm_quality   == SIGNAL_VALID) { validity_flags |= 0x02U; }
    out_payload[5] = validity_flags;

    /* Byte 7[3:0]: Rolling counter 0..15 */
    out_payload[7] = (rolling_counter & 0x0FU);
}

/* -------------------------------------------------------------------------
 * Frame 0x101: VehicleStatus (DLC = 8, Cycle = 100 ms)
 * - Byte 0[1:0]: Ignition (0=OFF, 1=ACC, 2=ON, 3=invalid)
 * ------------------------------------------------------------------------- */
void CanCodec_EncodeVehicleStatus(const VehicleMotionSnapshot_t *motion,
                                  uint8_t out_payload[8])
{
    memset(out_payload, 0, 8U);
    if (motion == NULL) {
        return;
    }

    uint8_t ign_raw = ((uint8_t)motion->ignition <= 2U) ? (uint8_t)motion->ignition : 0x03U;
    out_payload[0] = (ign_raw & 0x03U);
}

/* -------------------------------------------------------------------------
 * Frame 0x200: BodyStatus (DLC = 8, Cycle = 100 ms)
 * - Byte 0[3:0]: DoorFL/FR/RL/RR (0=closed, 1=open)
 * - Byte 1[1:0]: CentralLock     (0=unlocked, 1=locked)
 * - Byte 2     : Light state (Bit 0), Horn state (Bit 1)
 * - Byte 7[3:0]: Rolling counter
 * ------------------------------------------------------------------------- */
void CanCodec_EncodeBodyStatus(const BodyStateSnapshot_t *body,
                               uint8_t rolling_counter,
                               uint8_t out_payload[8])
{
    memset(out_payload, 0, 8U);
    if (body == NULL) {
        return;
    }

    out_payload[0] = (body->door_open_mask & 0x0FU);
    out_payload[1] = ((uint8_t)body->central_lock & 0x03U);

    uint8_t act_flags = 0U;
    if (body->light_active) { act_flags |= 0x01U; }
    if (body->horn_active)  { act_flags |= 0x02U; }
    out_payload[2] = act_flags;

    out_payload[7] = (rolling_counter & 0x0FU);
}

/* -------------------------------------------------------------------------
 * Frame 0x500: CommandAck (DLC = 8, Event)
 * - Byte 0: Command sequence
 * - Byte 1: Command ID
 * - Byte 2: Result / Reject reason (CmdAckResult_t)
 * - Byte 3: Resulting state
 * ------------------------------------------------------------------------- */
void CanCodec_EncodeCommandAck(const CommandAckMsg_t *ack,
                               uint8_t out_payload[8])
{
    memset(out_payload, 0, 8U);
    if (ack == NULL) {
        return;
    }

    out_payload[0] = ack->sequence;
    out_payload[1] = ack->cmd_id;
    out_payload[2] = (uint8_t)ack->result;
    out_payload[3] = ack->resulting_state;
}

/* -------------------------------------------------------------------------
 * Frame 0x400: RemoteCommand (DLC = 8, Event từ TCU)
 * - Byte 0   : seq
 * - Byte 1   : cmd_id
 * - Bytes 2..3: parameter (uint16 LE)
 * - Byte 7   : simple CRC (XOR của bytes 0..6)
 * ------------------------------------------------------------------------- */
void CanCodec_DecodeRemoteCommand(const uint8_t in_payload[8],
                                  uint8_t dlc,
                                  RemoteCommandReq_t *out_req)
{
    if ((in_payload == NULL) || (out_req == NULL)) {
        return;
    }

    if (dlc < 8U) {
        out_req->crc_valid = false;
        return;
    }

    out_req->sequence  = in_payload[0];
    out_req->cmd_id    = in_payload[1];
    out_req->parameter = (uint16_t)in_payload[2] | ((uint16_t)in_payload[3] << 8U);

    uint8_t expected_crc = CalcSimpleChecksum(in_payload, 7U);
    out_req->crc_valid   = (in_payload[7] == expected_crc);
}/*
 * can_codec.c
 *
 *  Created on: Nov 1, 2026
 *      Author: vandang
 */


