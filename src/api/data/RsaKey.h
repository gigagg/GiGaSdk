/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#ifndef GIGA_DATA_RSAKEY_MODEL_H_
#define GIGA_DATA_RSAKEY_MODEL_H_

#include <string>
#include <vector>
#include <map>
#include "prepoc_manage.h"

namespace giga {
namespace data {

/**
 * An rsa key pair. Private key is aes encrypted.
 */
struct RsaKey {
    std::string publicKey   = {};
    std::string privateKey  = {};
    std::string aesIv       = {};
    std::string aesSalt     = {};
    
    
    template <class Manager>
    void visit(const Manager& m) {
        GIGA_MANAGE(m, publicKey);
        GIGA_MANAGE(m, privateKey);
        GIGA_MANAGE(m, aesIv);
        GIGA_MANAGE(m, aesSalt);
    }
};

} /* namespace data */
} /* namespace giga */
#endif /* GIGA_DATA_RSAKEY_MODEL_H_ */
