#pragma once
#include "pch.h"

class UnclassifiedException
    :
    public Exception
{
public:
    using Exception::Exception;
};