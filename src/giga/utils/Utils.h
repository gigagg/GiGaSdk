/*
 * Utils.h
 *
 *  Created on: 11 févr. 2016
 *      Author: thomas
 */

#ifndef GIGA_UTILS_UTILS_H_
#define GIGA_UTILS_UTILS_H_

#include <pplx/pplxtasks.h>
#include <string>

namespace giga
{
namespace utils
{

std::string
httpsPrefix(const std::string& url);

std::string
cleanUpFilename(std::string name);

template <typename T> void
waitTasks(std::initializer_list<T> tasks)
{
    pplx::when_all(tasks.begin(), tasks.end()).wait();
}

} /* namespace utils */
} /* namespace giga */

#endif /* GIGA_UTILS_UTILS_H_ */
