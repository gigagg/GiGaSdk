#include <boost/exception/diagnostic_information.hpp>
#include <giga/Application.h>
#include <giga/core/Uploader.h>
#include <cpprest/details/basic_types.h>
#include <boost/algorithm/string/predicate.hpp>
#include <giga/utils/Utils.h>


using giga::Config;
using giga::Application;
using giga::core::Uploader;
using giga::core::Node;
using giga::core::FolderNode;
using boost::filesystem::path;
using giga::utils::str2wstr;

using utility::string_t;

class Parameters
{
public:
    bool
    add(utility::string_t key, utility::string_t value)
    {
        if (!exists(key))
        {
            _kvs.emplace_back(std::make_pair(std::move(key), std::move(value)));
            return true;
        }
        return false;
    }
    bool
    add(utility::string_t key, int64_t value)
    {
        if (!exists(key))
        {
            _kvi.emplace_back(std::make_pair(std::move(key), std::move(value)));
            return true;
        }
        return false;
    }

    void
    rawAdd(const web::json::value& val, const utility::string_t& key, bool isInt)
    {
        if(val.has_field(key)) {
            if(isInt) {
                add(key, val.at(key).as_integer());
            } else {
                add(key, giga::utils::wstr2str(val.at(key).as_string()));
            }
        }
    }

    utility::string_t
    to_json() const {
        auto v = web::json::value::object(false);
        for(const auto& p : _kvs) {
            v[p.first] = web::json::value::string(p.second);
        }
        for(const auto& p : _kvi) {
            v[p.first] = web::json::value::number(p.second);
        }
        return v.serialize();
    }

    int64_t&
    findInt(utility::string_t key)
    {
        auto it = std::find_if(_kvi.begin(), _kvi.end(), [&key](const std::pair<utility::string_t, int64_t>& v){
           return v.first == key;
        });
        if (it != _kvi.end())
        {
           return it->second;
        }
        throw new std::runtime_error("not found");
    }

    const utility::string_t&
    findString(utility::string_t key) const
    {
        auto it = std::find_if(_kvs.begin(), _kvs.end(), [&key](const std::pair<utility::string_t, utility::string_t>& v){
           return v.first == key;
        });
        if (it != _kvs.end())
        {
           return it->second;
        }
        throw new std::runtime_error("not found");
    }

    bool
    exists(utility::string_t key) const
    {
        auto it = std::find_if(_kvs.begin(), _kvs.end(), [&key](const std::pair<utility::string_t, utility::string_t>& v){
            return v.first == key;
        });
        if (it != _kvs.end())
        {
            return true;
        }
        auto it2 = std::find_if(_kvi.begin(), _kvi.end(), [&key](const std::pair<utility::string_t, int64_t>& v){
            return v.first == key;
        });
        return it2 != _kvi.end();
    }


private:
    std::vector<std::pair<utility::string_t, utility::string_t>> _kvs;
    std::vector<std::pair<utility::string_t, int64_t>>           _kvi;
};

