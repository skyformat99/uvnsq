/*
   Copyright ©2019, orcaer@yeah All rights reserved.

   Author: hebaichuan

   Last modified: 2019-9-16

   Description: uv-nsq
*/

#include <string>
#include <json/json.h>
#include <uv/uv11.h>

#include "NsqLookupd.h"
#include "http/EasyCurl.h"

using namespace std;
using namespace nsq;

long NsqLookupd::HttpGet(std::string&& url, std::string& header, std::string& resp)
{
    base::EasyCurl http;
    http.setopt(CURLOPT_URL, url.c_str());

    http.setopt(CURLOPT_WRITEFUNCTION, base::EasyCurl::WriteCallback);
    http.setopt(CURLOPT_WRITEDATA, &resp);
    http.setopt(CURLOPT_HEADERDATA, &header);

    long code;
    http.getinfo(CURLINFO_RESPONSE_CODE, &code);

    http.perform();
    return code;
}

int nsq::NsqLookupd::GetNodes(std::string&& url, std::vector<NsqNode>& nodes)
{
    std::string header;
    std::string context;
    auto code = NsqLookupd::HttpGet(std::move(url), header, context);

    Json::Reader reader;
    Json::Value root;
    if (reader.parse(context, root))
    {
        auto nodesCnt = root["producers"].size();
        auto& producers = root["producers"];
        for (auto i = 0; i < nodesCnt; i++)
        {
            NsqNode node;
            node.remoteaddr = producers[i]["remote_address"].asString();
            node.httpport = producers[i]["http_port"].asInt();
            node.tcpport = producers[i]["tcp_port"].asInt();
            nodes.push_back(node);
        }
    }
    else
    {
        uv::LogWriter::Instance()->error("parse node's json fail.");
    }
    return code;
}