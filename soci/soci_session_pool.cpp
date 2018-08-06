#include <iostream>
#include <soci/soci.h>
#include <soci/mysql/soci-mysql.h>
#include <soci/connection-pool.h>

using namespace soci;

class soci_session_pool {
    public:
        std::shared_ptr<connection_pool> c_pool_str;
        
        soci_session_pool(size_t pool_size,const std::string& uri){
            c_pool_str = std::make_shared<connection_pool>(pool_size); 
            for(size_t i=0 ; i < pool_size; i++){
                session& sql = c_pool_str->at(i);
                sql.open(uri);
            }
        }

        void reconnect(session& sql){
            mysql_session_backend * mysqlBackEnd = static_cast<mysql_session_backend *>(sql.get_backend());
            int i = mysql_ping(mysqlBackEnd->conn_);
            if(i==1){
                sql.reconnect();
            }
        }

        session& get_session(size_t& pos){
            pos = c_pool_str->lease();
            session& sql = c_pool_str->at(pos);
            reconnect(sql);
            return sql;
        }

        void release(size_t pos){
            c_pool_str->give_back(pos);
        }

};

int main(){

    std::string uri = "mysql://db=testSoci user=root password='yuangehaha'";
    size_t pos;
    soci_session_pool sp(3,uri);
    for(int i=0;i<10;i++){
        session& sql = sp.get_session(pos);
        int num;
        sql << "select 1;",into(num);
        std::cout<<i<<":"<<num<<std::endl;
        std::cout<<pos<<std::endl;
        sp.release(pos);
    }

    return 0;
}

