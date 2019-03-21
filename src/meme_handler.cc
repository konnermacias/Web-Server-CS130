#include <string>
#include <vector>
#include <boost/algorithm/string/classification.hpp> // Include boost::for is_any_of
#include <boost/algorithm/string/split.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include "meme_handler.h"
#include <sstream>
#include <sstream>
#include <fstream>
#include <thread>
#include <mutex>
#include <string>

std::mutex mtx;

RequestHandler *MemeHandler::create(const NginxConfig &config, const std::string &path)
{
    MemeHandler *mh = new MemeHandler();
    mh->root_ = path;
    mh->filedir_ = config.GetAttribute("location");
    mh->savedir_ = config.GetAttribute("save");
    mh->uri_ = config.GetAttribute("url");

    BOOST_LOG_TRIVIAL(trace) << "Meme handler created";

    return mh;
};

std::unique_ptr<Response> MemeHandler::HandleRequest(const Request &request)
{
    BOOST_LOG_TRIVIAL(trace) << "Meme handler building response for request...";

    std::string full_url = request.uri_path();
    memepage_ = full_url.substr(uri_.length() + 1, full_url.length());
   // std::cout << "URI: " << full_url << std::endl;

    if (memepage_ == "create") //Landing Page for Meme Creation
    {
        errorflag = MemeCreate();
    }
    else if (memepage_ == "list") // List all created memes
    {
        errorflag = MemeList();
    }
    else if (memepage_.substr(0, 7) == "list?q=") // Page to view a meme
    {
        errorflag = MemeSearch();
    }
    else if (memepage_.substr(0, 4) == "view") // Page to view a meme
    {
        errorflag = MemeView(false);
    }
    else if (memepage_.substr(0, 8) == "edit?id=") // Page to view a meme
    {
        errorflag = verifyTokenAuthentication(request) ? MemeEdit() : MemeView(true);
    }
    else if (memepage_.substr(0, 10) == "delete?id=")
    {
        errorflag = verifyTokenAuthentication(request) ? MemeDelete() : MemeView(true);
    }
    else if (request.method() == "POST")
    {
        std::map<std::string, std::string> memeMap = parseRequestBody(request.body());
        std::string redirect_id;

        // Determine new or edit type
        bool newMeme = true;
        if (memeMap["posttype"] == "edit")
            newMeme = false;

        // add meme-id with correct locks in place to database
        mtx.lock();
        //std::string meme_id = std::to_string(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())); // still does not solve race condition completely
       // std::cout << "New meme: " << newMeme << std::endl;
        redirect_id = AddToDatabase(
            memeMap["meme_id"],
            memeMap["access_token"],
            memeMap["name"],
            memeMap["image"],
            memeMap["top"],
            memeMap["bottom"],
            newMeme);
        mtx.unlock();

        errorflag = MemeResult(redirect_id);
    }
    else
    {
        errorflag = true;
    }

    std::unique_ptr<Response> response(new Response());

    //Send Correct Response
    if (!errorflag)
    {
        response->SetStatus(Response::OK);
        response->ReSetHeader("Content-Type", "text/html");
        response->SetHeader("Content-Length", std::to_string(memebody_.length()));
        response->SetBody(memebody_);
    }
    else //Send Error Response
    {
        std::string error_msg = "404: File not found on uri_path. Please provide correct uri_path.";
        response->SetStatus(Response::NOT_FOUND);
        response->ReSetHeader("Content-Type", "text/plain");
        response->SetHeader("Content-Length", std::to_string(error_msg.length()));
        response->SetBody(error_msg);

        //Server monitoring 
        std::cout << "::HandlerMetrics:: handler:error" << std::endl;
        std::cout << "::ResponseMetrics:: response_code:404" << std::endl;
    }

    BOOST_LOG_TRIVIAL(trace) << "Response built by meme handler...";
    return response;
}

