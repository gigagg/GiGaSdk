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
#define BOOST_TEST_MODULE crypto
#include <boost/test/included/unit_test.hpp>
#include <cpprest/details/basic_types.h>
#include <giga/utils/Crypto.h>
#include <giga/utils/Utils.h>

using namespace boost::unit_test;
using namespace giga;
using utility::string_t;

BOOST_AUTO_TEST_CASE(test_calculate_fid)
{
    auto fid = Crypto::calculateFid("177abc9bcd3bc9785b96e06fcf63d82c58b6f8f6");
    BOOST_CHECK("MUr243SzLSVf11/c7T0SZqyf" == fid);
}

BOOST_AUTO_TEST_CASE(test_calculate_fkey)
{
    auto fkey = Crypto::calculateFkey("177abc9bcd3bc9785b96e06fcf63d82c58b6f8f6");
    BOOST_CHECK("V9leIEW8PYNMhlDuMNvkpWei" == fkey);
}

BOOST_AUTO_TEST_CASE(test_aes)
{
    auto result    = Crypto::aesEncrypt(U("password"), "data");
    auto decrypted = Crypto::aesDecrypt(U("password"),
                                        std::get<2>(result),
                                        std::get<1>(result),
                                        std::get<0>(result));

    BOOST_CHECK("data" == decrypted);
}

BOOST_AUTO_TEST_CASE(test_load_rsa_keys)
{
    auto privateKey     = std::string(R"(MIICXgIBAAKBgQCvupt+SoaJxTnyvzo7d2xtVvhe5N2qxhyW/OVNuh/OeWO2T+BZPJsh6n3+YV4mulsAxmbUvvwpL9EI3xynjw5WpSsXgNAg+s2mAbyGTAOLAlPlymBy1R57XA0DgnIfEh3mQsiWP9uaXp8V8PNkWNC6d+JtRUEImDFy+mFI5XIcQwIDAQABAoGBAKvnEiES6lVvd+Z9JovxoUG8qC68GBPsxR72uaG1h1PF78LylHb3EJK2/xWPTjj4Z/l0HDfVtHrd6dCR4UgQa2a7I1ktzc2ZmCgNiL5iqLjhoQCMTD4uzkjzdTRYiAacLo/nj7f8Q/mIC6fmqY/spDqmX2Ei8+ebEhZvoPfxUsHBAkEA43t1Xm2Eh1RAG+jvaXnRc6H75bCJHHX8X0Kr6dEamH5tejAytVFsG46Ag5qR3N2vtJ/ZTlRuRKADEJPe0FfLkwJBAMXCPXZ19M49Oxm9+paDIUge6NuenN+KntBUh4ny30pImtIRiNAsGGEm/k9Bv1091hlN1zERcPg461WUXLZVupECQQCABA0z26v2QGpninmYwO4nBaRtSSCP/LHOpgV66qrt4FAZsCAJO8374ADgjs7BHZxwqHgxBYCca/Ptk1mTo6SzAkBryeCIVei2FOfGsycHrLeSQM65De4BZeQEN16dERUlvwcr4sTIepBbO863UmXEo7SznBrjstb5ZMTYo4IZHAcRAkEA3irDWI1kVMgwr3DMtzjMTNJ2I4aRKGPFkRw2gSfZYr5H3JfJmgXnH3lN6jElvdtW6TCwflsNi/Q3951UyUHLAg==)");
    auto publicKey      = std::string(R"(MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCvupt+SoaJxTnyvzo7d2xtVvhe5N2qxhyW/OVNuh/OeWO2T+BZPJsh6n3+YV4mulsAxmbUvvwpL9EI3xynjw5WpSsXgNAg+s2mAbyGTAOLAlPlymBy1R57XA0DgnIfEh3mQsiWP9uaXp8V8PNkWNC6d+JtRUEImDFy+mFI5XIcQwIDAQAB)");

    auto r = Rsa{publicKey, privateKey};
    auto encrypted = r.encrypt("test");
    BOOST_CHECK("test" == r.decrypt(encrypted));
}

