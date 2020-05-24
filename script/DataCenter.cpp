#include "DataCenter.h"

DataCenter::DataCenter()
{	
    m_sql = new SqlInterface();

    if (!m_sql->init("ip", "user", "password", "database_name", 80))
    {
        cout << "m_sql init error " << endl;
    }
    cout << "m_sql success " << endl;
}

DataCenter::~DataCenter()
{
    delete m_sql;
}

//mysql
void DataCenter::do_sql(string& out_data, const vector<uint8_t>& bodyCvec)
{
    cout << "\033[1;33m==== do_sql ...\033[0m" << endl;
    string sJson;
    for (auto& i : bodyCvec)
    {
        sJson += static_cast<char>(i);
    }

    cout << "sJson: " << sJson << endl;
    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(sJson, root))
    {
        cout << "#### Json parse error" << "\n" << endl;
        Json::Value errorRoot;
        errorRoot["status"] = -1;
        errorRoot["reason"] = "Json parse error";
        out_data = errorRoot.toStyledString();
        return;
    }

    if (!m_sql->insert_data())
    {
        cout << "#### insert_data" << "\n" << endl;
        Json::Value errorRoot;
        errorRoot["status"] = -2;
        errorRoot["reason"] = "insert_data";
        out_data = errorRoot.toStyledString();
        return;
    }

    Json::Value success_data;
    success_data["status"] = 0;
    success_data["reason"] = "success";

    out_data = success_data.toStyledString();
    cout << "\033[1;33m==== month_data end\033[0m" << endl;

}