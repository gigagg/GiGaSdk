/*
 * FileNode.cpp
 *
 *  Created on: 4 févr. 2016
 *      Author: thomas
 */

#include "FileNode.h"
#include "FolderNode.h"
#include "../Application.h"
#include "../api/data/Node.h"
#include "../utils/Utils.h"

#include <cpprest/http_client.h>

using web::uri;
using utility::string_t;

namespace giga
{
namespace core
{

//
// FileNodeData
//

#define THROW_IF_NOT_INITIALIZED(name) \
    if (!n->name.is_initialized())                          \
    {                                                       \
        BOOST_THROW_EXCEPTION(ErrorException{U(#name " is not initialized")});    \
    } do {} while(0)                                        \

FileNodeData::FileNodeData(std::shared_ptr<data::Node> n) :
        n{n}
{
    THROW_IF_NOT_INITIALIZED(url);
    THROW_IF_NOT_INITIALIZED(mimeType); // application/octet-stream
    THROW_IF_NOT_INITIALIZED(fid);
    THROW_IF_NOT_INITIALIZED(previewState); // no preview
//    THROW_IF_NOT_INITIALIZED(icon);
//    THROW_IF_NOT_INITIALIZED(square);
//    THROW_IF_NOT_INITIALIZED(original);
//    THROW_IF_NOT_INITIALIZED(poster);
}

const string_t&
FileNodeData::mimeType () const
{
    return n->mimeType.get();
}

const string_t&
FileNodeData::fid () const
{
    return n->fid.get();
}

int64_t
FileNodeData::previewState () const
{
    return n->previewState.get();
}

uri
FileNodeData::iconUrl () const
{
    return uri{utils::httpsPrefix(n->icon.get())};
}

uri
FileNodeData::squareUrl () const
{
    return uri{utils::httpsPrefix(n->square.get())};
}

uri
FileNodeData::originalUrl () const
{
    return uri{utils::httpsPrefix(n->original.get())};
}

uri
FileNodeData::posterUrl () const
{
    return uri{utils::httpsPrefix(n->poster.get())};
}

uri
FileNodeData::fileUrl () const
{
    return uri{utils::httpsPrefix(n->url.get()) + web::uri::encode_data_string(Application::get().currentUser().personalData().nodeKeyClear())};
}

//
// FileNode
//


FileNode::FileNode (std::shared_ptr<data::Node> n) :
        Node(n), _fileData(n)
{
}

FileNode::FileNode(const FileNode& rhs) :
        Node(rhs), _fileData{rhs._data}
{
}

FileNode&
FileNode::operator=(const FileNode& rhs)
{
    Node::operator =(rhs);
    _fileData = FileNodeData{rhs._data};
    return *this;
}


static std::vector<std::unique_ptr<Node>> emptyVector{};

const std::vector<std::unique_ptr<Node>>&
FileNode::children () const
{
    return emptyVector;
}

FolderNode&
FileNode::addChildFolder(const string_t&)
{
    BOOST_THROW_EXCEPTION(ErrorException{U("Illegal action: this is a fileNode")});
}

pplx::task<std::shared_ptr<FileUploader>>
FileNode::uploadFile(const string_t&)
{
    BOOST_THROW_EXCEPTION(ErrorException{U("Illegal action: this is a fileNode")});
}

FileDownloader
FileNode::download(const string_t& destinationPath, FileDownloader::Policy policy)
{
    return FileDownloader{destinationPath, *this, policy};
}

const FileNodeData&
FileNode::fileData() const
{
    return _fileData;
}

} /* namespace core */
} /* namespace giga */
