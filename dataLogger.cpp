#include "dataLogger.h"
#include <iostream> 
#include <fstream>
#include <stdexcept> 
#include <filesystem>
#include <vector>
#include <utility>
#include <string>
#define DEBUG

void dataLogger::readStore(){
    #ifdef DEBUG
        std::cout<<"debug: readStore() called"<<std::endl;
    #endif
    
    for(const auto& folderEntry : std::filesystem::directory_iterator("copy_file_here")){
        if(!folderEntry.is_regular_file()){
        #ifdef DEBUG
            std::cout<<"debug: file found"<<std::endl;
        #endif
           continue;
           
        }
        std::ifstream insertedFile(folderEntry.path());
        #ifdef DEBUG
            std::cout<<"debug: file opened"<<std::endl;
        #endif

        if(!insertedFile){
            throw std::runtime_error("Failed to open file");
        }
          #ifdef DEBUG
            std::cout<<"debug: file stream created"<<std::endl;
        #endif
        getMetadata(insertedFile);
        getChannels(insertedFile);
        break;
    }
}



void dataLogger::getMetadata(std::ifstream& insertedFile){
    #ifdef DEBUG
        std::cout<<"debug: getMetadata() called"<<std::endl;
    #endif
    std::string line;
    std::getline(insertedFile,line);
    std::getline(insertedFile,line);
    std::getline(insertedFile,line);
    std::getline(insertedFile,line);

    #ifdef DEBUG
        std::cout<<"debug: line read from file"<<std::endl;
        std::cout<<line<<std::endl;
    #endif
    std::stringstream sLine(line);
    std::string parsed;

    while(std::getline(sLine,parsed,'\t'))
    {
        dataLogger::metadata.push_back(parsed);
        #ifdef DEBUG
            std::cout<<"debug: metadata element created" << std::endl;
        #endif
                
    }            
            
        
    for (auto element = dataLogger::metadata.begin(); element != dataLogger::metadata.end();){
            if (element->size() < 3){
                    element = dataLogger::metadata.erase(element);
            }
            else{
                ++element;
            }
        #ifdef DEBUG
            std::cout<<"debug: metadata element erased" << std::endl;
        #endif
    }
    #ifdef DEBUG
       
        for(auto it : dataLogger::metadata){
             std::cout<<"debug: metadata element is "<<it<<std::endl;
        }
    #endif
}

void dataLogger::getChannels(std::ifstream& insertedFile){
    std::string line;
    std::string line6;
    std::string line7;
    std::getline(insertedFile,line);
    std::getline(insertedFile,line6);
    std::getline(insertedFile,line7);
    std::stringstream sLine6(line6);
    std::stringstream sLine7(line7);
    std::string parsed;
    while(std::getline(sLine6,parsed,'\t'))
        {
            dataLogger::channels.push_back({parsed,""});       

        }
    int j = 0;
    while(std::getline(sLine7,parsed,'\t'))
        { 
            dataLogger::channels[j].second = parsed;             
            j++;
        }
    #ifdef DEBUG
        for(auto it : dataLogger::channels){
            std::cout<<"debug: channel name is "<<it.first<<std::endl;
            std::cout<<"debug: channel units are "<<it.second<<std::endl;
        }
    #endif
        

}




    

