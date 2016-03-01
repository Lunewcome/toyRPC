#include "common/registerer.h"

namespace registerer {

BaseClassMap& global_factory_map() {
  static BaseClassMap *factory_map = new BaseClassMap();
  return *factory_map;
}

}  // namespace registerer
