#include "pid.h"

int pid_init(pid_t* pid, const float Kp, const float Ki, const float Kd)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    return 0;
}

int pid_set_gains(pid_t* pid, float Kp, float Ki, float Kd)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;

    return 0;
}

int pid_set_setpoint(pid_t* pid, float setPoint)
{
    pid->setPoint = setPoint;
    return 0;
}

int pid_process(pid_t* pid, float actual, float actualTime)
{
    // auto timeDiff = actualTime - _lastTime;
    // _error = _setPoint - *_input;
    //
    // pid->pro
    //
    // proportionalError = static_cast<int>(_proportionalGain * _error);
    // _integralError += static_cast<int>(_integralGain * (_error + _lastError) / 2 * timeDiff);
    // _derivativeError = static_cast<int>(_derivativeGain * (_error - _lastError) / timeDiff);
    //
    // if (_integralError > _maxIntError) _integralError = _maxIntError;
    // if (_integralError < -_maxIntError) _integralError = -_maxIntError;
    //
    // auto controlOutput = _proportionalError + _integralError + _derivativeError;
    //
    // if (_outputBounded) {
    //     if (controlOutput < _outputLowerBound) controlOutput = _outputLowerBound;
    //     if (controlOutput > _outputUpperBound) controlOutput = _outputUpperBound;
    // }
    //
    // _lastTime = actualTime;
    // _lastError = _error;
    //
    //
    // *_output = controlOutput;
}


//
// PID::PID() : PID(0,0,0, nullptr, nullptr) {
//
// }
//
// PID::PID(double Kp) : PID(Kp, 0, 0, nullptr, nullptr){
//
// }
//
// PID::PID(double Kp, double Ki) : PID(Kp, Ki, 0, nullptr, nullptr){
//
// }
//
// PID::PID(double Kp, double Ki, double Kd) : PID(Kp, Ki, Kd, nullptr, nullptr) {
//
// }
//
// PID::PID(double Kp, uint16_t *input, uint16_t *output) : PID(Kp, 0, 0, input, output) {
//
// }
//
// PID::PID(double Kp, double Ki, uint16_t *input, uint16_t *output) : PID(Kp, Ki, 0, input, output) {
//
// }
//
// PID::PID(double Kp, double Ki, double Kd, uint16_t *input, uint16_t *output) {
//     _proportionalGain = Kp;
//     _integralGain = Ki;
//     _derivativeGain = Kd;
//
//     _input = input;
//     _output = output;
// }
//
// PID::~PID() {
//
// }
//
// void PID::processData(uint32_t actualTime) {
//     if (!_enabled) {
//         *_output = 0;
//         return;
//     }
//
//     auto timeDiff = actualTime - _lastTime;
//     _error = _setPoint - *_input;
//
//     _proportionalError = static_cast<int>(_proportionalGain * _error);
//     _integralError += static_cast<int>(_integralGain * (_error + _lastError) / 2 * timeDiff);
//     _derivativeError = static_cast<int>(_derivativeGain * (_error - _lastError) / timeDiff);
//
//     if (_integralError > _maxIntError) _integralError = _maxIntError;
//     if (_integralError < -_maxIntError) _integralError = -_maxIntError;
//
//     auto controlOutput = _proportionalError + _integralError + _derivativeError;
//
//     if (_outputBounded) {
//         if (controlOutput < _outputLowerBound) controlOutput = _outputLowerBound;
//         if (controlOutput > _outputUpperBound) controlOutput = _outputUpperBound;
//     }
//
//     _lastTime = actualTime;
//     _lastError = _error;
//
//
//     *_output = controlOutput;
// }
//
// double PID::getProportionalGain() const {
//     return _proportionalGain;
// }
//
// double PID::getIntegralGain() const {
//     return _integralGain;
// }
//
// double PID::getDerivativeGain() const {
//     return _derivativeGain;
// }
//
// void PID::setProportionalGain(double Kp) {
//     _proportionalGain = Kp;
// }
//
// void PID::setIntegralGain(double Ki) {
//     _integralGain = Ki;
// }
//
// void PID::setDerivativeGain(double Kd) {
//     _derivativeGain = Kd;
// }
//
// void PID::setInput(uint16_t *input) {
//     _input = input;
// }
//
// void PID::setOutput(uint16_t *output) {
//     _output = output;
// }
//
// uint16_t *PID::getInput() const {
//     return _input;
// }
//
// uint16_t *PID::getOutput() const {
//     return _output;
// }
//
// int PID::getSetPoint() const {
//     return _setPoint;
// }
//
// void PID::setSetPoint(int setPoint) {
//     _setPoint = setPoint;
// }
//
// bool PID::isEnabled() const {
//     return _enabled;
// }
//
// bool PID::enableControl() {
//     return _enabled = true;
// }
//
// bool PID::disableControl() {
//     return _enabled = false;
// }
//
// bool PID::toggleControl() {
//     return _enabled = !_enabled;
// }
//
// void PID::setOutputBounds(int lower, int upper) {
//     _outputBounded = true;
//     _outputLowerBound = lower;
//     _outputUpperBound = upper;
// }
//
// bool PID::isOutputBounded() const {
//     return _outputBounded;
// }
//
// int PID::getOutputLowerBound() const {
//     return _outputLowerBound;
// }
//
// int PID::getOutputUpperBound() const {
//     return _outputUpperBound;
// }