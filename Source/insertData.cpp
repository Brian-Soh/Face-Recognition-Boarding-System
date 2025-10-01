////
////  insertData.cpp
////  Face-Recognition-Boarding-System
////
////  Created by Brian Soh on 2025-09-17.
//
//
//#include <pqxx/pqxx>
//#include <iostream>
//#include <stdexcept>
//
//using namespace std;
//
//int main() {
//    try {
//        // For local tunnels, you can use sslmode=require.
//        // For direct RDS, prefer verify-full + sslrootcert, and host must match the cert.
//        string conn =
//        "host=xxx"
//        " port=5432"
//        " dbname=boarding_system"
//        " user=postgres"
//        " password=xxx"
//        " sslmode=verify-full"
//        " sslrootcert=Resources/us-west-2-bundle.pem";
//
//        pqxx::connection c{conn};
//        if (!c.is_open()) throw runtime_error("DB not open");
//
//        c.prepare("ins", "INSERT INTO public.boarding_system(id, name, seat, class) VALUES($1, $2, $3, $4)");
//
//        vector<vector<string>> passengers = {
//            {"192894", "Elon Musk", "1A", "First"},
//            {"390439", "Jeff Bezos", "7C", "Business"},
//            {"490303", "Mark Zuckerberg", "25B", "Economy"}
//        };
//
//        pqxx::work tx{c};
//
//        for (auto& p : passengers) {
//            tx.exec_prepared("ins", p[0], p[1], p[2], p[3]);
//        }
//
//        tx.commit();
//
//    } catch (const std::exception& e){
//        std::cerr << "Error: " << e.what() << "\n";
//        return -1;
//    }
//
//    return 0;
//}
//