bool MemeHandler::verifyTokenAuthentication(Request req)
{
    /*
        Verify's that the access-token and meme-id match up in database

        Author: Konner Macias
    */
    std::map<std::string, std::string> memeMap = parseRequestBody(req.body());
    
    // master token check
    if (memeMap["access-token"].length() > 0 && memeMap["access-token"] == "partialnotcomplete") { return true; }
    // check that access token has length and our - in it
    bool isAuthorized = (memeMap["access-token"].length() > 0 && (memeMap["access-token"].find("-") != std::string::npos));
    if (isAuthorized)
    {
        std::vector<std::map<std::string, std::string>> our_memes = GetAllFromDatabase();
        isAuthorized = false;
        // gather meme_id from acess token
        std::vector<std::string> meme_access_data;
        boost::split(meme_access_data, memeMap["access-token"], boost::is_any_of("-"), boost::token_compress_on);
        std::string meme_id = meme_access_data[0];

        for (size_t i = 0; i < our_memes.size(); i++)
        {
            if (our_memes[i]["ACCESS_TOKEN"] == memeMap["access-token"] && our_memes[i]["MEME_ID"] == meme_id)
            {
                isAuthorized = true;
                continue;
            }
        }
    }

    return isAuthorized;
}

static int callback_GetAllFromDatabase(void *ptr, int argc, char **argv, char **azColName)
{
    /*
        Callback function for SQL command. Returns a vector of maps to the address pointer 
        by ptr. ptr must point to a vector map.

        Author: Will Htun
    */

    std::vector<std::map<std::string, std::string>> *vectormap = static_cast<std::vector<std::map<std::string, std::string>> *>(ptr);
    std::map<std::string, std::string> *entry = new std::map<std::string, std::string>();
    for (int i = 0; i < argc; i++)
    {
        entry->insert(std::pair<std::string, std::string>(std::string(azColName[i]), std::string(argv[i])));
    }
    vectormap->push_back(*entry);

    return 0;
}

bool MemeHandler::MemeCreate()
{
    /*
        Hosts create.html page. Returns false if we achieved no error.
    */
    //std::ifstream ifs(".." + filedir_ + "/create.html", std::ios::in | std::ios::binary);
    std::ifstream ifs("memes_r_us/create.html", std::ios::in | std::ios::binary);
    if (!ifs.is_open() || memepage_.length() == 0)
    {
        return true;
    }

    char buf[512];
    while (ifs.read(buf, sizeof(buf)).gcount() > 0)
    {
        memebody_.append(buf, ifs.gcount());
    }

    ifs.close();
    return false;
}

bool MemeHandler::MemeDelete()
{
    /*
        Deletes meme specified by id
    */
    std::string meme_id;
    int meme_id_index = memepage_.find("id=");
    meme_id = memepage_.substr(meme_id_index + 3, memepage_.length() - 1);

    // TODO: throw an error for bad meme_id?
    std::map<std::string, std::string> meme_object = GetMemeFromDatabase(meme_id);

    // open database
    int rc;
    if ((rc = sqlite3_open(("../" + savedir_ + "/meme_vault.db").c_str(), &db)))
    {
        BOOST_LOG_TRIVIAL(trace) << "Error opening database...";
        sqlite3_close(db);
        return true;
    }
    BOOST_LOG_TRIVIAL(trace) << "Opened database for writing...";

    // delete meme from database
    sqlite3_stmt *stmt;
    std::string delete_sql = "DELETE FROM MEME_HISTORY WHERE MEME_ID = ";
    delete_sql += meme_id;

    rc = sqlite3_exec(db, delete_sql.c_str(), callback_GetAllFromDatabase, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(trace) << "Error reading table...";
    }

    // close database
    sqlite3_close(db);
    BOOST_LOG_TRIVIAL(trace) << "Closed database";

    // take them to list page after deletion
    //std::ifstream ifs(".." + filedir_ + "/delete.html", std::ios::in | std::ios::binary);
    std::ifstream ifs("memes_r_us/delete.html", std::ios::in | std::ios::binary);
    if (!ifs.is_open() || memepage_.length() == 0)
    {
        return true;
    }

    char buf[512];
    while (ifs.read(buf, sizeof(buf)).gcount() > 0)
    {
        memebody_.append(buf, ifs.gcount());
    }

    ifs.close();
}

