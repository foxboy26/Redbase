#pragma once

#include "PF_Exception.h"

typedef int PageNum;

//static const int PF_PAGE_SIZE = 4092;
static const int PF_PAGE_SIZE = 4092 + 4;

static const int PF_BUFFER_SIZE = 40;

class PF_Manager;

class PF_FileHandle;

class PF_PageHandle;
