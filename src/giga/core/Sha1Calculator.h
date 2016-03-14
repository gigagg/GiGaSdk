/*
 * Sha1Calculator.h
 *
 *  Created on: 14 mars 2016
 *      Author: thomas
 */

#ifndef GIGA_CORE_SHA1CALCULATOR_H_
#define GIGA_CORE_SHA1CALCULATOR_H_

#include "FileTransferer.h"

#include <cpprest/details/basic_types.h>
#include <pplx/pplxtasks.h>
#include <fstream>
#include <memory>

namespace giga
{
namespace core
{

class Sha1Calculator : public FileTransferer
{
public:
    explicit
    Sha1Calculator(const utility::string_t& filename);
    ~Sha1Calculator();

    Sha1Calculator ()                                = delete;
    Sha1Calculator (Sha1Calculator&&)                = delete;
    Sha1Calculator& operator=(const Sha1Calculator&) = delete;
    Sha1Calculator (const Sha1Calculator&)           = delete;

public:
    /**
     * @brief Gets the task managing the sha1 calculation process.
     * Make sure you have started the sha1 calculation before calling this method.
     * @see FileTransferer::start()
     * @see https://github.com/Microsoft/cpprestsdk
     * @see http://microsoft.github.io/cpprestsdk/classpplx_1_1task.html
     */
    const pplx::task<std::string>&
    task () const;

    /**
     * @brief Gets the progress of the sha1 calculation.
     */
    FileTransferer::Progress
    progress () const;

    const utility::string_t&
    fileName() const;

protected:
    void
    doStart () override;

private:
    utility::string_t       _filename;
    pplx::task<std::string> _task;
    std::ifstream           _is;
    std::unique_ptr<char[]> _buf;
    uint64_t                _fileSize;

};

} /* namespace core */
} /* namespace giga */

#endif /* GIGA_CORE_SHA1CALCULATOR_H_ */
