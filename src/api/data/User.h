/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#ifndef GIGA_DATA_USER_MODEL_H_
#define GIGA_DATA_USER_MODEL_H_

#include <string>
#include <vector>
#include <map>
#include "prepoc_manage.h"

#include "Node.h"
#include "RsaKey.h"
#include "ExternalAccount.h"
#include "Offer.h"

namespace giga {
namespace data {

/**
 * The user has private (for him only) properties, protected (available to his contacts) and public properties.
 */
struct User {
    int64_t id = 0;
    std::string login = {};
    /** unix timestamp in seconds */
    int64_t creationDate = 0;
    /** unix timestamp in seconds */
    boost::optional<int64_t> lastConnectionDate = boost::none;
    std::string activity = {};
    boost::optional<std::string> gender = boost::none;
    /** Y-m-d ex: 1960-01-26 */
    boost::optional<std::string> birthDate = boost::none;
    boost::optional<std::string> description = boost::none;
    std::string avatarUrl = {};
    std::string bigAvatarUrl = {};
    int64_t contactCount = 0;
    boost::optional<int64_t> maxContact = boost::none;
    std::vector<std::string> tags = {};
    std::string isSeeder = {};
    /** True if the user is a GiGa bot */
    boost::optional<bool> isBot = boost::none;
    /** The rsa public key of this user */
    boost::optional<std::string> publicKey = boost::none;
    /** DEPRECATED: the country */
    boost::optional<std::string> country = boost::none;
    /** DEPRECATED: The interface languge for this user */
    boost::optional<std::string> language = boost::none;
    /** Protected. The user full name */
    boost::optional<std::string> name = boost::none;
    /** Private. The user root folder */
    std::unique_ptr<Node> node = nullptr;
    /** Private. */
    boost::optional<std::string> reportedState = boost::none;
    /** Private. */
    boost::optional<std::string> nodeKey = boost::none;
    /** Private. */
    std::unique_ptr<RsaKey> rsaKeys = nullptr;
    /** Private. */
    boost::optional<std::string> salt = boost::none;
    /** Private. */
    boost::optional<std::string> email = boost::none;
    /** Private. The email when not validated yet */
    boost::optional<std::string> nextEmail = boost::none;
    /** Private. Value in Gio */
    boost::optional<int64_t> maxStorage = boost::none;
    /** Private. Value in Gio */
    boost::optional<int64_t> standatdMaxStorage = boost::none;
    /** Private. bitfield */
    boost::optional<int64_t> achievements = boost::none;
    /** Private. Associeted facebook/gigatribe accounts */
    std::vector<std::unique_ptr<ExternalAccount>> externalAccounts = {};
    /** Private. Value in octets */
    boost::optional<int64_t> downloaded = boost::none;
    /** Private. */
    std::unique_ptr<Offer> offer = nullptr;
    /** Private. Value in Octets */
    boost::optional<int64_t> dlAvailable = boost::none;
    /** Private */
    boost::optional<int64_t> starCount = boost::none;
    
    template <class Manager>
    void visit(const Manager& m) {
        GIGA_MANAGE(m, id);
        GIGA_MANAGE(m, login);
        GIGA_MANAGE(m, creationDate);
        GIGA_MANAGE(m, lastConnectionDate);
        GIGA_MANAGE(m, activity);
        GIGA_MANAGE(m, gender);
        GIGA_MANAGE(m, birthDate);
        GIGA_MANAGE(m, description);
        GIGA_MANAGE(m, avatarUrl);
        GIGA_MANAGE(m, bigAvatarUrl);
        GIGA_MANAGE(m, contactCount);
        GIGA_MANAGE(m, maxContact);
        GIGA_MANAGE(m, tags);
        GIGA_MANAGE(m, isSeeder);
        GIGA_MANAGE(m, isBot);
        GIGA_MANAGE(m, publicKey);
        GIGA_MANAGE(m, country);
        GIGA_MANAGE(m, language);
        GIGA_MANAGE(m, name);
        GIGA_MANAGE(m, node);
        GIGA_MANAGE(m, reportedState);
        GIGA_MANAGE(m, nodeKey);
        GIGA_MANAGE(m, rsaKeys);
        GIGA_MANAGE(m, salt);
        GIGA_MANAGE(m, email);
        GIGA_MANAGE(m, nextEmail);
        GIGA_MANAGE(m, maxStorage);
        GIGA_MANAGE(m, standatdMaxStorage);
        GIGA_MANAGE(m, achievements);
        GIGA_MANAGE(m, externalAccounts);
        GIGA_MANAGE(m, downloaded);
        GIGA_MANAGE(m, offer);
        GIGA_MANAGE(m, dlAvailable);
        GIGA_MANAGE(m, starCount);
    }
};

} /* namespace data */
} /* namespace giga */
#endif /* GIGA_DATA_USER_MODEL_H_ */