std::map<std::string, std::string> MemeHandler::parseRequestBody(std::string body)
{
    /*
        Parses request body and assigns appropriate key value pairs inside
        of a meme map which is returned.

        Author: Konner Macias
    */
    std::map<std::string, std::string> memeMap;
    // URL to ASCII decoding and
    // split on & frist
    std::vector<std::string> items;
    boost::split(items, body, boost::is_any_of("&"), boost::token_compress_on);
    bool deepfry = false;

    // bad request checks
    if (items.size() < 3)
    {
        if (body.find("access") != std::string::npos)
        {
            //std::cout << "getting access" << std::endl;
            std::vector<std::string> key_value;
            boost::split(key_value, items[0], boost::is_any_of("="), boost::token_compress_on);
            memeMap[key_value[0]] = key_value[1];
            //std::cout << "Key-val: " << key_value[0] << " : " << key_value[1] << std::endl;
        }
        return memeMap;
    }

    for (size_t i = 0; i < items.size(); i++)
    {
        std::vector<std::string> key_value;
        boost::split(key_value, items[i], boost::is_any_of("="), boost::token_compress_on);
        key_value[1] = URLParser::urlDecode(key_value[1]);

        if (key_value.size() != 2)
        {
            return memeMap;
        }

        // replace "%2F" with "/" if needed
        int symbol_ind = key_value[1].find("%2F");
        if (symbol_ind != std::string::npos)
        {
            key_value[1].replace(symbol_ind, 3, "/");
        }

        // replace "+" with " " if needed
        std::string::size_type n = 0;
        while ((n = key_value[1].find("+", n)) != std::string::npos)
        {
            //key_value[1].replace(n, 1, " ");
            key_value[1][n] = ' ';
            n += 1; // update for added " "
        }

        // add key value to memeMap
        if (key_value[0] == "deepfry" && key_value[1] == "True")
            deepfry = true;

        memeMap[key_value[0]] = key_value[1];
    }

    if (deepfry)
    {
        memeMap["image"] = "df-" + memeMap["image"];
    }

    memeMap["image"] = "meme_templates/" + memeMap["image"];
    memeMap["name"] = URLParser::htmlEncode(memeMap["name"]);
    memeMap["top"] = URLParser::htmlEncode(memeMap["top"]);
    memeMap["bottom"] = URLParser::htmlEncode(memeMap["bottom"]);

    std::cout << URLParser::htmlEncode(std::string(memeMap["top"])) << std::endl;
     
    return memeMap;
}

bool MemeHandler::MemeView(bool incorrectAccessToken)
{
    /*
        Returns an html page pouplated with the meme object associated with the meme ID

        Author: Will Htun
    */
    std::string meme_object_name;
    std::string meme_object_img;
    std::string meme_object_top;
    std::string meme_object_bot;
    std::string meme_object_atoken;

    int meme_id_index = memepage_.find("id=");
    std::string meme_id;
    if (meme_id_index != std::string::npos)
    {
        meme_id = memepage_.substr(meme_id_index + 3, memepage_.length() - 1);
    }
    else
    {
        BOOST_LOG_TRIVIAL(trace) << "Invalid meme id...";
        return true;
    }

    // SQL SELECT operation
    int rc;
    sqlite3_stmt *stmt;
    rc = sqlite3_open(("../" + savedir_ + "/meme_vault.db").c_str(), &db);
    if (rc)
    {
        BOOST_LOG_TRIVIAL(trace) << "Error opening database...";
        sqlite3_close(db);
        return true;
    }
    BOOST_LOG_TRIVIAL(trace) << "Opened database for writing...";
    // Injection Test
    // meme_id = "1551771020; DROP TABLE MEME_HISTORY";

    std::string selector = "SELECT * FROM MEME_HISTORY WHERE MEME_ID = ?";
    rc = sqlite3_prepare_v2(db, selector.c_str(), selector.length(), &stmt, NULL);
    if (rc == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, meme_id.c_str(), meme_id.length(), 0);
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            meme_object_name = strdup((const char *)sqlite3_column_text(stmt, 1));
            meme_object_img = strdup((const char *)sqlite3_column_text(stmt, 2));
            meme_object_top = strdup((const char *)sqlite3_column_text(stmt, 3));
            meme_object_bot = strdup((const char *)sqlite3_column_text(stmt, 4));
            meme_object_atoken = strdup((const char *)sqlite3_column_text(stmt, 5));
        }
        else
        {
            return true; 
        }
        
        sqlite3_finalize(stmt);
    }
    if (rc != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(trace) << "Error reading table...";
    }

    sqlite3_close(db);
    BOOST_LOG_TRIVIAL(trace) << "Closed database";

    //std::ifstream ifs(".." + filedir_ + "/view.html", std::ios::in | std::ios::binary);
    std::ifstream ifs("memes_r_us/view.html", std::ios::in | std::ios::binary);
    if (!ifs.is_open() || memepage_.length() == 0)
    {
        return true;
    }

    char buf[512];
    while (ifs.read(buf, sizeof(buf)).gcount() > 0)
    {
        memebody_.append(buf, ifs.gcount());
    }

    ifs.close();

    if (incorrectAccessToken)
    {
        memebody_ += "<div id=\"editor\"><p>Access Token Denied</p>";
    }
    memebody_ += "</div></body></html>";

    boost::replace_all(memebody_, "meme_object_id", meme_id);
    boost::replace_all(memebody_, "meme_object_name", meme_object_name);
    boost::replace_all(memebody_, "meme_object_img", meme_object_img);
    boost::replace_all(memebody_, "meme_object_top", meme_object_top);
    boost::replace_all(memebody_, "meme_object_bot", meme_object_bot);
    boost::replace_all(memebody_, "meme_object_atoken", meme_object_atoken);

    return false;
}

