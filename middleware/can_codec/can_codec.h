/*
 * can_codec.h
 *
 *  Created on: Nov 1, 2026
 *      Author: vandang
 */

#ifndef CAN_CODEC_CAN_CODEC_H_
#define CAN_CODEC_CAN_CODEC_H_
#include <stdint.h>
#include <stdbool.h>
#include "app_types.h"

/* =========================================================================
 * 1. CÁC HÀM MÃ HÓA (ENCODE: Struct Nội bộ -> 8 Bytes CAN Payload)
 * Xuất phát từ Bảng 9.2 & 9.4 của Customer Spec (Little-Endian)
 * ========================================================================= */

/* Mã hóa frame 0x100 (VehicleDynamic - chu kỳ 20 ms) */
void CanCodec_EncodeVehicleDynamic(const VehicleMotionSnapshot_t *motion,
                                   uint8_t rolling_counter,
                                   uint8_t out_payload[8]);

/* Mã hóa frame 0x101 (VehicleStatus - chu kỳ 100 ms) */
void CanCodec_EncodeVehicleStatus(const VehicleMotionSnapshot_t *motion,
                                  uint8_t out_payload[8]);

/* Mã hóa frame 0x200 (BodyStatus - chu kỳ 100 ms) */
void CanCodec_EncodeBodyStatus(const BodyStateSnapshot_t *body,
                               uint8_t rolling_counter,
                               uint8_t out_payload[8]);

/* Mã hóa frame 0x500 (CommandAck - sự kiện phản hồi lệnh) */
void CanCodec_EncodeCommandAck(const CommandAckMsg_t *ack,
                               uint8_t out_payload[8]);

/* =========================================================================
 * 2. CÁC HÀM GIẢI MÃ (DECODE: 8 Bytes CAN Payload -> Struct Nội bộ)
 * Xuất phát từ Bảng 9.3 & Mục 11 của Customer Spec
 * ========================================================================= */

/* Giải mã frame 0x400 (RemoteCommand từ TCU gửi xuống) */
void CanCodec_DecodeRemoteCommand(const uint8_t in_payload[8],
                                  uint8_t dlc,
                                  RemoteCommandReq_t *out_req);


#endif /* CAN_CODEC_CAN_CODEC_H_ */
