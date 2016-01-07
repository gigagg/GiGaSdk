/*
 * Empty.h
 *
 *  Created on: 6 janv. 2016
 *      Author: thomas
 */

#ifndef REST_EMPTY_H_
#define REST_EMPTY_H_

namespace giga {

struct Empty {
    template <class Manager>
    void visit(const Manager&) {}
};

} /* namespace giga */



#endif /* REST_EMPTY_H_ */