bool MemeHandler::MemeResult(std::string id_)
{
    /*
        Returns an html page pouplated with the meme object associated with the meme ID

        Author: Will Htun
    */

    std::string meme_object_name;
    std::string meme_object_img;
    std::string meme_object_top;
    std::string meme_object_bot;
    std::string meme_object_atoken;

    // SQL SELECT operation
    int rc;
    sqlite3_stmt *stmt;
    rc = sqlite3_open(("../" + savedir_ + "/meme_vault.db").c_str(), &db);
    if (rc)
    {
        BOOST_LOG_TRIVIAL(trace) << "Error opening database...";
        sqlite3_close(db);
        return true;
    }
    BOOST_LOG_TRIVIAL(trace) << "Opened database for writing...";

    std::string selector = "SELECT * FROM MEME_HISTORY WHERE MEME_ID = ?";
    rc = sqlite3_prepare_v2(db, selector.c_str(), selector.length(), &stmt, NULL);
    if (rc == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, id_.c_str(), id_.length(), 0);
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            meme_object_name = strdup((const char *)sqlite3_column_text(stmt, 1));
            meme_object_img = strdup((const char *)sqlite3_column_text(stmt, 2));
            meme_object_top = strdup((const char *)sqlite3_column_text(stmt, 3));
            meme_object_bot = strdup((const char *)sqlite3_column_text(stmt, 4));
            meme_object_atoken = strdup((const char *)sqlite3_column_text(stmt, 5));
        }
        sqlite3_finalize(stmt);
    }

    if (rc != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(trace) << "Error reading table...";
    }

    sqlite3_close(db);
    BOOST_LOG_TRIVIAL(trace) << "Closed database";

    //std::ifstream ifs(".." + filedir_ + "/view.html", std::ios::in | std::ios::binary);
    std::ifstream ifs("memes_r_us/view.html", std::ios::in | std::ios::binary);
    if (!ifs.is_open() || memepage_.length() == 0)
    {
        return true;
    }

    char buf[512];
    while (ifs.read(buf, sizeof(buf)).gcount() > 0)
    {
        memebody_.append(buf, ifs.gcount());
    }

    ifs.close();

    // showcase access-token on result only if it's a new meme
    if (meme_object_atoken.length() > 0)
    {
        memebody_ +=    "<div id=\"infobox\">"
                            "<a href=\"http://ss.gitrdone.cs130.org/meme/view?id=" + id_ + "\" class=\"description\"> URL: http://ss.gitrdone.cs130.org/meme/view?id=" + id_ + "</a><br>"
                            "<p>Access Token (Used for Deletion): meme_object_atoken</p>"
                        "</div>"
                        "</body></html>";
    }
    
    boost::replace_all(memebody_, "meme_object_id", id_);
    boost::replace_all(memebody_, "meme_object_name", meme_object_name);
    boost::replace_all(memebody_, "meme_object_img", meme_object_img);
    boost::replace_all(memebody_, "meme_object_top", meme_object_top);
    boost::replace_all(memebody_, "meme_object_bot", meme_object_bot);
    boost::replace_all(memebody_, "meme_object_atoken", meme_object_atoken);

    return false;
}

