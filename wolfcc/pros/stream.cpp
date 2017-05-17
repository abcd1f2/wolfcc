#include "pros/stream.h"
#include "pros/procpackfactory.h"
#include "utils/logging.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

size_t GetMaxPacketSize(int protocol)
{
	return 0;
}

int ParseProtocol(const char* str)
{
	if (strcmp(str, "TEXT_STREAM_PROTOCOL") == 0) return TEXT_STREAM_PROTOCOL;
	if (strcmp(str, "BINARY_STREAM_PROTOCOL") == 0) return BINARY_STREAM_PROTOCOL;
	if (strcmp(str, "TEXT_STREAM_PROTOCOL_2") == 0) return TEXT_STREAM_PROTOCOL_2;
	if (strcmp(str, "BINARY_STREAM_PROTOCOL_2") == 0) return BINARY_STREAM_PROTOCOL_2;
	if (strcmp(str, "SERIALNO_WRAPPER_PROTOCOL") == 0) return SERIALNO_WRAPPER_PROTOCOL;
	if (strcmp(str, "XCODE_PROTOCOL") == 0) return XCODE_PROTOCOL;
	if (strcmp(str, "FAKE_HTTP_PROTOCOL") == 0) return FAKE_HTTP_PROTOCOL;
	if (strcmp(str, "FAKE_HTTP_PROTOCOL_2") == 0) return FAKE_HTTP_PROTOCOL_2;
	if (strcmp(str, "FAKE_HTTP_PROTOCOL_4") == 0) return FAKE_HTTP_PROTOCOL_4;
	if (strcmp(str, "FAKE_HTTP_INCLUDE_PROTOCOL_2") == 0) return FAKE_HTTP_INCLUDE_PROTOCOL_2;
	if (strcmp(str, "FAKE_HTTP_INCLUDE_PROTOCOL_4") == 0) return FAKE_HTTP_INCLUDE_PROTOCOL_4;
	log(LOG_ERR, "Unknown protocol: %s", str);
	return -1;
}

bool ReadStream::Read(double& f)
{
	char buffer[32];
	size_t size;
	if (!Read(buffer, sizeof(buffer)-1, size)) {
		return false;
	}
	buffer[size] = '\0';
	f = atof(buffer);
	return true;
}

bool WriteStream::Write(const double& f)
{
	char buffer[128];
	snprintf(buffer, sizeof(buffer), "%.15lf", f);
	return Write(buffer, strlen(buffer));
}
