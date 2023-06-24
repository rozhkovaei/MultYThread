#include <iostream>
#include <sstream>
#include <fstream>
#include <list>
#include <thread>

#include "async.h"

using namespace std;

void console_producer( WaitingQueue< list< string > >& queue ) {

    list< string > bulk;
    
    while( queue.pop( bulk ) )
    {
        for( const auto& cmd : bulk )
        {
            std::cout << cmd << std::endl;
        }
        bulk.clear();
    }
}

void file_producer( WaitingQueue< list< string > >& queue ) {

    list< string > bulk;
    
    while( queue.pop( bulk ) ) {
      //  std::cout << "file_producer - pop new value: " << std::endl;
        
        if( bulk.empty() )
        {
         //   std::cout << "file_producer - empty bulk: " << std::endl;
            continue;
        }
        
        stringstream ss;
        ss << std::chrono::system_clock::now().time_since_epoch().count();
        ss << "_";
        ss << std::this_thread::get_id();
        ss << ".log";
        
        string filename = ss.str();
        
        ofstream myfile;
        myfile.open ( filename );
        
        for( const auto& cmd : bulk )
        {
            myfile << cmd << endl;
        }
        
        myfile.close();
        
        bulk.clear();
    }
 //   std::cout << "file_producer - end of cycle" << std::endl;
}

int main( int argc, char const *argv[] )
{
    WaitingQueue< std::list< std::string > > file_queue;
    WaitingQueue< std::list< std::string > > console_queue;
    
    thread logger = std::thread( &console_producer, std::ref( console_queue ) );
    
    thread file1 = std::thread( &file_producer, std::ref( file_queue ) );
    thread file2 = std::thread( &file_producer, std::ref( file_queue ) );
    
    int size = atoi( argv[ 1 ] );
    
    void* bulk_manager = connect( file_queue, console_queue, size );
    
    for ( string line; getline( cin, line );)
    {
        recieve( bulk_manager, line.data(), line.length() );
    }
    
    disconnect( bulk_manager );
    
    file_queue.stop();
    console_queue.stop();
    
    logger.join();
    file1.join();
    file2.join();
    
    return 0;
}
