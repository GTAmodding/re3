#include "common.h"
#include "patcher.h"
#include "Darkel.h"

WRAPPER void CDarkel::DrawMessages(void) { EAXJMP(0x420920); }

bool CDarkel::Status = *(bool*)0x95CCB4;

bool CDarkel::FrenzyOnGoing()
{
	return Status;
}