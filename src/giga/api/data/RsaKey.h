/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#ifndef GIGA_DATA_RSAKEY_MODEL_H_
#define GIGA_DATA_RSAKEY_MODEL_H_

#include <string>
#include <vector>
#include <map>
#include "../../rest/prepoc_manage.h"

namespace giga {
namespace data {

/**
 * An rsa key pair. Private key is aes encrypted.
 */
struct RsaKey {
    utility::string_t publicKey   = {};
    utility::string_t privateKey  = {};
    utility::string_t aesIv       = {};
    utility::string_t aesSalt     = {};
    
    
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