bool MemeHandler::MemeList()
{
    std::vector<std::map<std::string,std::string>> memelist = GetAllFromDatabase();
    std::reverse(memelist.begin(),memelist.end());

    // build body string
    //std::ifstream ifs(".." + filedir_ + "/list.html", std::ios::in | std::ios::binary);
    std::ifstream ifs("memes_r_us/list.html", std::ios::in | std::ios::binary);
    if (!ifs.is_open() || memepage_.length() == 0)
    {
        return true;
    }

    char buf[512];
    while (ifs.read(buf, sizeof(buf)).gcount() > 0)
    {
        memebody_.append(buf, ifs.gcount());
    }

    ifs.close();

    // bring in meme information and create links TODO: adjust according to how the information is stored. 
    if (memelist.size() == 0) 
    {
        memebody_ += "<p class=\"description\"> No results found </p>";
    }

    for (int i = 0; i < memelist.size(); i++)
    {
        memebody_ += "<a id=\"entry\" href=\"http://ss.gitrdone.cs130.org/meme/view?id=" + memelist[i]["MEME_ID"] + "\">";
        memebody_ += "MemeID: " + memelist[i]["MEME_ID"] + " :-: Name: " + memelist[i]["NAME"] + "\n";
        memebody_ += "</a><br />\n";
    }

    memebody_ += "</div>\n</div>\n";
    memebody_ += "</body>\n</html>";

    boost::replace_all(memebody_, "searchterm_replacethis", "");

    return false;
}

bool MemeHandler::MemeSearch()
{
    int search_index = memepage_.find("list?q=");
    std::string search_term;
    std::string search_term_replacement;

    int j = memepage_.find("&");
    search_term = memepage_.substr(search_index + 7, j - search_index - 7);
    boost::replace_all(search_term, "+", " ");

    search_term = URLParser::urlDecode(search_term);
    search_term = URLParser::htmlDecode(search_term);
    search_term_replacement = search_term;

    int sort_index = memepage_.find("&sort=");
    std::string sort_term;
    if (sort_index != std::string::npos)
    {
        sort_term = memepage_.substr(sort_index + 6, memepage_.length() - 1);
    }
    else
    {
        BOOST_LOG_TRIVIAL(trace) << "Invalid meme id...";
        return true;
    }

    std::vector<std::map<std::string, std::string>> memelist = SearchFromDatabase(search_term, sort_term);


    // build body string
    //std::ifstream ifs(".." + filedir_ + "/list.html", std::ios::in | std::ios::binary);
    std::ifstream ifs("memes_r_us/list.html", std::ios::in | std::ios::binary);
    if (!ifs.is_open() || memepage_.length() == 0)
    {
        return true;
    }

    char buf[512];
    while (ifs.read(buf, sizeof(buf)).gcount() > 0)
    {
        memebody_.append(buf, ifs.gcount());
    }

    ifs.close();

    // bring in meme information and create links TODO: adjust according to how the information is stored. 
    if (memelist.size() == 0) 
    {
        memebody_ += "<p class=\"description\"> No results found </p>";
    }
    else
    {
        memebody_ += "<p class=\"description\"> Found " + std::to_string(memelist.size()) + " current search terms</p>";
    }

    for (int i = 0; i < memelist.size(); i++) 
    {
        memebody_ += "<a id=\"entry\" href=\"http://ss.gitrdone.cs130.org/meme/view?id=" + memelist[i]["MEME_ID"] + "\">";
        memebody_ += "MemeID: " + memelist[i]["MEME_ID"] + " :-: Name: " + memelist[i]["NAME"] + "\n";
        memebody_ += "</a><br />\n";
    }

    memebody_ += "</div>\n</div>\n";
    memebody_ += "</body>\n</html>";

    boost::replace_all(memebody_, "searchterm_replacethis", search_term_replacement);
    boost::replace_all(memebody_, "sortterm_replacethis", sort_term);

    return false;
}