void replace_all(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

void
apiGetInfo(Parameters& param)
{

    std::string from{"'"};
    std::string to{"'\"'\"'"};
    auto name = param.findString(U("name"));
    replace_all(name, from, to);
    if (std::system(("guessit -j '"+ name +" ' > /tmp/result.json").c_str()) != 0)
    {
       return;
    }
    std::ifstream stream("/tmp/result.json");
    auto value = web::json::value::parse(stream);
    if (!value.has_field("title"))
    {
       return;
    }

    param.rawAdd(value, U("title"), false);
    param.rawAdd(value, U("season"), true);
    param.rawAdd(value, U("episode"), true);
    param.rawAdd(value, U("year"), true);
    param.rawAdd(value, U("episode_title"), false);
    param.rawAdd(value, U("format"), false);
    param.rawAdd(value, U("video_codec"), false);
    param.rawAdd(value, U("release_group"), false);
    param.rawAdd(value, U("type"), false);


    if (param.exists(U("episode")) && param.exists(U("season"))) {
        std::cout << "S" << std::setfill('0') << std::setw(2) << param.findInt(U("season"))
                  << "E" << std::setfill('0') << std::setw(2) << param.findInt(U("episode")) << " - ";
    }

    std::cout << param.findString(U("title")) << std::endl;

    web::uri_builder uri{U("http://www.omdbapi.com/?plot=short&r=json")};
    if (param.exists(U("episode")) && param.exists(U("season")))
    {
        uri.append_query(U("&type=episode"));
        uri.append_query(U("&season=") + str2wstr(std::to_string(param.findInt("season"))));
        uri.append_query(U("&episode=") + str2wstr(std::to_string(param.findInt("episode"))));
    }
    else if (param.exists(U("episode")) || param.exists(U("season")))
    {
        uri.append_query(U("&type=series"));
    }
    if (param.exists(U("year"))) {
        uri.append_query(U("&y=") + str2wstr(std::to_string(param.findInt("year"))));
    }
    uri.append_query(U("&t=") + param.findString(U("title")), true);

    std::cout << uri.to_string() << std::endl;

    web::http::client::http_client client{uri.to_uri()};
    auto response = client.request(web::http::methods::GET).get();
    auto json = response.extract_json(true).get();
    for(const auto& val : json.as_object())
    {
        param.rawAdd(json, val.first, false);
    }

    std::cout << param.to_json() << std::endl;
}

void
retrieveMoviesFrom(Application& app, std::string nodeId, std::vector<Parameters>& params) {
    app.api().nodes.forEachFiles(nodeId, U("video"), [&params](web::json::value&& value) {
            Parameters p;
            p.rawAdd(value, "id", false);
            p.rawAdd(value, "name", false);
            p.rawAdd(value, "mimeType", false);
            p.rawAdd(value, "fid", false);
//            p.rawAdd(value, "url", false);
            p.rawAdd(value, "ownerId", true);
            p.rawAdd(value, "previewState", true);
            p.rawAdd(value, "size", true);

            if (value.has_field(U("meta"))) {
                auto meta = value.at(U("meta"));
                p.rawAdd(meta, "vcodec", false);
                p.rawAdd(meta, "width", true);
                p.rawAdd(meta, "height", true);
                p.rawAdd(meta, "rotate", true);
                p.rawAdd(meta, "duration", true);
                p.rawAdd(meta, "acodec", false);
                p.rawAdd(meta, "lang", false);
            }

            params.emplace_back(std::move(p));
        }).get();
}

int main(int, char**)
{
    try
    {
        Config::init(string_t(U("http://localhost:5001")),
                        string_t(U("1142f21cf897")),
                        string_t(U("65934eaddb0b233dddc3e85f941bc27e")));

        Application app;
        auto owner = app.authenticate(U("thomas"), U("ufbth4d"));
        ucout << U("Hello ") << owner.login() << U(" your id is ") << owner.id() << std::endl;

        std::vector<Parameters> params;
        std::cout << "getNodes from current user" << std::endl;
        retrieveMoviesFrom(app, app.mutableCurrentUser().contactData().node().id(), params);

        for(auto& contact: app.getContacts()) {
            for(const auto& node : contact.contactData().node().getChildren())
            {
                std::cout << "getNodes from " << node->id() << " owner: " << node->ownerId() << std::endl;
                retrieveMoviesFrom(app, node->id(), params);
            }
        }


        std::sort(params.begin(), params.end(), [](const Parameters& rhs, const Parameters& lhs) {
           return rhs.findString("fid") < lhs.findString("fid");
        });
        auto it = std::unique(params.begin(), params.end(), [](const Parameters& rhs, const Parameters& lhs) {
            return rhs.findString("fid") == lhs.findString("fid");
        });
        params.resize(std::distance(params.begin(), it));


        for(auto& param : params)
        {
            apiGetInfo(param);
        }
        return 0;
    }
    catch (...)
    {
        std::cerr << "Unhandled exception!" << std::endl <<
        boost::current_exception_diagnostic_information();
        return 1;
    }
}

// TODO !! -> When there is a crypto exception we need to be able to continue.
// TODO !! -> On timeout retry the http request.
