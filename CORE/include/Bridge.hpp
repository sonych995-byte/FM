#ifndef BRIDGE_HPP
#define BRIDGE_HPP


#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>
#include <string>
#include <stdexcept>
#include <iostream>


namespace fs = std::filesystem;


class Bridge
{

private:

    fs::path runtime;


public:

    Bridge(
        fs::path folder = "runtime"
    )
    {

        runtime = folder;


        std::error_code ec;

        fs::create_directories(
            runtime,
            ec
        );


        if(ec)
        {
            throw std::runtime_error(
                "Cannot create bridge runtime: "
                + ec.message()
            );
        }


        std::cout
            << "[BRIDGE] Runtime: "
            << fs::absolute(runtime)
            << std::endl;

    }



    void send(
    const std::string& data,
    const std::string& name
)
{
    fs::path json_file =
        runtime / (name + ".json");


    fs::path tmp_file =
        runtime /
        (
            name + "_" +
            std::to_string(
                std::chrono::high_resolution_clock::now()
                .time_since_epoch()
                .count()
            )
            + ".tmp"
        );


    {
        std::ofstream out(
            tmp_file,
            std::ios::trunc
        );


        if(!out)
        {
            throw std::runtime_error(
                "Unable to open bridge temporary file: "
                + tmp_file.string()
            );
        }


        out << data;


        out.flush();


        if(!out)
        {
            throw std::runtime_error(
                "Unable to write bridge message"
            );
        }
    }


    std::error_code ec;


    fs::rename(
        tmp_file,
        json_file,
        ec
    );


    if(ec)
    {
        throw std::runtime_error(
            "Bridge rename failed: "
            + ec.message()
        );
    }
}




    std::string receive(
        const std::string& name
    )
    {

        fs::path json_file =
            runtime / (name + ".json");



        while(true)
        {

            if(fs::exists(json_file))
            {

                std::ifstream in(
                    json_file
                );


                if(!in)
{
    std::this_thread::sleep_for(
        std::chrono::milliseconds(10)
    );

    continue;
}



                std::string data(
                    (
                        std::istreambuf_iterator<char>(in)
                    ),
                    std::istreambuf_iterator<char>()
                );


                in.close();



                std::error_code ec;

                fs::remove(
                    json_file,
                    ec
                );



                return data;

            }



            std::this_thread::sleep_for(
                std::chrono::milliseconds(10)
            );

        }

    }

};


#endif