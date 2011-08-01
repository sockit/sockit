/*
 * NetworkObject.cpp
 *
 *  Created on: Jun 10, 2011
 *      Author: jtedesco
 */

#include "NetworkObject.h"

NetworkObject::NetworkObject()
{
	registerMethod("close", make_method(this, &NetworkObject::shutdown));
}
