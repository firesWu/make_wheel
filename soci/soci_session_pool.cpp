#include <iostream>
#include <soci/soci.h>
#include <soci/mysql/soci-mysql.h>
#include <soci/connection-pool.h>

using namespace soci;

class soci_session_pool {
    public:
        std::shared_ptr<connection_pool> c_pool_ptr;
        
        soci_session_pool(size_t pool_size,const std::string& uri){
            c_pool_ptr = std::make_shared<connection_pool>(pool_size); 
            for(size_t i=0 ; i < pool_size; i++){
                session& sql = c_pool_ptr->at(i);
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
            pos = c_pool_ptr->lease();
            session& sql = c_pool_ptr->at(pos);
            reconnect(sql);
            return sql;
        }

        void release(size_t pos){
            c_pool_ptr->give_back(pos);
        }

        // void reconnect(std::unique_ptr<session> sql_ptr){
        //     mysql_session_backend * mysqlBackEnd = static_cast<mysql_session_backend *>(sql_ptr->get_backend());
        //     int i = mysql_ping(mysqlBackEnd->conn_);
        //     if(i==1){
        //         sql_ptr->reconnect();
        //     }
        // }

        std::unique_ptr<session> get_session(){
            auto sql_ptr = std::make_unique<soci::session>(*c_pool_ptr);
            mysql_session_backend * mysqlBackEnd = static_cast<mysql_session_backend *>(sql_ptr->get_backend());

            int i = mysql_ping(mysqlBackEnd->conn_);
            if(i==1){
                sql_ptr->reconnect();
            }

            return sql_ptr;
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

    std::cout<<"new way"<<std::endl;

    for(int i=0;i<10;i++){
        auto sql = sp.get_session();
        int num;
        *sql << "select 1;",into(num);
        std::cout<<i<<":"<<num<<std::endl;
    }

    return 0;
}
