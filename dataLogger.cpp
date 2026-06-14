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
        #ifdef DEBUG
            std::cout<<"debug: metadata generated"<<std::endl;
        #endif
        getChannels(insertedFile);
        #ifdef DEBUG
            std::cout<<"debug: channel info generated"<<std::endl;
        #endif
        getData(insertedFile);
        #ifdef DEBUG
            std::cout<<"debug: channel data generated"<<std::endl;
        #endif
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
            dataLogger::channels.push_back({{},parsed,""});       

        }
    int j = 0;
    while(std::getline(sLine7,parsed,'\t'))
        { 
            dataLogger::channels[j].units = parsed;             
            j++;
        }
    #ifdef DEBUG
        for(auto it : dataLogger::channels){
            std::cout<<"debug: channel name is "<<it.id<<std::endl;
            std::cout<<"debug: channel units are "<<it.units<<std::endl;
        }
    #endif
        
}
void dataLogger::getData(std::ifstream& insertedFile){
    #ifdef DEBUG
            std::cout<<"debug: getData called"<<std::endl;
        #endif
    std::string line;
    std::string parsed;
    while(getline(insertedFile,line)){
        #ifdef DEBUG
            std::cout<<"debug: while files exist in the document loop"<<std::endl;
        #endif
        std::stringstream sLine(line);
        int i=0;
        while(getline(sLine,parsed,'\t')){
            try{
                dataLogger::channels[i].data.push_back(std::stod(parsed));
                i++;
            } catch (std::invalid_argument& error){
                dataLogger::channels[i].data.push_back(std::numeric_limits<double>::quiet_NaN());
                i++;
                #ifdef DEBUG
                    std::cout<<"debug: skipping invalid argument"<<parsed<<std::endl;
                #endif
            }

        }
    }
    #ifdef DEBUG
        for(auto it : dataLogger::channels){
            std::cout<<"debug: channel name is "<<it.id<<std::flush;
            std::cout<<" debug: channel units are "<<it.units<<std::flush;
            for (int i = 0; i < 2; i++)
            {
                try{
                std::cout<<" value " << i+1 << ": = " << it.data[i]<< std::endl;
                }catch(std::invalid_argument& error){
                    continue;
                #ifdef DEBUG
                    std::cout<<"debug: skipping invalid argument"<<it.data[i]<<std::endl;
                #endif
                }
            }
            
        }
    #endif
    



}




    