bool MemeHandler::MemeEdit()
{
    /*
        Hosts create.html page. Returns false if we achieved no error.
    */
    int meme_id_index = memepage_.find("id=");
    std::string meme_id = memepage_.substr(meme_id_index + 3, memepage_.length() - 1);

    //std::ifstream ifs(".." + filedir_ + "/edit.html", std::ios::in | std::ios::binary);
    std::ifstream ifs("memes_r_us/edit.html", std::ios::in | std::ios::binary);
    if (!ifs.is_open() || memepage_.length() == 0)
    {
        return true;
    }

    char buf[512];
    while (ifs.read(buf, sizeof(buf)).gcount() > 0)
    {
        memebody_.append(buf, ifs.gcount());
    }
    ifs.close();

    std::map<std::string, std::string> meme_object = GetMemeFromDatabase(meme_id);

    boost::replace_all(memebody_, "meme_object_id", meme_id);
    boost::replace_all(memebody_, "meme_object_name", URLParser::htmlDecode(meme_object["NAME"]));
    boost::replace_all(memebody_, "meme_object_img", meme_object["IMAGE"]);
    boost::replace_all(memebody_, "meme_object_top", URLParser::htmlDecode(meme_object["TOP"]));
    boost::replace_all(memebody_, "meme_object_bot", URLParser::htmlDecode(meme_object["BOTTOM"]));
    boost::replace_all(memebody_, "meme_object_atoken", meme_object["ACCESS_TOKEN"]);

    return false;
}

// credit: https://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c
std::string MemeHandler::gen_access_token(size_t len)
{
    std::string scramble = "";
    // scrambled string example: 10001-AB903
    for (size_t i = 0; i < len * 2; ++i)
    {
        int randomChar = rand() % (26 + 26 + 10);
        if (randomChar < 26)
            scramble += 'a' + randomChar;
        else if (randomChar < 26 + 26)
            scramble += 'A' + randomChar - 26;
        else
            scramble += '0' + randomChar - 26 - 26;
    }

    //std::cout << scramble << std::endl;
    return scramble;
}

