#include <string>
#include <vector>
#include <iostream>
#include <cstdint>
#include <cassert>
#include "tinytmx/tinytmx.hpp"
#include "ghc/filesystem.hpp"
#include "yaml-cpp/yaml.h"

namespace fs = ghc::filesystem;

bool CheckPathValid(fs::path &input_dir, fs::path &output_dir)
{
    std::error_code ec;
    fs::file_status input_dir_status = fs::status(input_dir, ec);
    if (ec)
    {
        std::cerr << "input is not exists." << std::endl;
		return false;
    }

    if (!fs::is_directory(input_dir_status))
    {
        std::cerr << "input is not directory." << std::endl;
		return false;
    }

    fs::file_status output_dir_status = fs::status(output_dir, ec);
    if (ec)
    {
        fs::create_directories(output_dir);
    }
    else
    {
        if (!fs::is_directory(output_dir_status))
        {
            std::cerr << "output is not directory." << std::endl;
            return false;
        }
        else
        {
            if (!fs::is_empty(output_dir, ec))
            {
                std::cerr << "output is not empty directory." << std::endl;
                return false;
            }
        }
    }

	return true;
}

fs::path ConvertOutPath(fs::path &input_dir, fs::path &output_dir, fs::path &input_path)
{
	fs::path out_path{ "." };
	std::string input_str = input_path.u8string();
	std::string input_dir_str = input_dir.u8string();
	std::size_t len = input_dir_str.length();
	std::size_t pos = input_str.find(input_dir_str);
	if (pos != std::string::npos)
	{
		std::string input_left_str = input_str.substr(pos + len);
		std::string output_dir_str = output_dir.u8string();
		std::string out_str = output_dir_str + std::string(1, fs::path::preferred_separator) + input_left_str;
		out_path = out_str;
	}

	return out_path;
}

void ConvertFormat(fs::path &input_dir, fs::path &output_dir)
{
    if (!CheckPathValid(input_dir, output_dir))
    {
        return;
    }

    auto rdi = fs::recursive_directory_iterator(input_dir);
    for (auto de : rdi)
    {
        fs::path input_path = de.path();
        fs::path output_path = ConvertOutPath(input_dir, output_dir, input_path);

        if (de.is_regular_file())
        {
            if (!input_path.has_extension()) continue;
            if (!input_path.has_stem()) continue;

            std::string extension = input_path.extension().u8string();
            std::string stem = input_path.stem().u8string();
            uint64_t stem_len = stem.length();

            if (extension == ".tmx")
            {                    
                tinytmx::Map *map = new tinytmx::Map();
                map->ParseFile(input_path.string());
                assert(!map->HasError());

                unsigned **grids = new unsigned*[map->GetHeight()];
                for (uint32_t i = 0;i < map->GetHeight();++i)
                {
                    grids[i] = new unsigned[map->GetWidth()];
                }

                std::vector<tinytmx::TileLayer *> tile_layers = map->GetTileLayers();
                for (tinytmx::TileLayer *tile_layer : tile_layers)
                {                   
                    tinytmx::DataChunkTile const *data = tile_layer->GetDataTileFiniteMap();
                    for (uint32_t i = 0;i < data->GetHeight();++i)
                    {
                        for (uint32_t j = 0;j < data->GetWidth();++j)
                        {
                            grids[i][j] = data->GetTileGid(j,i);
                            //std::cout << grids[i][j] << " ";
                        }
                        //std::cout << std::endl;                            
                    }
                }

                fs::ofstream ofs;
                ofs.open(output_path.replace_extension(""));
                ofs << stem_len;
                ofs << stem;
				ofs << map->GetTileWidth();
				ofs << map->GetTileHeight();
                ofs << map->GetWidth();
                ofs << map->GetHeight();				

                for (uint32_t i = 0;i < map->GetHeight();++i)
                {
                    for (uint32_t j = 0;j < map->GetHeight();++j)
                    {
                        ofs << grids[i][j];
                    }
                }

                ofs.flush();
                ofs.close();
                    
                for (uint32_t i = 0;i < map->GetHeight();++i)
                {
                    delete []grids[i];
                }
                    
                delete []grids;
            }
        }
        else if (de.is_directory())
        {
            fs::create_directories(output_path);
        }
    }
}

int main(int argc, char *argv[])
{
	try
	{
		YAML::Node config = YAML::LoadFile("config.yaml");
		std::string input = config["input_directory"].as<std::string>();
		std::string output = config["output_directory"].as<std::string>();

		fs::path input_dir = fs::u8path(input);
		fs::path output_dir = fs::u8path(output);
		ConvertFormat(input_dir, output_dir);
	}
	catch (fs::filesystem_error const &fe)
	{
		std::cerr << "File Error: " << fe.what() << std::endl;
	}
	catch (std::exception const &ex)
	{
		std::cerr << "Error:" << ex.what() << std::endl;
	}

    return 0;
}
