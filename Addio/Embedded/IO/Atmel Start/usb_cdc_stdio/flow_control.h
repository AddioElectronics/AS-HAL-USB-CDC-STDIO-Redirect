
#ifndef FLOW_CONTROL_H_
#define FLOW_CONTROL_H_

#include <stdbool.h>

void test_flow_control_init();
void test_send_serial_state(bool dcd, bool dsr, bool breakstate);
void test_send_call_state_change(bool idle);

#endif /* FLOW_CONTROL_H_ */