std::string MemeHandler::AddToDatabase(std::string meme_id_, std::string access_token_, std::string name_, std::string image_, std::string top_, std::string bottom_, bool newmeme_)
{
    /*
        Add an entry to the database with the given values.

        Author: Will Htun
    */

    // SQL CREATE operation
    int rc;
    std::string id_ = "10000";
    std::string atoken_ = "00001";

    sqlite3_stmt *stmt;
    rc = sqlite3_open(("../" + savedir_ + "/meme_vault.db").c_str(), &db);
    if (rc)
    {
        BOOST_LOG_TRIVIAL(trace) << "Error opening/creating database...";
        sqlite3_close(db);
        errorflag = true;
    }
    BOOST_LOG_TRIVIAL(trace) << "Opened database for writing...";

    std::string table = "CREATE TABLE IF NOT EXISTS MEME_HISTORY("
                        "MEME_ID  TEXT     PRIMARY KEY  NOT NULL,"
                        "NAME     TEXT                  NOT NULL,"
                        "IMAGE    TEXT                  NOT NULL,"
                        "TOP      TEXT                  NOT NULL,"
                        "BOTTOM   TEXT                  NOT NULL,"
                        "ACCESS_TOKEN TEXT              NOT NULL);";

    rc = sqlite3_exec(db, table.c_str(), NULL, NULL, NULL);
    if (rc != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(trace) << "Error creating table...";
        sqlite3_close(db);
        errorflag = true;
    }
    BOOST_LOG_TRIVIAL(trace) << "Successfully created table!";

    if (newmeme_)
    {   // New meme, assign a fresh ID
        // SQL SELECT operation to get last ID for counter
        std::string selector = ("SELECT * FROM MEME_HISTORY WHERE MEME_ID = (SELECT MAX(MEME_ID) FROM MEME_HISTORY)");
        rc = sqlite3_prepare_v2(db, selector.c_str(), selector.length(), &stmt, NULL);
        if (rc == SQLITE_OK)
        {
            if (sqlite3_step(stmt) != SQLITE_DONE)
            {
                id_ = strdup((const char *)sqlite3_column_text(stmt, 0));
                id_ = std::to_string(stoi(id_) + 1);
            }
            // assign access token. Ex: 10001-AM?GE4
            atoken_ = id_ + "-" + gen_access_token(id_.length());
            //std::cout << atoken_ << std::endl;
            sqlite3_finalize(stmt);
        }
    }
    else
    { // Edit meme, use old ID
        id_ = meme_id_;
        std::string selector = ("SELECT ACCESS_TOKEN FROM MEME_HISTORY WHERE MEME_ID = ?");
        rc = sqlite3_prepare_v2(db, selector.c_str(), selector.length(), &stmt, NULL);
        if (rc == SQLITE_OK)
        {
            sqlite3_bind_text(stmt, 1, id_.c_str(), id_.length(), 0);
            if (sqlite3_step(stmt) != SQLITE_DONE)
            {
                atoken_ = strdup((const char *)sqlite3_column_text(stmt, 0));
                //std::cout << "atoken_: " << atoken_ << std::endl;
            }
            sqlite3_finalize(stmt);
        }

        // read access token of existing entry
    }

    // SQL INSERT operation
    std::string entry = "INSERT OR REPLACE INTO MEME_HISTORY (MEME_ID,NAME,IMAGE,TOP,BOTTOM,ACCESS_TOKEN) "
                        "VALUES (?,?,?,?,?,?);";

    rc = sqlite3_prepare_v2(db, entry.c_str(), entry.length(), &stmt, NULL);
    if (rc == SQLITE_OK)
    {
        // bind the value
        sqlite3_bind_text(stmt, 1, id_.c_str(), id_.length(), 0);
        sqlite3_bind_text(stmt, 2, name_.c_str(), name_.length(), 0);
        sqlite3_bind_text(stmt, 3, image_.c_str(), image_.length(), 0);
        sqlite3_bind_text(stmt, 4, top_.c_str(), top_.length(), 0);
        sqlite3_bind_text(stmt, 5, bottom_.c_str(), bottom_.length(), 0);
        sqlite3_bind_text(stmt, 6, atoken_.c_str(), atoken_.length(), 0);

        // commit
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    if (rc != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(trace) << "Error adding entry to table...";
        sqlite3_close(db);
        errorflag = true;
    }
    BOOST_LOG_TRIVIAL(trace) << "Successfully added entry!";

    sqlite3_close(db);
    BOOST_LOG_TRIVIAL(trace) << "Closed database";
    errorflag = false;
    return id_;
}

std::map<std::string, std::string> MemeHandler::GetMemeFromDatabase(std::string id_)
{
    /*
        Returns the meme with matching id

        Author: Will Htun
    */

    std::map<std::string, std::string> *meme_object = new std::map<std::string, std::string>();

    // SQL SELECT operation
    int rc;
    sqlite3_stmt *stmt;
    rc = sqlite3_open(("../" + savedir_ + "/meme_vault.db").c_str(), &db);

    if (rc)
    {
        BOOST_LOG_TRIVIAL(trace) << "Error opening/creating database...";
    }
    BOOST_LOG_TRIVIAL(trace) << "Opened database for reading...";

    std::string selector = "SELECT * FROM MEME_HISTORY WHERE MEME_ID = ?";
    rc = sqlite3_prepare_v2(db, selector.c_str(), selector.length(), &stmt, NULL);
    if (rc == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, id_.c_str(), id_.length(), 0);
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            meme_object->insert(std::pair<std::string, std::string>("MEME_ID", strdup((const char *)sqlite3_column_text(stmt, 0))));
            meme_object->insert(std::pair<std::string, std::string>("NAME", strdup((const char *)sqlite3_column_text(stmt, 1))));
            meme_object->insert(std::pair<std::string, std::string>("IMAGE", strdup((const char *)sqlite3_column_text(stmt, 2))));
            meme_object->insert(std::pair<std::string, std::string>("TOP", strdup((const char *)sqlite3_column_text(stmt, 3))));
            meme_object->insert(std::pair<std::string, std::string>("BOTTOM", strdup((const char *)sqlite3_column_text(stmt, 4))));
            meme_object->insert(std::pair<std::string, std::string>("ACCESS_TOKEN", strdup((const char *)sqlite3_column_text(stmt, 5))));
        }
        sqlite3_finalize(stmt);
    }
    if (rc != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(trace) << "Error reading table...";
    }

    sqlite3_close(db);
    BOOST_LOG_TRIVIAL(trace) << "Closed database";

    return *meme_object;
}

