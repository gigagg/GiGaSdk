/*
 * Copyright 2016 Gigatribe
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "FileNode.h"
#include "FolderNode.h"
#include "../Application.h"
#include "../api/data/Node.h"
#include "../api/data/MimeIconAssociation.h"
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

FileNodeData::FileNodeData(std::shared_ptr<data::Node> n, const Application& app) :
        n{n}, _app(&app)
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

const std::string&
FileNodeData::fid () const
{
    return n->fid.get();
}

FileNodeData::PreviewState
FileNodeData::previewState () const
{
    return static_cast<PreviewState>(n->previewState.get());
}

uri
FileNodeData::iconUrl () const
{
    if (n->icon.is_initialized())
    {
        return uri{utils::httpsPrefix(n->icon.get())};
    }
    return data::MimeIconAssociation::icon(utils::wstr2str(mimeType()), n->name);
}

uri
FileNodeData::squareUrl () const
{
    if (n->square.is_initialized())
    {
        return uri{utils::httpsPrefix(n->square.get())};
    }
    return data::MimeIconAssociation::bigIcon(utils::wstr2str(mimeType()), n->name);
}

uri
FileNodeData::originalUrl () const
{
    if (n->original.is_initialized())
    {
        return uri{utils::httpsPrefix(n->original.get())};
    }
    return data::MimeIconAssociation::bigIcon(utils::wstr2str(mimeType()), n->name);
}

uri
FileNodeData::posterUrl () const
{
    if (n->poster.is_initialized())
    {
        return uri{utils::httpsPrefix(n->poster.get())};
    }
    return data::MimeIconAssociation::bigIcon(utils::wstr2str(mimeType()), n->name);
}

uri
FileNodeData::fileUrl () const
{
    if (_app == nullptr)
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("Application is null")});
    }
    auto nodeKey = utils::str2wstr(_app->getNodeKeyClear(n->ownerId));
    return uri{utils::httpsPrefix(n->url.get()) + web::uri::encode_data_string(nodeKey)};
}

//
// FileNode
//


FileNode::FileNode (std::shared_ptr<data::Node> n, const Application& app) :
        Node(n, app), _fileData(n, app)
{
}

FileNode::FileNode(const FileNode& rhs) :
        Node(rhs), _fileData{rhs._data, *rhs._app}
{
}

FileNode&
FileNode::operator=(const FileNode& rhs)
{
    Node::operator =(rhs);
    _fileData = FileNodeData{rhs._data, *rhs._app};
    return *this;
}


static std::vector<std::unique_ptr<Node>> emptyVector{};

const std::vector<std::unique_ptr<Node>>&
FileNode::getChildren () const
{
    return emptyVector;
}

FolderNode&
FileNode::addChildFolder(const string_t&)
{
    BOOST_THROW_EXCEPTION(ErrorException{U("Illegal action: this is a fileNode")});
}

FolderNode
FileNode::createChildFolder(const utility::string_t&) const
{
    BOOST_THROW_EXCEPTION(ErrorException{U("Illegal action: this is a fileNode")});
}

Node::UploadingFile
FileNode::uploadFile(const utility::string_t&, pplx::cancellation_token_source)
{
    BOOST_THROW_EXCEPTION(ErrorException{U("Illegal action: this is a fileNode")});
}

FileDownloader
FileNode::download(const string_t& destinationPath, FileDownloader::Policy policy)
{
    if (_app == nullptr)
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("Application is null")});
    }
    return FileDownloader{destinationPath, *this, *_app, pplx::cancellation_token_source{}, policy};
}

const FileNodeData&
FileNode::fileData() const
{
    return _fileData;
}

} /* namespace core */
} /* namespace giga */
