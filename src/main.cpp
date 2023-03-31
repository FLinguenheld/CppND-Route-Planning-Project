#include <optional>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

#include <io2d.h>
#include "route_model.h"
#include "render.h"
#include "route_planner.h"

using namespace std;
using namespace std::experimental;

static std::optional<vector<std::byte>> ReadFile(const string &path)
{   
    std::ifstream is{path, std::ios::binary | std::ios::ate};
    if( !is )
        return std::nullopt;
    
    auto size = is.tellg();
    vector<std::byte> contents(size);    
    
    is.seekg(0);
    is.read((char*)contents.data(), size);

    if( contents.empty() )
        return std::nullopt;
    return std::move(contents);
}

/* Ask user to enter a value and convert it into a float */
float AskFloat(string PromptTxt)
{
    string user_input;

    while (1) {
        cout << PromptTxt;
        cin >> user_input;
        try{
            return std::stof(user_input);
        }
        catch (const std::invalid_argument& ia){
            cout << " ! Please enter a float !\n";
        }
    }
}

int main(int argc, const char **argv)
{    
    string osm_data_file = "";
    if( argc > 1 ) {
        for( int i = 1; i < argc; ++i )
            if( std::string_view{argv[i]} == "-f" && ++i < argc )
                osm_data_file = argv[i];
    }
    else {
        cout << "To specify a map file use the following format: " << std::endl;
        cout << "Usage: [executable] [-f filename.osm]" << std::endl;
        osm_data_file = "../map.osm";
    }
    
    vector<std::byte> osm_data;
 
    if( osm_data.empty() && !osm_data_file.empty() ) {
        cout << "Reading OpenStreetMap data from the following file: " <<  osm_data_file << std::endl;
        auto data = ReadFile(osm_data_file);
        if( !data )
            cout << "Failed to read." << std::endl;
        else
            osm_data = std::move(*data);
    }
    
    // TODO 1
    cout << "Please enter start and end values:\n";
    float start_x = AskFloat("  - Start X: ");
    float start_y = AskFloat("  - Start Y: ");
    float end_x = AskFloat("  - End X: ");
    float end_y = AskFloat("  - End Y: ");

    // Build Model.
    RouteModel model{osm_data};

    // Create RoutePlanner object and perform A* search.
    RoutePlanner route_planner{model, start_x, start_y, end_x, end_y};
    route_planner.AStarSearch();

    cout << "Distance: " << route_planner.GetDistance() << " meters. \n";

    // Render results of search.
    Render render{model};

    auto display = io2d::output_surface{400, 400, io2d::format::argb32, io2d::scaling::none, io2d::refresh_style::fixed, 30};
    display.size_change_callback([](io2d::output_surface& surface){
        surface.dimensions(surface.display_dimensions());
    });
    display.draw_callback([&](io2d::output_surface& surface){
        render.Display(surface);
    });
    display.begin_show();
}