BOOST_AUTO_TEST_CASE(test_decrypt_nodekey)
{
    auto password       = U(R"(gigatribe)");
    auto passwordSalt   = R"(FLOhgPcpK+IzLOqqzUw2Dbe3o47IsDitc2DBQNiU0i8=)";
    auto privateKey     = std::string(R"(MIICXgIBAAKBgQCvupt+SoaJxTnyvzo7d2xtVvhe5N2qxhyW/OVNuh/OeWO2T+BZPJsh6n3+YV4mulsAxmbUvvwpL9EI3xynjw5WpSsXgNAg+s2mAbyGTAOLAlPlymBy1R57XA0DgnIfEh3mQsiWP9uaXp8V8PNkWNC6d+JtRUEImDFy+mFI5XIcQwIDAQABAoGBAKvnEiES6lVvd+Z9JovxoUG8qC68GBPsxR72uaG1h1PF78LylHb3EJK2/xWPTjj4Z/l0HDfVtHrd6dCR4UgQa2a7I1ktzc2ZmCgNiL5iqLjhoQCMTD4uzkjzdTRYiAacLo/nj7f8Q/mIC6fmqY/spDqmX2Ei8+ebEhZvoPfxUsHBAkEA43t1Xm2Eh1RAG+jvaXnRc6H75bCJHHX8X0Kr6dEamH5tejAytVFsG46Ag5qR3N2vtJ/ZTlRuRKADEJPe0FfLkwJBAMXCPXZ19M49Oxm9+paDIUge6NuenN+KntBUh4ny30pImtIRiNAsGGEm/k9Bv1091hlN1zERcPg461WUXLZVupECQQCABA0z26v2QGpninmYwO4nBaRtSSCP/LHOpgV66qrt4FAZsCAJO8374ADgjs7BHZxwqHgxBYCca/Ptk1mTo6SzAkBryeCIVei2FOfGsycHrLeSQM65De4BZeQEN16dERUlvwcr4sTIepBbO863UmXEo7SznBrjstb5ZMTYo4IZHAcRAkEA3irDWI1kVMgwr3DMtzjMTNJ2I4aRKGPFkRw2gSfZYr5H3JfJmgXnH3lN6jElvdtW6TCwflsNi/Q3951UyUHLAg==)");
    auto privateKeyEnc  = std::string(R"(tr0/O1OZGIHMj+k/JCRCL/5P3Q74fz2A7SM4fXvnV5GminzJg8PvYTMQ/zeWFua9gaDIixihm6DbBcVE/Ktb7cSeIR1VwzdRzTK+u7MzL+1jYEYk1FPDGZJwhNgy/yIfrgoyJp+ThpM9wMKh37eg6K69Uqscsj6MWbsx714RSmdO/5ypIcdIJom7Oc6h6oFBX0hllIQxqHI8Me1GRbi1Nbkbcis7T+ej3sgzokJVGxCG/tWs+nt/QXERf04wFaxyzrZ16NzQAincL2PNdNiTLdVFtAMAIeYyT3UBtkOHPZ7RCIt0UP9YCfZIWHkQLNfXwR7M9EtiqbuFhDpgvKMVaNnT8GDTWTZRq2vCpAVmklUvhIMz3p0Oi4z+LGLE+OHCFzrPiPH1S197Hc5Pvx71pUDap0kZrkQziinxs4TiM3JsP5K6AIDwNhK072gcV5Nyx2GME96wsiaSLmHdha0orcir+GK9SAoTbwY4mEn/3UxKJTeF//HPgoO1kB/g8FXWQ+Xbyte9XBuUJCb5Dxq8eI+9Ai0zG8f7dIPkA/NaYITH6p+RHU4nIdJQeltJGzwmKbtzL4e5s/BniN81UJ+9/3TN0/V9sORSHCvQwdj+b4JvRUtSa+If2/9vVL3hQUU/YA+M1GVinyS09dg68YHTjeZzGVZfoGCyo18iOm/0GcmrJex8AXeBOGuek7DSXoGukxNIS0fZC5AZ/LcZUPWr/ReEmjemDheu/6t6HjkEL2XiM2rQGwGiBy//rVkUsjZlVITzLo/G8G8xDbYhq4+KOAzEuNQcIHJWtygq0zJHgCuyn6tVXs3xdioKgmPeJxMa8UtpSFKNen0YCwMi+fYbg+gLRGOJZbNzo4VXU6ld1C1vpM84DzFFz7eySvIcXYGqbujFOBR+zMXbeNyv/22WKttn+uU2fy3SUbCDd9iBfBKBWgsrXWbzWa7XK9j12xHI6SQwvjnaXHU1VhGqQAvDU7BVDNIIXfs/n9rYaUUztZOfmfXXFWNmpyms7jkU+myrGo02ysXQJrGc5YExU6Np70pMqcXyT2GWjGKiVE/FJ9HnJOx0f1siPbtfoao7Tm4xt/X8CDXCTaw8Q7nvs7IttQ==)");
    auto publicKey      = std::string(R"(MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCvupt+SoaJxTnyvzo7d2xtVvhe5N2qxhyW/OVNuh/OeWO2T+BZPJsh6n3+YV4mulsAxmbUvvwpL9EI3xynjw5WpSsXgNAg+s2mAbyGTAOLAlPlymBy1R57XA0DgnIfEh3mQsiWP9uaXp8V8PNkWNC6d+JtRUEImDFy+mFI5XIcQwIDAQAB)");
    auto nodeKeyEnc     = R"(HNhyBaoz42\/FFPKpI+YDoxeKLEyB0Pvwai4Fpo49AiqPegdSjqQMaxmtZO\/HtWX48M65QnRcxF6AEcKMQqxQCy4NibgpN61rcpXmcOXNc9recE\/D\/lPdLys9nT5Wrtn7BuwPpWd9vOu0dIzmkNO2ReAcxBedyT4zPXGtOxi+zms=)";
    auto nodeKey        = "vohbW5WRs048GOIarXjchsymnSRPHvx9UuxHu8N9q3g=";
    auto aesIv          = R"(2yTS4KO5M14ix34g1aUFhA==)";
    auto aesSalt        = R"(NRxHK0GO+yA=)";

    auto masterKey = Crypto::calculateMasterPassword(passwordSalt, password);

    auto pkey = Crypto::aesDecrypt(giga::utils::str2wstr(masterKey), Crypto::base64decode(aesSalt), Crypto::base64decode(aesIv), Crypto::base64decode(privateKeyEnc));
    BOOST_CHECK(privateKey == pkey);

    auto rsa = Rsa{publicKey, pkey};
    auto nKey = rsa.decrypt(Crypto::base64decode(nodeKeyEnc));
    BOOST_CHECK(44 == nKey.size());
    if (nKey.size() < 44) {
        nKey = Crypto::base64encode(nKey);
    }
    BOOST_CHECK(nodeKey == nKey);
}

