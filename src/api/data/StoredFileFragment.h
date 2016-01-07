/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#ifndef GIGA_DATA_StoredFileFragment_MODEL_H_
#define GIGA_DATA_StoredFileFragment_MODEL_H_

#include <string>
#include <vector>
#include <map>
#include "prepoc_manage.h"


namespace giga {
namespace data {

/**
 * Describe a file
 */
struct StoredFileFragment {
    
    
    template <class Manager>
    void visit(const Manager& m) {
        
    }
};

} /* namespace data */
} /* namespace giga */
#endif /* GIGA_DATA_StoredFileFragment_MODEL_H_ */
