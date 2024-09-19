#pragma once

#include "../VFDSpindle.h"

namespace Spindles {
    namespace VFD {
        class NowForever : public VFDDetail {
        protected:
            uint16_t _minFrequency = 0;
            uint16_t _maxFrequency = 0;

            void updateRPM(VFDSpindle* spindle);

            void direction_command(SpindleState mode, ModbusCommand& data) override;
            void set_speed_command(uint32_t hz, ModbusCommand& data) override;

            response_parser initialization_sequence(int index, ModbusCommand& data) override;
            response_parser get_current_speed(ModbusCommand& data) override;
            response_parser get_current_direction(ModbusCommand& data) override;
            response_parser get_status_ok(ModbusCommand& data) override;
            bool            safety_polling() const { return true; }

            bool use_delay_settings() const override { return false; }
        };
    }
}
