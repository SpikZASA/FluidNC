#include "OnOffSpindle.h"

#include "../System.h"  // sys.abort

namespace Spindles {

    void OnOff::init() {
        if (_output_pin.undefined() && _enable_pin.undefined()) {
            log_error("Either output pin or enable pin must be defined for OnOff Spindle");
            return;
        }

        _enable_pin.setAttr(Pin::Attr::Output);
        _output_pin.setAttr(Pin::Attr::Output);
        _direction_pin.setAttr(Pin::Attr::Output);

        is_reversable = _direction_pin.defined();

        if (_speeds.size() == 0) {
            // The default speed map for an On/Off spindle is off - 0% -
            // for speed 0 and on - 100% - for any nonzero speedl
            // In other words there is a step transition right at 0.
            _speeds.push_back({ 0, 0 });
            _speeds.push_back({ 0, 100 });
        }
        setupSpeeds(1);
        config_message();
    }

    // prints the startup message of the spindle config
    void OnOff ::config_message() {
        log_info(name() << " spindle Ena:" << _enable_pin.name().c_str() << " Out:" << _output_pin.name().c_str()
                        << " Dir:" << _direction_pin.name().c_str());
    }

    void OnOff::setState(SpindleState state, SpindleSpeed speed) {
        if (sys.abort) {
            return;  // Block during abort.
        }

        // We always use mapSpeed() with the unmodified input speed so it sets
        // sys.spindle_speed correctly.
        uint32_t dev_speed = mapSpeed(speed);
        if (state == SpindleState::Disable) {  // Halt or set spindle direction and speed.
            if (_zero_speed_with_disable) {
                dev_speed = offSpeed();
            }
        } else {
            // XXX this could wreak havoc if the direction is changed without first
            // spinning down.
            set_direction(state == SpindleState::Cw);
        }
        set_output(dev_speed);
        set_enable(state != SpindleState::Disable);
        spinDelay(state, speed);

        sys.report_ovr_counter = 0;  // Set to report change immediately
    }

    void IRAM_ATTR OnOff::set_output(uint32_t dev_speed) { _output_pin.write(dev_speed != 0); }

    void IRAM_ATTR OnOff::setSpeedfromISR(uint32_t dev_speed) { set_output(dev_speed != 0); }

    void OnOff::set_enable(bool enable) {
        if (_disable_with_zero_speed && sys.spindle_speed == 0) {
            enable = false;
        }

        _enable_pin.write(enable);
    }

    void OnOff::set_direction(bool Clockwise) { _direction_pin.write(Clockwise); }

    // 0=0% 0=100%
    void OnOff::deinit() {
        stop();
        _enable_pin.setAttr(Pin::Attr::Input);
        _output_pin.setAttr(Pin::Attr::Input);
        _direction_pin.setAttr(Pin::Attr::Input);
    }

    // Configuration registration
    namespace {
        SpindleFactory::InstanceBuilder<OnOff> registration("OnOff");
    }
}
