#ifndef __WOLFCC_HANDLE_H__
#define __WOLFCC_HANDLE_H__

struct Handle
{
	int hdl;

	Handle(int hdl_ = -1)
	: hdl(hdl_)
	{
	}

	operator int() const 
	{
		return hdl;
	}
};

#endif

