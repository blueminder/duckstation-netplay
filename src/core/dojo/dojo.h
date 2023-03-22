#include <cinttypes>
#include <iostream>
#include <sstream>

#include "session.h"
#include "frame.h"
#include "message.h"
#include "replay.h"
#include "training.h"

#include "net/net.h"

#define FRAME_SIZE 12
#define HEADER_LEN 12

