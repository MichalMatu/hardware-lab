#include "FactoryReset.h"

// Application hook used by the ESP32-SvelteKit framework.
// When the UI calls POST /rest/factoryReset, the framework FactoryResetService
// will delegate here (if present) instead of only deleting config files.
extern "C" void svk_appFactoryReset() {
    System::performFactoryReset();
}
