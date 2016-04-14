/*
 * TransferProgress.h
 *
 *  Created on: 12 avr. 2016
 *      Author: thomas
 */

#ifndef GIGA_CORE_TRANSFERPROGRESS_H_
#define GIGA_CORE_TRANSFERPROGRESS_H_

#include "../rest/prepoc_manage.h"

namespace giga
{
namespace core
{

struct TransferProgress
{
    uint64_t fileCount       = 0ul;
    uint64_t fileDone        = 0ul;
    uint64_t bytesTotal      = 0ul;
    uint64_t bytesTransfered = 0ul;

    TransferProgress
    getProgressAddByte(uint64_t bytesTransfered) const
    {
        TransferProgress p{*this};
        p.bytesTransfered += bytesTransfered;
        return p;
    }

    template <class Manager>
    void visit(const Manager& m) {
        GIGA_MANAGE(m, fileCount);
        GIGA_MANAGE(m, fileDone);
        GIGA_MANAGE(m, bytesTotal);
        GIGA_MANAGE(m, bytesTransfered);
    }
};

} /* namespace core */
} /* namespace giga */

#endif /* GIGA_CORE_TRANSFERPROGRESS_H_ */
