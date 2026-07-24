#ifndef BRIDGE_HPP
#define BRIDGE_HPP


#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>
#include <string>
#include <stdexcept>


namespace fs = std::filesystem;


class Bridge
{

private:

    std::string runtime;


public:

    Bridge(
        std::string folder = "runtime"
    )
    {
        runtime = folder;

        fs::create_directories(
            runtime
        );
    }



    void send(
        std::string data,
        std::string name
    )
    {

        std::string tmp_file =
            runtime + "/" + name + ".tmp";


        std::string json_file =
            runtime + "/" + name + ".json";



        // เขียนไฟล์ชั่วคราว

        std::ofstream out(tmp_file, std::ios::trunc);
        if (!out) {
            throw std::runtime_error("Unable to open bridge temporary file");
        }


        out << data;


        out.close();

        if (!out) {
            throw std::runtime_error("Unable to write bridge message");
        }



        // เปลี่ยนชื่อเมื่อเสร็จ

        fs::remove(json_file);
        fs::rename(tmp_file, json_file);

    }



    std::string receive(std::string name)
    {

        std::string json_file =
            runtime + "/" + name + ".json";



        while(true)
        {

            if(fs::exists(json_file))
            {

                std::ifstream in(json_file);
                if (!in) {
                    continue;
                }


                std::string data(
                    (
                        std::istreambuf_iterator<char>(in)
                    ),
                    std::istreambuf_iterator<char>()
                );


                in.close();



                in.close();
                fs::remove(json_file);



                return data;

            }



            std::this_thread::sleep_for(
                std::chrono::milliseconds(10)
            );

        }

    }

};


#endif