BOOST_AUTO_TEST_CASE(test_uncipher_keys)
{
    auto login          = U(R"(mobiuser01)");
    auto password       = U(R"(azertyuiop)");
    auto passwordSalt   = R"(3Xu9EtDPsnyYPjaNPKcbeACoM9chz9+lUdNxMBCDg2A=)";
    auto privateKeyEnc  = std::string(R"(qXfQCxZ7BeAb+2WVbxmRQGhn13ZrVwfvY1UyJfKGBea8KDQOpTg0RSFZ/ICuKBys9MWVND0Wn59mV3lvXnW51Z4BKtsnMESvWfGX2eb1fdhT5YMz5ba99WGc/yOFZJ3nXzcqVDWY1d6ujVMAffEmfGu2QgeWnWKE5S+AZkfgkElYiJK8jP1SLxOhUeEDJTli+enaZa2gBggZVA/x46TfZk9dZvSBv2DxvTBE6OnUAiOhSDdt3LVkCY3hgZT0W6ExDuz+LXc/mdWpyqhqz0KV2cDmYKzA2YhRsYp4ASJ3p25yMoekcAno73xq7saN9SOYgVpsirK1BxqXM1m5mlyyt672/5F8HCrK2MwblK1XgQKFLm5lr/2uSmtIyF8KmfWdwmwYw4aNYlj8Om0jBfLfOcmAMLhrR5TiJO6fp79/KBbQ5ykMNDizfxrcirC3wbVBHFXqg+/YKKfLL1KPuk+cOy2Zshe4PixvF1VHgyskJgrnmqUwmYuq2oY1nhzMuC5BbhyvVh6mwtSOjOpxR5zMg/GSusiE715RXdB2JYSQP6RSlgsHDem+pZWjVWToRqZwVIT9C2YW9w5x/WfEOCe28OQlrd3SiUyAWlQ74B5uE5bsqYGrrF7PtaC/ZrCCshIeAJOTR9s4PEWv5q2kb1Os4ETggw1sQnbwcvDKkBm5G1BO3yerrVH4pZJx7fNMcU+yy46EvRBxgdRe7OIPiggeZWnIe8RnzJqSbkinFnhlkX6z+9CcexhNGNZkTYNz4W2/V7L4lJ22ATJY4e1c3dYHREUn71sZ6iJV5NrnGz+pnUHTeiV5LMJGI/6eyg/Z2QSMqxoN8DCMvNSJfmhZUaDMgVNgLJNrIwH1zLF7z1wRnfN8ZQfzLQEYuqLHikhOB9yDPoLh843Q6Ia2Bk85BO/K4JoJvWRU4byTLJNgx8+pfvvHtCd88UPcT/Rc5mKj1Qo/QMbc2uU8xr+e+iLic+uNe88czzwP6RLGr5iFrqDt2i2V2eEDtcASB+ccuCPRZSY137Tyw98erXNZisGmqiqlVVmXi2I1cTEQmcrJ9RRuhmz1gGoOkovrqFgDr77O72u7mP5VpTocFefqqKZpE+jzio1gIEzpDKSJ6uzlRHo6cr5wE5PLgt1GoElRrfHqAkKq)");
    auto privateKey     = std::string(R"(MIICdgIBADANBgkqhkiG9w0BAQEFAASCAmAwggJcAgEAAoGBAJCrapHIRiA5OQJKX30/4hwxKkwM069W97vUyHTcL2yOjr2Yc0w/uopOwE2/5l6lBOkFjLptvbeCZN34tuPI4bydVSuGRtYqNh7I5xAu67xp5xpvqzFWxX/qq02nsu5eRcMT6AcTVR1RPaIOY7a6anrs8R/gI7jVtutVzXAHmYrRAgMBAAECgYAc1f4U9s17iMxjN33XEk+4fNhIdloPXg9dIimemXDee3/aLLP2o9FD1jSgirqZbN0QsWl+k3c4R8ZhND0IJDdSOqPaStqciC4QmwN0wbPPQV0jwWSrRpERTEr697ooJ/dznmrSRsRATrBrbhfTncxlXK/kzxj8nJT6dEeHAZPbfQJBAN68RJbtDS5hzwd59MmWGuRYCESWyiWVEepX6anzxDDFRrV5LXU6IvTXeKZzkwTzrkhdLoPHrYvkdXKuKWlrzK8CQQCmRn8iNStx3ztBVjjJ6qFcjbJiulIUyTGOV5lSP32K2gi/RkAZovtmmFQePz1yjvq274LuEdqyB3h4D/H0kQB/AkEA0pJerejyGzVwynqrx6DXT6VCO8Xq2ljyFQCD8Z55jjoTPSGQrlEGFz9V2tdM2NOfot4lSYSPN8AGoSKyF+5IFQJABDyrpK2CK+aqVeHilJKcBgW+U/G3gbwWvS/SQlxH0K/5pRYfMAgNK/IZwqON50tpbBvyVCq0TfEOytBtM/Q19QJADC9wxAYhRiiVZzg9ZyAzUtV+UBtS64PBwmdlJuUQkeCNzFWkxNrbj3RsQ/8Wl5WDVGpOTKr6kCY+YZ6VICUxlA==)");
    auto publicKey      = std::string(R"(MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCQq2qRyEYgOTkCSl99P+IcMSpMDNOvVve71Mh03C9sjo69mHNMP7qKTsBNv+ZepQTpBYy6bb23gmTd+LbjyOG8nVUrhkbWKjYeyOcQLuu8aecab6sxVsV/6qtNp7LuXkXDE+gHE1UdUT2iDmO2ump67PEf4CO41bbrVc1wB5mK0QIDAQAB)");
    auto nodeKeyEnc     = R"(JwXYePKScBfV7zOaT3EnVvOPMONTT1yhjqnyBs8pngnjc7dr2aJOF0QJrC3kCFQ8IfArGqxeQC8Hi52nTtrTrege4DYdbZkJIM2ckaBqMlc613pVbUz6AdX3qZ/q9WfNXbbbE0bJdLXBstSVMsBb86sed1gVvR5JkrOuPsnYBzY=)";
    auto nodeKey        = "asmiWz0ITE17vX4vWJ9Zw0ud581DBJ43RBBIdHBmeBU=";
    auto aesIv          = R"(Sc37HJg+jZLMtPq9UCmx7w==)";
    auto aesSalt        = R"(J/wabyBG4WA=)";

    auto loginPassword = Crypto::calculateLoginPassword(login, password);
    BOOST_CHECK("Ju51bwKeziurk32HMdVx8g==" == loginPassword);

    auto masterKey = Crypto::calculateMasterPassword(passwordSalt, password);
    BOOST_CHECK("+Q5WdxgwsDFC8/6JgK2JPA==" == masterKey);

    auto pkey = Crypto::aesDecrypt(giga::utils::str2wstr(masterKey), Crypto::base64decode(aesSalt), Crypto::base64decode(aesIv), Crypto::base64decode(privateKeyEnc));
    BOOST_CHECK(privateKey == pkey);

    auto rsa = Rsa{publicKey, pkey};
    auto nKey = rsa.decrypt(Crypto::base64decode(nodeKeyEnc));
    BOOST_CHECK(32 == nKey.size());
    if (nKey.size() < 44) {
        nKey = Crypto::base64encode(nKey);
    }
    BOOST_CHECK(nodeKey == nKey);
}
