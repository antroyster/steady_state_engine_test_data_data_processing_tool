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

        break;
    }
}
void dataLogger::getMetadata(std::ifstream& insertedFile){
    #ifdef DEBUG
        std::cout<<"debug: getMetadata() called"<<std::endl;
    #endif
    std::string line;
    for (int i = 0; i < 4; i++)
    {
        #ifdef DEBUG
            std::cout<<"debug: reading line "<<i<<std::endl;
        #endif
        std::getline(insertedFile,line);
        if (i ==3){
            #ifdef DEBUG
                std::cout<<"debug: line read from file"<<std::endl;
                std::cout<<line<<std::endl;
            #endif
            std::stringstream sLine(line);
            std::string parsed;

            for (int j=0; j<11;j++){
                std::getline(sLine,parsed,'\t');
                
                dataLogger::metadata.push_back(parsed);
                
                #ifdef DEBUG
                    std::cout<<"debug: metadata parsed and stored"<<std::endl;
                    std::cout<<dataLogger::metadata[j]<<std::endl;
                #endif
            }            
            }
        }
    for (auto element = dataLogger::metadata.begin(); element != dataLogger::metadata.end(); ++element){
            if (element->size() < 3){
                    element = dataLogger::metadata.erase(element);
            }
    }
    #ifdef DEBUG
        for(auto it : dataLogger::metadata){
             std::cout<<"debug: metadata element is "<<it<<std::endl;
        }
    #endif
        }
    
    

