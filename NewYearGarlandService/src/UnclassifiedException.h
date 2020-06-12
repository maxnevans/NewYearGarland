#pragma once
#include "Exception.h"

class UnclassifiedException
    :
    public Exception
{
public:
    using Exception::Exception;
};