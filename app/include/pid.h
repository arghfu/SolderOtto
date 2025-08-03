#pragma once

typedef struct pid {
    float Kp;
    float Ki;
    float Kd;
    float error;
    float lastError;
    float integralError;
    float derivativeError;
    float setPoint;
    float output;
}pid_t;


int pid_init(pid_t *pid, float Kp, float Ki, float Kd);
int pid_set_gains(pid_t *pid, float Kp, float Ki, float Kd);
int pid_set_setpoint(pid_t *pid, float setPoint);
int pid_process(pid_t *pid, float actual, float actualTime);

// class PID
// {
// private:
//     double _proportionalGain = 0;
//     double _integralGain = 0;
//     double _derivativeGain = 0;
//
//     uint16_t * _input;
//     uint16_t* _output;
//
//     int _error = 0;
//     int _proportionalError = 0;
//     int _integralError = 0;
//     int _derivativeError = 0;
//     int _lastError = 0;
//     int _setPoint = 0;
//     int _maxIntError = 2500;
//     uint32_t _lastTime = 0;
//
//     bool _outputBounded = false;
//     int _outputLowerBound = 0;
//     int _outputUpperBound = 0;
//
//     bool _enabled = false;
//
// public:
//     PID();
//     PID(double Kp);
//     PID(double Kp, double Ki);
//     PID(double Kp, double Ki, double Kd);
//     PID(double Kp, uint16_t *input, uint16_t *output);
//     PID(double Kp, double Ki, uint16_t *input, uint16_t *output);
//     PID(double Kp, double Ki, double Kd, uint16_t *input, uint16_t *output);
//     virtual ~PID();
//
//     double getProportionalGain() const;
//     double getIntegralGain() const;
//     double getDerivativeGain() const;
//     void setProportionalGain(double Kp);
//     void setIntegralGain(double Ki);
//     void setDerivativeGain(double Kd);
//
//     void setInput(uint16_t *input);
//     void setOutput(uint16_t *output);
//     uint16_t *getInput() const;
//     uint16_t *getOutput() const;
//
//     int getSetPoint() const;
//     void setSetPoint(int setPoint);
//
//     bool isEnabled() const;
//     bool enableControl();
//     bool disableControl();
//     bool toggleControl();
//
//     void setOutputBounds(int lower, int upper);
//     bool isOutputBounded() const;
//
//     int getOutputLowerBound() const;
//     int getOutputUpperBound() const;
//
//     void processData(uint32_t actualTime);
// };
