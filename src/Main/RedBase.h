#pragma once

#include "log.h"

#include <fstream>

typedef int RC;

static const RC OK = 0;

static const RC START_PF_WARN  = 1;

static const RC START_PF_ERR   = -1;

static std::ofstream log;
