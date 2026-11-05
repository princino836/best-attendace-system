#include "crow.h"
#include <cmath>
#include <fstream>
#include <mutex>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <random>
#include <sstream>

struct Record { std::string name,id,kind,time; };
std::vector<Record> records; std::mutex mx;
bool faceOpen=false, codeActive=false; std::string currentCode;
const std::string adminPassword = []{const char* p=std::getenv("ADMIN_PASSWORD");return p?std::string(p):"ChangeThisAdminPassword";}();
const double centerLat=std::getenv("CENTER_LAT")?std::atof(std::getenv("CENTER_LAT")):5.6037;
const double centerLon=std::getenv("CENTER_LON")?std::atof(std::getenv("CENTER_LON")):-0.1870;
const double radius=std::getenv("RADIUS_METERS")?std::atof(std::getenv("RADIUS_METERS")):100.0;
const std::string dataFile=std::getenv("ATTENDANCE_FILE")?std::getenv("ATTENDANCE_FILE"):"attendance.csv";
std::string now(){std::time_t t=std::time(nullptr);char b[64];std::strftime(b,sizeof b,"%Y-%m-%d %H:%M:%S UTC",std::gmtime(&t));return b;}
double distance(double a,double b){const double R=6371000.0,pi=3.141592653589793;double p1=a*pi/180,p2=centerLat*pi/180,dp=(centerLat-a)*pi/180,dl=(centerLon-b)*pi/180;double h=std::sin(dp/2)*std::sin(dp/2)+std::cos(p1)*std::cos(p2)*std::sin(dl/2)*std::sin(dl/2);return 2*R*std::asin(std::sqrt(h));}
void save(){std::ofstream f(dataFile,std::ios::app);for(auto&r:records)f<<'"'<<r.name<<"\",\""<<r.id<<"\",\""<<r.kind<<"\",\""<<r.time<<"\"\n";records.clear();}
crow::response json(crow::json::wvalue v){return crow::response(v);}
int main(){crow::SimpleApp app;
CROW_ROUTE(app,"/")([]{return crow::response("Attendance C++ server is running. Upload index.html, style.css and script.js to the public/ folder.");});
CROW_ROUTE(app,"/api/admin/login").methods("POST"_method)([](const crow::request& req){auto j=crow::json::load(req.body);if(!j||!j.has("password")||j["password"].s()!=adminPassword)return json({{"error","Incorrect admin password"}});
return json({{"token",adminPassword}});});
auto auth=[](const crow::json::rvalue& j){return j.has("token")&&j["token"].s()==adminPassword;};
CROW_ROUTE(app,"/api/attendance").methods("POST"_method)([](const crow::request& req){auto j=crow::json::load(req.body);if(!j)return json({{"error","Invalid request"}});std::string name=j["name"].s(),id=j["studentId"].s(),kind=j["kind"].s();if(name.empty()||id.empty())return json({{"error","Name and student ID required"}});
std::lock_guard<std::mutex> lock(mx);
for(auto&r:records)if(r.id==id&&r.kind==kind)return json({{"error","Attendance already submitted for this session"}}); 
if(kind=="face"){if(!faceOpen)return json({{"error","Face-to-face attendance is closed"}});if(!j.has("lat")||!j.has("lon"))return json({{"error","Location data missing"}});double d=distance(j["lat"].d(),j["lon"].d());if(d>radius)return json({{"error","You are "+std::to_string((int)d)+" metres away. You must be within "+std::to_string((int)radius)+" metres."}});}
else if(kind=="online"){if(!codeActive||!j.has("code")||j["code"].s()!=currentCode)return json({{"error","Invalid or expired attendance code"}});}
else return json({{"error","Unknown attendance type"}});
records.push_back({name,id,kind,now()});save();return json({{"message","Attendance submitted successfully!"}});});
CROW_ROUTE(app,"/api/admin/face/open").methods("POST"_method)([&](const crow::request& req){auto j=crow::json::load(req.body);if(!j||!auth(j))return json({{"error","Unauthorized"}});faceOpen=true;return json({{"ok",true}});});
CROW_ROUTE(app,"/api/admin/face/close").methods("POST"_method)([&](const crow::request& req){auto j=crow::json::load(req.body);if(!j||!auth(j))return json({{"error","Unauthorized"}});faceOpen=false;return json({{"ok",true}});});
CROW_ROUTE(app,"/api/admin/code/generate").methods("POST"_method)([&](const crow::request& req){auto j=crow::json::load(req.body);if(!j||!auth(j))return json({{"error","Unauthorized"}});std::mt19937 g(std::random_device{}());currentCode=std::to_string(100000+g()%900000);codeActive=true;return json({{"code",currentCode}});});
CROW_ROUTE(app,"/api/admin/code/close").methods("POST"_method)([&](const crow::request& req){auto j=crow::json::load(req.body);if(!j||!auth(j))return json({{"error","Unauthorized"}});codeActive=false;currentCode="";return json({{"ok",true}});});
CROW_ROUTE(app,"/api/admin/status").methods("GET"_method)([&](const crow::request& req){auto t=req.url_params.get("token");if(!t||t!=adminPassword)return json({{"error","Unauthorized"}});crow::json::wvalue arr=crow::json::wvalue::list();std::ifstream f(dataFile);std::string line;while(std::getline(f,line)){std::stringstream ss(line);std::string a,b,c,d;std::getline(ss,a,'"');std::getline(ss,b,'"');std::getline(ss,c,'"');std::getline(ss,d,'"');if(!a.empty()&&a[0]=='"')a.erase(0,1);arr[arr.size()]={{"name",a},{"studentId",b},{"kind",c},{"time",d}};}return json({{"faceOpen",faceOpen},{"codeActive",codeActive},{"records",arr}});});
app.port(std::getenv("PORT")?std::atoi(std::getenv("PORT")):8080).multithreaded().run();
}