std::vector<std::map<std::string, std::string>> MemeHandler::GetAllFromDatabase()
{
    /*
        Returns all the memes in the database in the form of a vector of maps.

        Author: Will Htun
    */

    std::vector<std::map<std::string, std::string>> meme_entries_;

    // SQL SELECT operation
    int rc;
    rc = sqlite3_open(("../" + savedir_ + "/meme_vault.db").c_str(), &db);

    if (rc)
    {
        BOOST_LOG_TRIVIAL(trace) << "Error opening/creating database...";
    }
    BOOST_LOG_TRIVIAL(trace) << "Opened database for reading...";

    std::string selector = "SELECT * FROM MEME_HISTORY";
    rc = sqlite3_exec(db, selector.c_str(), callback_GetAllFromDatabase, &meme_entries_, NULL);
    if (rc != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(trace) << "Error reading table...";
    }

    sqlite3_close(db);
    BOOST_LOG_TRIVIAL(trace) << "Closed database";
    return meme_entries_;
}

std::vector<std::map<std::string, std::string>> MemeHandler::SearchFromDatabase(std::string term, std::string sort)
{
    /*
        Returns all the memes in the database that contain the term string

        Author: Will Htun
    */

    std::vector<std::map<std::string, std::string>> meme_entries_;
    term = URLParser::htmlEncode(term);
    term = "%" + term + "%";
    // SQL SELECT operation
    int rc;
    sqlite3_stmt *stmt;
    rc = sqlite3_open(("../" + savedir_ + "/meme_vault.db").c_str(), &db);

    if (rc)
    {
        BOOST_LOG_TRIVIAL(trace) << "Error opening/creating database...";
    }
    BOOST_LOG_TRIVIAL(trace) << "Opened database for reading...";

    std::string selector = "SELECT * FROM MEME_HISTORY WHERE NAME LIKE ? OR TOP LIKE ? OR BOTTOM LIKE ?";
    if (sort == "a-z")
        selector = "SELECT * FROM MEME_HISTORY WHERE NAME LIKE ? OR TOP LIKE ? OR BOTTOM LIKE ? ORDER BY NAME ASC";
    else if (sort == "z-a")
        selector = "SELECT * FROM MEME_HISTORY WHERE NAME LIKE ? OR TOP LIKE ? OR BOTTOM LIKE ? ORDER BY NAME DESC";
    else if (sort == "id")
        selector = "SELECT * FROM MEME_HISTORY WHERE NAME LIKE ? OR TOP LIKE ? OR BOTTOM LIKE ? ORDER BY MEME_ID ASC";

    rc = sqlite3_prepare_v2(db, selector.c_str(), selector.length(), &stmt, NULL);
    if (rc == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, term.c_str(), term.length(), 0);
        sqlite3_bind_text(stmt, 2, term.c_str(), term.length(), 0);
        sqlite3_bind_text(stmt, 3, term.c_str(), term.length(), 0);
        while (sqlite3_step(stmt) != SQLITE_DONE)
        {
            std::map<std::string, std::string> *meme_object = new std::map<std::string, std::string>;
            meme_object->insert(std::pair<std::string, std::string>("MEME_ID", strdup((const char *)sqlite3_column_text(stmt, 0))));
            meme_object->insert(std::pair<std::string, std::string>("NAME", strdup((const char *)sqlite3_column_text(stmt, 1))));
            meme_object->insert(std::pair<std::string, std::string>("IMAGE", strdup((const char *)sqlite3_column_text(stmt, 2))));
            meme_object->insert(std::pair<std::string, std::string>("TOP", strdup((const char *)sqlite3_column_text(stmt, 3))));
            meme_object->insert(std::pair<std::string, std::string>("BOTTOM", strdup((const char *)sqlite3_column_text(stmt, 4))));
            meme_object->insert(std::pair<std::string, std::string>("ACCESS_TOKEN", strdup((const char *)sqlite3_column_text(stmt, 5))));
            meme_entries_.push_back(*meme_object);
        }
        sqlite3_finalize(stmt);
    }
    if (rc != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(trace) << "Error reading table...";
    }

    sqlite3_close(db);
    BOOST_LOG_TRIVIAL(trace) << "Closed database";

    if (sort == "newest")
        std::reverse(meme_entries_.begin(),meme_entries_.end());

    return meme_entries_;
}