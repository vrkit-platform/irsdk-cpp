#include <IRacingSDK/Utils/Tracing.h>

TRACELOGGING_DEFINE_PROVIDER(
    gTracingProvider,                     // Handle to the provider
    "IRSDK.CPP",                   // Unique provider name
    // GUID generated uniquely for this provider (use a GUID generator tool like guidgen)
    // 6dbc9382-5c73-4bf0-9c01-6bf49f7ebe2a
    (0x6dbc9382, 0x5c73, 0x4bf0, 0x9c, 0x01, 0x6b, 0xf4, 0x9f, 0x7e, 0xbe, 0x2a)
);
