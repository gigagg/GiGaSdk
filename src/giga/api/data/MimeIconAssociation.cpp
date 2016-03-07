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

#include "MimeIconAssociation.h"
#include "../../utils/Utils.h"

#include <cpprest/details/basic_types.h>
#include <string>

namespace giga
{
namespace data
{

std::unordered_map<std::string, std::string> MimeIconAssociation::_data =
{
    {"application-vnd.ms-access", "database"},
    {"application-vnd.oasis.opendocument.database", "database"},
    {"application-x-sqlite2", "database"},
    {"application-x-sqlite3", "database"},
    {"openofficeorg3-database", "database"},
    {"openofficeorg3-oasis-database", "database"},
    {"text-x-sql", "database"},

    {"application-x-tar", "tar"},
    {"folder_tar", "tar"},
    {"gnome-mime-application-x-tar", "tar"},
    {"tar", "tar"},

    {"application-x-msdownload", "exe"},
    {"gnome-exe-thumbnailer-generic", "exe"},
    {"gnome-exe-thumbnailer-template", "exe"},
    {"application-x-ms-dos-executable", "exe"},
    {"application-x-dosexec", "exe"},
    {"gnome-exe-thumbnailer-generic-x", "exe"},
    {"gnome-mime-application-x-ms-dos-executable", "exe"},

    {"application-illustrator", "vectoriel"},
    {"application-vnd.oasis.opendocument.graphics", "vectoriel"},
    {"application-vnd.sun.xml.draw", "vectoriel"},
    {"gnome-mime-application-vnd.oasis.opendocument.graphics", "vectoriel"},
    {"gnome-mime-application-vnd.oasis.opendocument.image", "vectoriel"},
    {"gnome-mime-application-vnd.sun.xml.draw", "vectoriel"},
    {"image-svg+xml", "vectoriel"},
    {"image-x-svg+xml", "vectoriel"},
    {"office-illustration", "vectoriel"},
    {"openofficeorg3-drawing", "vectoriel"},
    {"svg", "vectoriel"},
    {"vnd.oasis.opendocument.drawing", "vectoriel"},
    {"x-office-drawing", "vectoriel"},

    {"application-pdf", "pdf"},
    {"gnome-mime-application-pdf", "pdf"},
    {"image-vnd.djvu", "pdf"},
    {"pdf", "pdf"},

    {"application-x-flash-video", "flash"},
    {"application-x-shockwave-flash", "flash"},
    {"gnome-mime-application-x-shockwave-flash", "flash"},

    {"application-x-rpm", "rpm"},
    {"gnome-mime-application-x-rpm", "rpm"},
    {"rpm", "rpm"},

    {"application-mbox", "message"},
    {"message", "message"},
    {"message-rfc822", "message"},

    {"font", "font"},
    {"font_bitmap", "font"},
    {"font_truetype", "font"},
    {"font_type1", "font"},
    {"font-x-generic", "font"},
    {"gnome-mime-application-x-font-afm", "font"},
    {"gnome-mime-application-x-font-bdf", "font"},
    {"gnome-mime-application-x-font-linux-psf", "font"},
    {"gnome-mime-application-x-font-pcf", "font"},
    {"gnome-mime-application-x-font-sunos-news", "font"},
    {"gnome-mime-application-x-font-ttf", "font"},
    {"gnome-mime-x-font-afm", "font"},

    {"application-x-arc", "archive"},
    {"application-x-archive", "archive"},
    {"application-x-compress", "archive"},
    {"application-x-lha", "archive"},
    {"application-x-lhz", "archive"},
    {"gnome-mime-application-x-archive", "archive"},
    {"gnome-mime-application-x-arj", "archive"},
    {"gnome-mime-application-x-bzip", "archive"},
    {"gnome-mime-application-x-bzip-compressed", "archive"},
    {"gnome-mime-application-x-bzip-compressed-tar", "archive"},
    {"gnome-mime-application-x-compress", "archive"},
    {"gnome-mime-application-x-cpio", "archive"},
    {"gnome-mime-application-x-cpio-compressed", "archive"},
    {"gnome-mime-application-x-lha", "archive"},
    {"gnome-mime-application-x-lhz", "archive"},
    {"gnome-mime-application-x-stuffit", "archive"},
    {"gnome-mime-application-x-tarz", "archive"},
    {"application-7zip", "archive"},
    {"application-x-7z-compressed", "archive"},
    {"application-x-7zip", "archive"},
    {"gnome-mime-application-x-7z-compressed", "archive"},
    {"gnome-mime-application-x-7zip", "archive"},
    {"application-x-compressed-tar", "archive"},
    {"application-x-gzip", "archive"},
    {"gnome-mime-application-x-compressed-tar", "archive"},
    {"gnome-mime-application-x-gzip", "archive"},
    {"application-x-ace", "archive"},
    {"gnome-mime-application-x-ace", "archive"},
    {"application-x-rar", "archive"},
    {"gnome-mime-application-x-rar", "archive"},

    {"application-x-deb", "deb"},
    {"deb", "deb"},
    {"gnome-mime-application-x-deb", "deb"},

    {"application-x-bittorrent", "torrent"},
    {"gnome-mime-application-x-bittorrent", "torrent"},

    {"application-vnd.scribus", "scribus"},
    {"application-x-scribus", "scribus"},
    {"gnome-mime-application-vnd.scribus", "scribus"},
    {"gnome-mime-application-x-scribus", "scribus"},

    {"application-vnd.rn-realmedia", "videos"},
    {"application-x-matroska", "videos"},
    {"gnome-mime-application-vnd.rn-realmedia", "videos"},
    {"gnome-mime-application-vnd.rn-realmedia-secure", "videos"},
    {"gnome-mime-application-vnd.rn-realmedia-vbr", "videos"},
    {"gnome-mime-video", "videos"},
    {"gnome-mime-video-x-ms-asf", "videos"},
    {"gnome-mime-video-x-ms-wmv", "videos"},
    {"media-video", "videos"},
    {"video", "videos"},
    {"video-x-generic", "videos"},

    {"application-x-jar", "jar"},
    {"application-x-java-archive", "jar"},
    {"gnome-mime-application-x-jar", "jar"},
    {"gnome-mime-application-x-java-archive", "jar"},

    {"application-x-cd-image", "cdimage"},
    {"application-x-cue", "cdimage"},

    {"audio-x-generic", "music"},
    {"gnome-mime-audio", "music"},
    {"media-audio", "music"},
    {"sound", "music"},
    {"audio-x-ms-wma", "music"},
    {"audio-x-mp3-playlist", "music"},
    {"audio-x-mpegurl", "music"},
    {"audio-x-scpls", "music"},
    {"playlist", "music"},
    {"source-playlist", "music"},
    {"audio-x-adpcm", "music"},
    {"audio-x-wav", "music"},
    {"audio-mpeg", "music"},
    {"audio-x-mpeg", "music"},
    {"application-ogg", "music"},
    {"audio-x-flac+ogg", "music"},
    {"audio-x-speex+ogg", "music"},
    {"audio-x-vorbis+ogg", "music"},
    {"gnome-mime-application-ogg", "music"},

    {"gnome-mime-image-vnd.adobe.photoshop", "psd"},
    {"gnome-mime-image-x-psd", "psd"},
    {"image-x-psd", "psd"},
    {"image-x-psdimage-x-psd", "psd"},

    {"application-x-class-file", "java"},
    {"application-x-java", "java"},
    {"gnome-mime-application-x-class-file", "java"},
    {"gnome-mime-application-x-java", "java"},
    {"gnome-mime-text-x-java", "java"},
    {"gnome-mime-text-x-java-source", "java"},
    {"text-x-java", "java"},
    {"text-x-java-source", "java"},

    {"application-x-zip", "zip"},
    {"application-zip", "zip"},
    {"gnome-mime-application-x-zip", "zip"},
    {"gnome-mime-application-zip", "zip"},
    {"zip", "zip"},

    {"gnome-mime-application-x-killustrator", "image"},
    {"gnome-mime-image", "image"},
    {"gnome-mime-image-x-cmu-raster", "image"},
    {"gnome-mime-image-x-xpixmap", "image"},
    {"image", "image"},
    {"image-x-generic", "image"},
    {"image-x-tga", "image"},
    {"media-image", "image"},

    {"gnome-mime-image-tiff", "tiff"},
    {"image-tiff", "tiff"},

    {"gnome-mime-image-gif", "gif"},
    {"image-gif", "gif"},

    {"gnome-mime-image-png", "png"},
    {"image-png", "png"},

    {"gnome-mime-image-jpeg", "jpg"},
    {"image-jpeg", "jpg"},
    {"image-jpeg2000", "jpg"},
    {"jpeg", "jpg"},
    {"jpg", "jpg"},

    {"application-vnd.oasis.opendocument.spreadsheet", "spreadsheet"},
    {"application-vnd.sun.xml.calc", "spreadsheet"},
    {"application-x-gnumeric", "spreadsheet"},
    {"gnome-mime-application-vnd.lotus-1-2-3", "spreadsheet"},
    {"gnome-mime-application-vnd.oasis.opendocument.spreadsheet", "spreadsheet"},
    {"gnome-mime-application-vnd.stardivision.calc", "spreadsheet"},
    {"gnome-mime-application-vnd.sun.xml.calc", "spreadsheet"},
    {"gnome-mime-application-x-applix-spreadsheet", "spreadsheet"},
    {"gnome-mime-application-x-gnumeric", "spreadsheet"},
    {"gnome-mime-application-x-kspread", "spreadsheet"},
    {"office-spreadsheet", "spreadsheet"},
    {"openofficeorg3-oasis-spreadsheet", "spreadsheet"},
    {"spreadsheet", "spreadsheet"},
    {"text-spreadsheet", "spreadsheet"},
    {"x-office-spreadsheet", "spreadsheet"},
    {"application-vnd.ms-excel", "spreadsheet"},
    {"application-vnd.openxmlformats-officedocument.spreadsheetml.sheet", "spreadsheet"},
    {"gnome-mime-application-vnd.ms-excel", "spreadsheet"},
    {"gnome-mime-application-vnd.openxmlformats-officedocument.spreadsheetml.sheet", "spreadsheet"},

    {"application-octet-stream", "octet"},
    {"application-x-ole-storage", "octet"},
    {"multipart-encrypted", "octet"},

    {"application-x-executable", "exec"},
    {"binary", "exec"},
    {"exec", "exec"},
    {"gnome-fs-executable", "exec"},
    {"gnome-mime-application-x-executable", "exec"},

    {"gnome-mime-application-magicpoint", "presentation"},
    {"gnome-mime-application-vnd.oasis.opendocument.presentation", "presentation"},
    {"gnome-mime-application-vnd.stardivision.impress", "presentation"},
    {"gnome-mime-application-vnd.sun.xml.impress", "presentation"},
    {"gnome-mime-application-x-kpresenter", "presentation"},
    {"kpresenter_kpr", "presentation"},
    {"x-office-presentation", "presentation"},
    {"application-vnd.ms-powerpoint", "presentation"},
    {"application-vnd.openxmlformats-officedocument.presentationml.presentation", "presentation"},
    {"application-vnd.openxmlformats-officedocument.presentationml.slideshow", "presentation"},
    {"gnome-mime-application-vnd.openxmlformats-officedocument.presentationml.presentation", "presentation"},
    {"gnome-mime-application-vnd.openxmlformats-officedocument.presentationml.slideshow", "presentation"},

    {"application-text", "txt"},
    {"gnome-mime-text", "txt"},
    {"gnome-mime-text-plain", "txt"},
    {"misc", "txt"},
    {"package_editors", "txt"},
    {"text-plain", "txt"},
    {"text-x-generic", "txt"},
    {"txt", "txt"},
    {"txt2", "txt"},
    {"application-rtf", "txt"},
    {"application-x-m4", "txt"},
    {"document", "txt"},
    {"gnome-mime-application-postscript", "txt"},
    {"gnome-mime-application-rtf", "txt"},
    {"gnome-mime-application-x-gzpostscript", "txt"},
    {"text-enriched", "txt"},
    {"text-richtext", "txt"},

    {"application-msword", "document"},
    {"application-vnd.openxmlformats-officedocument.wordprocessingml.document", "document"},
    {"gnome-mime-application-msword", "document"},
    {"gnome-mime-application-vnd.openxmlformats-officedocument.wordprocessingml.document", "document"},
    {"application-vnd.oasis.opendocument.text", "document"},
    {"application-vnd.sun.xml.writer", "document"},
    {"application-vnd.wordperfect", "document"},
    {"gnome-mime-application-vnd.oasis.opendocument.text", "document"},
    {"gnome-mime-application-vnd.stardivision.writer", "document"},
    {"gnome-mime-application-vnd.sun.xml.writer", "document"},
    {"gnome-mime-application-wordperfect", "document"},
    {"gnome-mime-application-x-abiword", "document"},
    {"gnome-mime-application-x-applix-word", "document"},
    {"gnome-mime-application-x-dvi", "document"},
    {"gnome-mime-application-x-kword", "document"},
    {"office-document", "document"},
    {"package_wordprocessing", "document"},
    {"wordprocessing", "document"},
    {"x-office-document", "document"},
};


web::uri
MimeIconAssociation::icon(const std::string& mimeType, const boost::filesystem::path& filename)
{
    return U("/public/assets/img/48/") + utils::str2wstr(name(mimeType, filename)) + U(".png");
}

web::uri
MimeIconAssociation::bigIcon(const std::string& mimeType, const boost::filesystem::path& filename)
{
    return U("/public/assets/img/255/") + utils::str2wstr(name(mimeType, filename)) + U(".png");
}

std::string
MimeIconAssociation::name(const std::string& mimeType, const boost::filesystem::path& filename)
{
    auto extension = filename.extension();
    if (mimeType == "application/zip" || mimeType == "application/vnd.ms-office")
    {
      if (extension == "xlsx" || extension == "xltx") {
          return "spreadsheet";
      }
      if (extension == "ppsx" || extension == "potx" || extension == "pptx" || extension == "sldx") {
          return "presentation";
      }
      if (extension == "docx" || extension == "dotx") {
          return "document";
      }
    }
    auto iconName = mimeType;
    iconName.replace(iconName.begin(), iconName.end(), "/", "-");
    {
        const auto it = _data.find(iconName);
        if (it != _data.end())
        {
            return it->second;
        }
    }
    {
        iconName = "gnome-mime-" + iconName;
        const auto it = _data.find(iconName);
        if (it != _data.end())
        {
            return it->second;
        }
    }

    auto video = std::string("video");
    auto audio = std::string("audio");
    if (video.compare(0, video.size(), mimeType) == 0) {
      return "videos";
    }
    if (audio.compare(0, audio.size(), mimeType) == 0) {
      return "music";
    }
    return "none";
}

} /* namespace data */
} /* namespace giga */
